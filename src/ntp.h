#ifndef HYDROPONICS_NTP_H
#define HYDROPONICS_NTP_H

void initNtp();
void handleTime();
void handleNetworkTime();
void sendNTPPacket();
bool checkNTPResponse();
void updateLocalTime();
void getTimeString(char *out, size_t len);
void getTimeString(time_t time, char *out, size_t len);
byte weekdayMondayFirst();
void checkTimers();
void calculateSunriseAndSunset();
#endif