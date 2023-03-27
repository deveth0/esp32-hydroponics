#ifndef WEB_H
#define WEB_H

void initServer();
void handleSettingsPOST(AsyncWebServerRequest *request);
bool captivePortal(AsyncWebServerRequest *request);

#endif
