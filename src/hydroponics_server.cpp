#include "main.h"

/*
 * Integrated HTTP web server page declarations
 */

bool handleIfNoneMatchCacheHeader(AsyncWebServerRequest *request);
void setStaticContentCacheHeaders(AsyncWebServerResponse *response);

// define flash strings once (saves flash memory)
static const char s_redirecting[] PROGMEM = "Redirecting...";
static const char s_content_enc[] PROGMEM = "Content-Encoding";

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

bool captivePortal(AsyncWebServerRequest *request)
{
  if (ON_STA_FILTER(request))
    return false; // only serve captive in AP mode
  String hostH;
  if (!request->hasHeader("Host"))
    return false;
  hostH = request->getHeader("Host")->value();

  DEBUG_PRINTF("Captive portal %s\n", hostH);

  if (!isIp(hostH) && hostH.indexOf("hydroponic.me") < 0 && hostH.indexOf(cmDNS) < 0)
  {
    AsyncWebServerResponse *response = request->beginResponse(302);
    response->addHeader(F("Location"), F("http://4.3.2.1"));
    request->send(response);
    return true;
  }
  return false;
}

void initServer()
{
  // CORS compatiblity
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Methods"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), "*");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (captivePortal(request)) return;
    serveIndexOrWelcome(request); });

  // settings page
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
            { serveSettings(request, false); });

  server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request)
            { serveSettings(request, true); });

  // called when the url is not defined here, ajax-in; get-settings
  server.onNotFound([](AsyncWebServerRequest *request)
                    {
    DEBUG_PRINTLN("Not-Found HTTP call:");
    DEBUG_PRINTLN("URI: " + request->url());
    if (captivePortal(request))
      return;

    // make API CORS compatible
    if (request->method() == HTTP_OPTIONS)
    {
      AsyncWebServerResponse *response = request->beginResponse(200);
      response->addHeader(F("Access-Control-Max-Age"), F("7200"));
      request->send(response);
      return;
    }
    handleFileRead(request, "/404.htm"); });
}

void serveIndexOrWelcome(AsyncWebServerRequest *request)
{
  if (!showWelcomePage)
  {
    serveIndex(request);
  }
  else
  {
    serveWelcome(request);
  }
}

void serveWelcome(AsyncWebServerRequest *request)
{
  handleFileRead(request, "/welcome.htm");
}

void serveIndex(AsyncWebServerRequest *request)
{
  handleFileRead(request, "/index.htm");
}

void serveSettings(AsyncWebServerRequest *request, bool post)
{
  byte subPage = 0, originalSubPage = 0;
  const String &url = request->url();

  if (url.indexOf("sett") >= 0)
  {
    if (url.indexOf(".js") > 0)
      subPage = 254;
    else if (url.indexOf(".css") > 0)
      subPage = 253;
    else if (url.indexOf("wifi") > 0)
      subPage = 1;
    else if (url.indexOf("sync") > 0)
      subPage = 4;
  }
  else
    subPage = 255; // welcome page

  if (post)
  { // settings/set POST request, saving
    DEBUG_PRINTF("Handle POST %s (%i)", url, subPage);

    handleSettingsSet(request, subPage);

    char s[32];
    char s2[45] = "";

    switch (subPage)
    {
    case 1:
      strcpy_P(s, PSTR("WiFi"));
      strcpy_P(s2, PSTR("Please connect to the new IP (if changed)"));
      forceReconnect = true;
      break;
    case 4:
      strcpy_P(s, PSTR("Sync"));
      break;
    }

    strcat_P(s, PSTR(" settings saved."));

    if (!s2[0])
      strcpy_P(s2, s_redirecting);

    serveMessage(request, 200, s, s2, (subPage == 1 || (subPage == 6 && doReboot)) ? 129 : 1);
    return;
  }

  DEBUG_PRINTF("Subpage: %i, requested %s\n", subPage, url);

  AsyncWebServerResponse *response;
  switch (subPage)
  {
  case 1:
    handleFileRead(request, "/settings_wifi.htm");
    return;
  case 4:
    handleFileRead(request, "/settings_sync.htm");
    return;
  case 254:
    serveSettingsJS(request);
    return;
  case 255:
    handleFileRead(request, "/welcome.htm");
    return;
  default:
    handleFileRead(request, "/settings.htm");
    return;
  }
  response->addHeader(FPSTR(s_content_enc), "gzip");
  setStaticContentCacheHeaders(response);
  request->send(response);
}

void serveMessage(AsyncWebServerRequest *request, uint16_t code, const String &headl, const String &subl, byte optionT)
{
  messageHead = headl;
  messageSub = subl;
  optionType = optionT;

  request->send(HYDROPONICS_FS, "/msg.htm", "text/html", false, msgProcessor);
}

String msgProcessor(const String &var)
{
  if (var == "MSG")
  {
    String messageBody = messageHead;
    messageBody += F("</h2>");
    messageBody += messageSub;
    uint32_t optt = optionType;

    if (optt < 60) // redirect to settings after optionType seconds
    {
      messageBody += F("<script>setTimeout(RS,");
      messageBody += String(optt * 1000);
      messageBody += F(")</script>");
    }
    else if (optt < 120) // redirect back after optionType-60 seconds, unused
    {
      // messageBody += "<script>setTimeout(B," + String((optt-60)*1000) + ")</script>";
    }
    else if (optt < 180) // reload parent after optionType-120 seconds
    {
      messageBody += F("<script>setTimeout(RP,");
      messageBody += String((optt - 120) * 1000);
      messageBody += F(")</script>");
    }
    else if (optt == 253)
    {
      messageBody += F("<br><br><form action=/settings><button class=\"bt\" type=submit>Back</button></form>"); // button to settings
    }
    else if (optt == 254)
    {
      messageBody += F("<br><br><button type=\"button\" class=\"bt\" onclick=\"B()\">Back</button>");
    }
    return messageBody;
  }
  return String();
}

void serveSettingsJS(AsyncWebServerRequest *request)
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
  request->send(200, "application/javascript", buf);
}

void setStaticContentCacheHeaders(AsyncWebServerResponse *response)
{
  char tmp[12];
// https://medium.com/@codebyamir/a-web-developers-guide-to-browser-caching-cc41f3b73e7c
#ifndef WLED_DEBUG
  // this header name is misleading, "no-cache" will not disable cache,
  // it just revalidates on every load using the "If-None-Match" header with the last ETag value
  response->addHeader(F("Cache-Control"), "no-cache");
#else
  response->addHeader(F("Cache-Control"), "no-store,max-age=0"); // prevent caching if debug build
#endif
  sprintf_P(tmp, PSTR("%8d-%02x"), VERSION, cacheInvalidate);
  response->addHeader(F("ETag"), tmp);
}

bool handleIfNoneMatchCacheHeader(AsyncWebServerRequest *request)
{
  AsyncWebHeader *header = request->getHeader("If-None-Match");
  if (header && header->value() == String(VERSION))
  {
    request->send(304);
    return true;
  }
  return false;
}