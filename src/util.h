#ifndef HYDROPONICS_UTIL_H
#define HYDROPONICS_UTIL_H

void prepareHostname(char* hostname);

bool requestJSONBufferLock(uint8_t module=255);
void releaseJSONBufferLock();

#endif