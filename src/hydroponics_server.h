#ifndef HYDROPONICS_SERVER_H
#define HYDROPONICS_SERVER_H

bool captivePortal(AsyncWebServerRequest *request);
void serveIndexOrWelcome(AsyncWebServerRequest *request);
void serveIndex(AsyncWebServerRequest* request);
void serveWelcome(AsyncWebServerRequest* request);
void serveSettings(AsyncWebServerRequest* request, bool post);
void serveSettingsJS(AsyncWebServerRequest* request);
void initServer();
void serveMessage(AsyncWebServerRequest* request, uint16_t code, const String& headl, const String& subl, byte optionT);
String msgProcessor(const String& var);
#endif