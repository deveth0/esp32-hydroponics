#include "main.h"
#include "const.h"

void prepareHostname(char* hostname)
{
  sprintf_P(hostname, "hydroponics-%*s", 6, escapedMac.c_str() + 6);
  const char *pC = serverDescription;
  uint8_t pos = 5;          // keep "wled-"
  while (*pC && pos < 24) { // while !null and not over length
    if (isalnum(*pC)) {     // if the current char is alpha-numeric append it to the hostname
      hostname[pos] = *pC;
      pos++;
    } else if (*pC == ' ' || *pC == '_' || *pC == '-' || *pC == '+' || *pC == '!' || *pC == '?' || *pC == '*') {
      hostname[pos] = '-';
      pos++;
    }
    // else do nothing - no leading hyphens and do not include hyphens for all other characters.
    pC++;
  }
  //last character must not be hyphen
  if (pos > 5) {
    while (pos > 4 && hostname[pos -1] == '-') pos--;
    hostname[pos] = '\0'; // terminate string (leave at least "wled")
  }
}