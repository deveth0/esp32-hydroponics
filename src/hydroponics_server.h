#ifndef HYDROPONICS_SERVER_H
#define HYDROPONICS_SERVER_H

bool captivePortal(AsyncWebServerRequest *request);
void serveIndexOrWelcome(AsyncWebServerRequest *request);
void serveIndex(AsyncWebServerRequest* request);
void initServer();

#endif