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
  server.serveStatic("/settings.html", HYDROPONICS_FS, "/settings.html");
  server.serveStatic("/settings/wifi.html", HYDROPONICS_FS, "/settings/wifi.html");
  server.serveStatic("/settings/sensors.html", HYDROPONICS_FS, "/settings/sensors.html");
  server.serveStatic("/settings/pump.html", HYDROPONICS_FS, "/settings/pump.html");
  server.serveStatic("/settings/mqtt.html", HYDROPONICS_FS, "/settings/mqtt.html");
  server.serveStatic("/settings/time.html", HYDROPONICS_FS, "/settings/time.html");
  server.serveStatic("/settings/backup.html", HYDROPONICS_FS, "/settings/backup.html");
  server.serveStatic("/settings/backup/cfg.json", HYDROPONICS_FS, "/cfg.json");
  server.serveStatic("/app.js", HYDROPONICS_FS, "/app.js").setCacheControl("max-age=600");
  server.serveStatic("/main.css", HYDROPONICS_FS, "/main.css").setCacheControl("max-age=600");
  server.serveStatic("/sprite.svg", HYDROPONICS_FS, "/sprite.svg").setCacheControl("max-age=600");

  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/html", "Not found"); });

  // CORS compatiblity
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Methods"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), "*");
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
