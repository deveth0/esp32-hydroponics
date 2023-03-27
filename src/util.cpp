#include "main.h"
#include "const.h"

void prepareHostname(char *hostname)
{
  sprintf_P(hostname, "hydroponics-%*s", 6, escapedMac.c_str() + 6);
  const char *pC = serverDescription;
  uint8_t pos = 5; // keep "wled-"
  while (*pC && pos < 24)
  { // while !null and not over length
    if (isalnum(*pC))
    { // if the current char is alpha-numeric append it to the hostname
      hostname[pos] = *pC;
      pos++;
    }
    else if (*pC == ' ' || *pC == '_' || *pC == '-' || *pC == '+' || *pC == '!' || *pC == '?' || *pC == '*')
    {
      hostname[pos] = '-';
      pos++;
    }
    // else do nothing - no leading hyphens and do not include hyphens for all other characters.
    pC++;
  }
  // last character must not be hyphen
  if (pos > 5)
  {
    while (pos > 4 && hostname[pos - 1] == '-')
      pos--;
    hostname[pos] = '\0'; // terminate string (leave at least "wled")
  }
}

// threading/network callback details: https://github.com/Aircoookie/WLED/pull/2336#discussion_r762276994
bool requestJSONBufferLock(uint8_t module)
{
  unsigned long now = millis();

  while (jsonBufferLock && millis() - now < 1000)
    delay(1); // wait for a second for buffer lock

  if (millis() - now >= 1000)
  {
    DEBUG_PRINT(F("ERROR: Locking JSON buffer failed! ("));
    DEBUG_PRINT(jsonBufferLock);
    DEBUG_PRINTLN(")");
    return false; // waiting time-outed
  }

  jsonBufferLock = module ? module : 255;
  DEBUG_PRINT(F("JSON buffer locked. ("));
  DEBUG_PRINT(jsonBufferLock);
  DEBUG_PRINTLN(")");
  fileDoc = &doc; // used for applying presets (presets.cpp)
  doc.clear();
  return true;
}

void releaseJSONBufferLock()
{
  DEBUG_PRINT(F("JSON buffer released. ("));
  DEBUG_PRINT(jsonBufferLock);
  DEBUG_PRINTLN(")");
  fileDoc = nullptr;
  jsonBufferLock = 0;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// append a numeric setting to string buffer
void sappend(char stype, const char *key, int val)
{
  char ds[] = "d.Sf.";

  switch (stype)
  {
  case 'c': // checkbox
    oappend(ds);
    oappend(key);
    oappend(".checked=");
    oappendi(val);
    oappend(";");
    break;
  case 'v': // numeric
    oappend(ds);
    oappend(key);
    oappend(".value=");
    oappendi(val);
    oappend(";");
    break;
  case 'i': // selectedIndex
    oappend(ds);
    oappend(key);
    oappend(SET_F(".selectedIndex="));
    oappendi(val);
    oappend(";");
    break;
  }
}
// append a string setting to buffer
void sappends(char stype, const char *key, char *val)
{
  switch (stype)
  {
  case 's':
  { // string (we can interpret val as char*)
    String buf = val;
    // convert "%" to "%%" to make EspAsyncWebServer happy
    // buf.replace("%","%%");
    oappend("d.Sf.");
    oappend(key);
    oappend(".value=\"");
    oappend(buf.c_str());
    oappend("\";");
    break;
  }
  case 'm': // message
    oappend(SET_F("d.getElementsByClassName"));
    oappend(key);
    oappend(SET_F(".innerHTML=\""));
    oappend(val);
    oappend("\";");
    break;
  }
}

bool oappendi(int i)
{
  char s[11];
  sprintf(s, "%d", i);
  return oappend(s);
}

bool oappend(const char *txt)
{
  uint16_t len = strlen(txt);
  if (olen + len >= SETTINGS_STACK_BUF_SIZE)
    return false; // buffer full
  strcpy(obuf + olen, txt);
  olen += len;
  return true;
}

bool isAsterisksOnly(const char *str, byte maxLen)
{
  for (byte i = 0; i < maxLen; i++)
  {
    if (str[i] == 0)
      break;
    if (str[i] != '*')
      return false;
  }
  // at this point the password contains asterisks only
  return (str[0] != 0); // false on empty string
}

uint16_t readAverage(uint8_t pin, uint8_t number)
{
  uint64_t read = 0;

  for (uint8_t i = 0; i < number; i++)
  {
    read += analogRead(pin);
    delay(100);
  }
  return read / number;
}