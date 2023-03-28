#include "main.h"

// Is this an IP?
bool isIp(String str)
{
  for (size_t i = 0; i < str.length(); i++)
  {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9'))
    {
      return false;
    }
  }
  return true;
}

void initServer()
{

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (captivePortal(request)) return;
              AsyncWebServerResponse *response = request->beginResponse(302);
              if (!showWelcomePage){
                  
                  response->addHeader(F("Location"), F("/index.html"));
                } else {

                  response->addHeader(F("Location"), F("/welcome.html"));
                }
                                
    request->send(response); });

  server.serveStatic("/index.html", HYDROPONICS_FS, "/index.html");
  server.serveStatic("/welcome.html", HYDROPONICS_FS, "/welcome.html");
  server.serveStatic("/settings/wifi.html", HYDROPONICS_FS, "/settings/wifi.html");
  server.serveStatic("/settings/mqtt.html", HYDROPONICS_FS, "/settings/mqtt.html");
  server.serveStatic("/app.js", HYDROPONICS_FS, "/app.js").setCacheControl("max-age=600");
  server.serveStatic("/main.css", HYDROPONICS_FS, "/main.css").setCacheControl("max-age=600");

  server.on("/settings/s.js", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              char buf[SETTINGS_STACK_BUF_SIZE + 37];
              buf[0] = 0;
              byte subPage = request->arg(F("p")).toInt();
              if (subPage > 10)
              {
                strcpy_P(buf, PSTR("alert('Settings for this request are not implemented.');"));
                request->send(501, "application/javascript", buf);
                return;
              }
              
              strcat_P(buf, PSTR("function GetV(){var d=document;"));
              getSettingsJS(subPage, buf + strlen(buf)); // this may overflow by 35bytes!!!
              strcat_P(buf, PSTR("}"));
              request->send(200, "application/javascript", buf); });

  server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request)
            { handleSettingsPOST(request); });

  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/html", "Not found"); });

  // CORS compatiblity
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Methods"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), "*");

  File root = HYDROPONICS_FS.open("/");
  File file = root.openNextFile();
  while (file)
  {

    DEBUG_PRINTLN(file.name());
    file = root.openNextFile();
  }
}

void handleSettingsPOST(AsyncWebServerRequest *request)
{
  byte subPage = 0, originalSubPage = 0;
  const String &url = request->url();
  if (url.indexOf("sett") >= 0)
  {
    if (url.indexOf("wifi") > 0)
      subPage = 1;
    else if (url.indexOf("mqtt") > 0)
      subPage = 4;
  }

  handleSettingsSet(request, subPage);
  if (subPage == 1)
    forceReconnect = true;

  if (subPage == 4)
    doReboot = true;

  AsyncWebServerResponse *response = request->beginResponse(302);
  response->addHeader(F("Location"), F("/index.html"));
  request->send(response);
}

bool captivePortal(AsyncWebServerRequest *request)
{
  if (ON_STA_FILTER(request))
    return false; // only serve captive in AP mode
  String hostH;
  if (!request->hasHeader("Host"))
    return false;
  hostH = request->getHeader("Host")->value();

  if (!isIp(hostH) && hostH.indexOf("hydroponics.me") < 0 && hostH.indexOf(cmDNS) < 0)
  {
    DEBUG_PRINTLN("Captive portal");
    AsyncWebServerResponse *response = request->beginResponse(302);
    response->addHeader(F("Location"), F("http://4.3.2.1/welcome.html"));
    request->send(response);
    return true;
  }
  return false;
}
