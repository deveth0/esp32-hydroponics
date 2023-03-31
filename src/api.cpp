#include "main.h"

void initApi()
{

  server.on("/api/status.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleApiStatus(request); });

  server.on("/api/wifi.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleWiFiNetworkList(request); });
}

void handleApiStatus(AsyncWebServerRequest *request)
{

  DynamicJsonDocument doc(1024);

  JsonObject sensors = doc.createNestedObject("sensors");
  long timer = millis();

  long lastUpdate = timer - min({lastDistanceMeasure, lastTemperatureMeasure, lastPhTdsMeasure});

  sensors["lastUpdate"] = lastUpdate;

  addSensor(sensors, F("distance"), F("cm"), lastDistance);
  addSensor(sensors, F("pressure"), F("Pa"), lastPressure);
  addSensor(sensors, F("temperature"), F("°C"), lastTemperature);
  addSensor(sensors, F("waterTemperature"), F("°C"), lastWaterTemperature);
  addSensor(sensors, F("ph"), F("pH"), lastPh);
  addSensor(sensors, F("tds"), F("ppm"), lastTds);

  String data;
  serializeJson(doc, data);
  request->send(200, "application/json", data);
}

void handleWiFiNetworkList(AsyncWebServerRequest *request)
{

  DEBUG_PRINTLN("handleWifiNetworkList");
  DynamicJsonDocument doc(1024);

  int16_t status = WiFi.scanComplete();

  int statusCode = 200;

  if (status == WIFI_SCAN_FAILED)
  {
    WiFi.scanNetworks(true);
    doc["status"] = F("failed");
    WiFi.scanDelete();
    statusCode = 503;
  }
  else if (status == WIFI_SCAN_RUNNING)
  {
    doc["status"] = F("inprogress");
  }
  else
  {
    doc["status"] = F("success");
    JsonArray networks = doc.createNestedArray(F("networks"));

    for (int i = 0; i < status; i++)
    {
      JsonObject node = networks.createNestedObject();
      node["ssid"] = WiFi.SSID(i);
      node["rssi"] = WiFi.RSSI(i);
      node["bssid"] = WiFi.BSSIDstr(i);
      node["channel"] = WiFi.channel(i);
      node["enc"] = WiFi.encryptionType(i);
    }

    WiFi.scanDelete();
  }

  String data;
  serializeJson(doc, data);
  request->send(statusCode, "application/json", data);
}

template <class T>
void addSensor(JsonObject jsonObj, String sensorName, String sensorUnit, T lastValue)
{
  JsonObject sensor = jsonObj.createNestedObject(sensorName);
  sensor["unit"] = sensorUnit;
  sensor["value"] = lastValue;
}