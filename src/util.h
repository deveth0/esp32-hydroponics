#ifndef HYDROPONICS_UTIL_H
#define HYDROPONICS_UTIL_H

void prepareHostname(char* hostname);

bool requestJSONBufferLock(uint8_t module=255);
void releaseJSONBufferLock();
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
void sappend(char stype, const char* key, int val);
void sappends(char stype, const char* key, char* val);
bool oappend(const char* txt);
bool oappendi(int i);
bool isAsterisksOnly(const char* str, byte maxLen);
#endif