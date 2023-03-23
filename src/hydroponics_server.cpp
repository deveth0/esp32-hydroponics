#include "main.h"

#include "html_ui.h"
#include "html_other.h"

/*
 * Integrated HTTP web server page declarations
 */

bool handleIfNoneMatchCacheHeader(AsyncWebServerRequest* request);
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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (captivePortal(request)) return;
    serveIndexOrWelcome(request);
  });

  // called when the url is not defined here, ajax-in; get-settings
  server.onNotFound([](AsyncWebServerRequest *request){
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

    AsyncWebServerResponse *response = request->beginResponse_P(404, "text/html", PAGE_404, PAGE_404_length);
    response->addHeader(FPSTR(s_content_enc), "gzip");
    setStaticContentCacheHeaders(response);
    request->send(response);
  });
}

void serveIndexOrWelcome(AsyncWebServerRequest *request)
{
    if (!showWelcomePage){
    serveIndex(request);
  } else {
    serveWelcome(request);
  }
}

void serveWelcome(AsyncWebServerRequest* request)
{

  if (handleFileRead(request, "/welcome.htm")) return;
  
  AsyncWebServerResponse *response;
  response = request->beginResponse_P(200, "text/html", PAGE_welcome,       PAGE_welcome_length);

  response->addHeader(FPSTR(s_content_enc),"gzip");
  setStaticContentCacheHeaders(response);
  request->send(response);

}
void serveIndex(AsyncWebServerRequest* request)
{
  if (handleFileRead(request, "/index.htm")) return;

  if (handleIfNoneMatchCacheHeader(request)) return;

  AsyncWebServerResponse *response;
  response = request->beginResponse_P(200, "text/html", PAGE_index, PAGE_index_L);

  response->addHeader(FPSTR(s_content_enc),"gzip");
  setStaticContentCacheHeaders(response);
  request->send(response);
}

void setStaticContentCacheHeaders(AsyncWebServerResponse *response)
{
  char tmp[12];
  // https://medium.com/@codebyamir/a-web-developers-guide-to-browser-caching-cc41f3b73e7c
  #ifndef WLED_DEBUG
  //this header name is misleading, "no-cache" will not disable cache,
  //it just revalidates on every load using the "If-None-Match" header with the last ETag value
  response->addHeader(F("Cache-Control"),"no-cache");
  #else
  response->addHeader(F("Cache-Control"),"no-store,max-age=0"); // prevent caching if debug build
  #endif
  sprintf_P(tmp, PSTR("%8d-%02x"), VERSION, cacheInvalidate);
  response->addHeader(F("ETag"), tmp);
}

bool handleIfNoneMatchCacheHeader(AsyncWebServerRequest* request)
{
  AsyncWebHeader* header = request->getHeader("If-None-Match");
  if (header && header->value() == String(VERSION)) {
    request->send(304);
    return true;
  }
  return false;
}