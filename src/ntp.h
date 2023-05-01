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
void checkTimers();
void calculateSunriseAndSunset();
boolean isDaytime();
#endif