
#ifndef HYDROPONICS_SETTINGS_H
#define HYDROPONICS_SETTINGS_H
void getSettingsJS(byte subPage, char* dest);
void handleSettingsSet(AsyncWebServerRequest *request, byte subPage);
#endif