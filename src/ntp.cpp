#include "main.h"

/*
 * Acquires time from NTP server
 */
// #define WLED_DEBUG_NTP
#define NTP_SYNC_INTERVAL 42000UL // Get fresh NTP time about twice per day

void initNtp()
{
  ntpConnected = ntpUdp.begin(ntpLocalPort);
}

void getTimeString(char *out, size_t len)
{
  updateLocalTime();
  time_t now = time_t(toki.second());
  getTimeString(now, out, len);
}

void getTimeString(time_t time, char *out, size_t len)
{
  strftime(out, len, "%FT%TZ", localtime(&time));
}

void sendNTPPacket()
{
  if (!ntpServerIP.fromString(ntpServerName)) // see if server is IP or domain
  {
#ifdef ESP8266
    WiFi.hostByName(ntpServerName, ntpServerIP, 750);
#else
    WiFi.hostByName(ntpServerName, ntpServerIP);
#endif
  }

  DEBUG_PRINTLN(F("send NTP"));
  byte pbuf[NTP_PACKET_SIZE];
  memset(pbuf, 0, NTP_PACKET_SIZE);

  pbuf[0] = 0b11100011; // LI, Version, Mode
  pbuf[1] = 0;          // Stratum, or type of clock
  pbuf[2] = 6;          // Polling Interval
  pbuf[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  pbuf[12] = 49;
  pbuf[13] = 0x4E;
  pbuf[14] = 49;
  pbuf[15] = 52;

  ntpUdp.beginPacket(ntpServerIP, 123); // NTP requests are to port 123
  ntpUdp.write(pbuf, NTP_PACKET_SIZE);
  ntpUdp.endPacket();
}

bool checkNTPResponse()
{
  int cb = ntpUdp.parsePacket();
  if (!cb)
    return false;

  uint32_t ntpPacketReceivedTime = millis();
  DEBUG_PRINT(F("NTP recv, l="));
  DEBUG_PRINTLN(cb);
  byte pbuf[NTP_PACKET_SIZE];
  ntpUdp.read(pbuf, NTP_PACKET_SIZE); // read the packet into the buffer

  Toki::Time arrived = toki.fromNTP(pbuf + 32);
  Toki::Time departed = toki.fromNTP(pbuf + 40);
  if (departed.sec == 0)
    return false;
  // basic half roundtrip estimation
  uint32_t serverDelay = toki.msDifference(arrived, departed);
  uint32_t offset = (ntpPacketReceivedTime - ntpPacketSentTime - serverDelay) >> 1;

  toki.adjust(departed, offset);
  toki.setTime(departed, TOKI_TS_NTP);

  // if time changed re-calculate sunrise/sunset
  updateLocalTime();
  calculateSunriseAndSunset();
  return true;
}

void handleTime()
{
  handleNetworkTime();

  toki.millisecond();
  toki.setTick();

  if (toki.isTick()) // true only in the first loop after a new second started
  {
    updateLocalTime();
  }
}

void handleNetworkTime()
{
  if (ntpConnected && millis() - ntpLastSyncTime > (1000 * NTP_SYNC_INTERVAL) && HYDROPONICS_CONNECTED)
  {
    if (millis() - ntpPacketSentTime > 10000)
    {
      sendNTPPacket();
      ntpPacketSentTime = millis();
    }
    if (checkNTPResponse())
    {
      ntpLastSyncTime = millis();
    }
  }
}

void updateLocalTime()
{
  unsigned long tmc = toki.second();
  localTime = tmc; // tz->toLocal(tmc);
}

void checkTimers()
{
  if (lastTimerMinute != minute(localTime)) // only check once a new minute begins
  {
    lastTimerMinute = minute(localTime);

    // re-calculate sunrise and sunset just after midnight
    if (!hour(localTime) && minute(localTime) == 1)
      calculateSunriseAndSunset();
  }
}

#define ZENITH -0.83
// get sunrise (or sunset) time (in minutes) for a given day at a given geo location
int getSunriseUTC(int year, int month, int day, float lat, float lon, bool sunset = false)
{
  // 1. first calculate the day of the year
  float N1 = 275 * month / 9;
  float N2 = (month + 9) / 12;
  float N3 = (1 + floor_t((year - 4 * floor_t(year / 4) + 2) / 3));
  float N = N1 - (N2 * N3) + day - 30;

  // 2. convert the longitude to hour value and calculate an approximate time
  float lngHour = lon / 15.0f;
  float t = N + (((sunset ? 18 : 6) - lngHour) / 24);

  // 3. calculate the Sun's mean anomaly
  float M = (0.9856f * t) - 3.289f;

  // 4. calculate the Sun's true longitude
  float L = fmod_t(M + (1.916f * sin_t(DEG_TO_RAD * M)) + (0.02f * sin_t(2 * DEG_TO_RAD * M)) + 282.634f, 360.0f);

  // 5a. calculate the Sun's right ascension
  float RA = fmod_t(RAD_TO_DEG * atan_t(0.91764f * tan_t(DEG_TO_RAD * L)), 360.0f);

  // 5b. right ascension value needs to be in the same quadrant as L
  float Lquadrant = floor_t(L / 90) * 90;
  float RAquadrant = floor_t(RA / 90) * 90;
  RA = RA + (Lquadrant - RAquadrant);

  // 5c. right ascension value needs to be converted into hours
  RA /= 15.0f;

  // 6. calculate the Sun's declination
  float sinDec = 0.39782f * sin_t(DEG_TO_RAD * L);
  float cosDec = cos_t(asin_t(sinDec));

  // 7a. calculate the Sun's local hour angle
  float cosH = (sin_t(DEG_TO_RAD * ZENITH) - (sinDec * sin_t(DEG_TO_RAD * lat))) / (cosDec * cos_t(DEG_TO_RAD * lat));
  if (cosH > 1 && !sunset)
    return 0; // the sun never rises on this location (on the specified date)
  if (cosH < -1 && sunset)
    return 0; // the sun never sets on this location (on the specified date)

  // 7b. finish calculating H and convert into hours
  float H = sunset ? RAD_TO_DEG * acos_t(cosH) : 360 - RAD_TO_DEG * acos_t(cosH);
  H /= 15.0f;

  // 8. calculate local mean time of rising/setting
  float T = H + RA - (0.06571f * t) - 6.622f;

  // 9. adjust back to UTC
  float UT = fmod_t(T - lngHour, 24.0f);

  // return in minutes from midnight
  return UT * 60;
}

// calculate sunrise and sunset (if longitude and latitude are set)
void calculateSunriseAndSunset()
{
  if ((int)(longitude * 10.) || (int)(latitude * 10.))
  {
    struct tm tim_0;
    tim_0.tm_year = year(localTime) - 1900;
    tim_0.tm_mon = month(localTime) - 1;
    tim_0.tm_mday = day(localTime);
    tim_0.tm_sec = 0;
    tim_0.tm_isdst = 0;

    int minUTC = getSunriseUTC(year(localTime), month(localTime), day(localTime), latitude, longitude);
    if (minUTC)
    {
      // there is a sunrise
      if (minUTC < 0)
        minUTC += 24 * 60; // add a day if negative
      tim_0.tm_hour = minUTC / 60;
      tim_0.tm_min = minUTC % 60;
      sunrise = mktime(&tim_0);
      DEBUG_PRINTF("Sunrise: %02d:%02d\n", hour(sunrise), minute(sunrise));
    }
    else
    {
      sunrise = 0;
    }

    minUTC = getSunriseUTC(year(localTime), month(localTime), day(localTime), latitude, longitude, true);
    if (minUTC)
    {
      // there is a sunset
      if (minUTC < 0)
        minUTC += 24 * 60; // add a day if negative
      tim_0.tm_hour = minUTC / 60;
      tim_0.tm_min = minUTC % 60;
      sunset = mktime(&tim_0);
      DEBUG_PRINTF("Sunset: %02d:%02d\n", hour(sunset), minute(sunset));
    }
    else
    {
      sunset = 0;
    }
  }
}