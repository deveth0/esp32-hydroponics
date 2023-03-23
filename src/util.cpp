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