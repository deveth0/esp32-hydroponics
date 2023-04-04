#include "main.h"
#include "AsyncJson.h"

void initApi()
{

  server.on("/api/status.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleApiStatus(request); });

  server.on("/api/config/sensors.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleApiConfigSensors(request); });
  server.addHandler(new AsyncCallbackJsonWebHandler("/api/config/sensors.json", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                    { handleApiConfigSensorsPOST(request, json); }));

  server.on("/api/config/pump.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleApiConfigPump(request); });

  server.addHandler(new AsyncCallbackJsonWebHandler("/api/config/pump.json", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                    { handleApiConfigPumpPOST(request, json); }));

  server.on("/api/wifi.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleWiFiNetworkList(request); });
}

void handleApiConfigSensors(AsyncWebServerRequest *request)
{
  DynamicJsonDocument doc(1024);

  JsonObject sensor = doc.createNestedObject("ph");
  sensor["neutralVoltage"] = phNeutralVoltage;
  sensor["acidVoltage"] = phAcidVoltage;

  sensor = doc.createNestedObject("temperature");
  sensor["adjustment"] = tempAdjustment;

  sensor = doc.createNestedObject("waterTemperature");
  sensor["adjustment"] = waterTempAdjustment;

  sensor = doc.createNestedObject("tank");
  sensor["width"] = tankWidth;
  sensor["height"] = tankHeight;
  sensor["length"] = tankLength;

  String data;
  serializeJson(doc, data);
  request->send(200, "application/json", data);
}

void handleApiConfigSensorsPOST(AsyncWebServerRequest *request, JsonVariant &json)
{

  StaticJsonDocument<512> data = json.as<JsonObject>();

  phNeutralVoltage = data["ph"]["neutralVoltage"];
  phAcidVoltage = data["ph"]["acidVoltage"];

  tempAdjustment = data["temperature"]["adjustment"];
  waterTempAdjustment = data["waterTemperature"]["adjustment"];

  tankWidth = data["tank"]["width"];
  tankHeight = data["tank"]["height"];
  tankLength = data["tank"]["length"];

  doSerializeConfig = true;

  handleApiConfigSensors(request);
}

void handleApiConfigPump(AsyncWebServerRequest *request)
{

  DynamicJsonDocument doc(1024);

  JsonObject pumpConfig = doc.createNestedObject("pumpConfig");

  addPumpConfigEntry(pumpConfig, "le10", pumpLe10Interval, pumpLe10Duration);
  addPumpConfigEntry(pumpConfig, "le15", pumpLe15Interval, pumpLe15Duration);
  addPumpConfigEntry(pumpConfig, "le20", pumpLe20Interval, pumpLe20Duration);
  addPumpConfigEntry(pumpConfig, "le25", pumpLe25Interval, pumpLe25Duration);
  addPumpConfigEntry(pumpConfig, "gt25", pumpGt25Interval, pumpGt25Duration);

  String data;
  serializeJson(doc, data);
  request->send(200, "application/json", data);
}

void handleApiConfigPumpPOST(AsyncWebServerRequest *request, JsonVariant &json)
{
  StaticJsonDocument<512> data = json.as<JsonObject>();

  JsonObject pumpConfig = data["pumpConfig"];

  pumpLe10Interval = pumpConfig["le10"]["interval"];
  pumpLe10Duration = pumpConfig["le10"]["duration"];

  pumpLe15Interval = pumpConfig["le15"]["interval"];
  pumpLe15Duration = pumpConfig["le15"]["duration"];

  pumpLe20Interval = pumpConfig["le20"]["interval"];
  pumpLe20Duration = pumpConfig["le20"]["duration"];

  pumpLe25Interval = pumpConfig["le25"]["interval"];
  pumpLe25Duration = pumpConfig["le25"]["duration"];

  pumpGt25Interval = pumpConfig["gt25"]["interval"];
  pumpGt25Duration = pumpConfig["gt25"]["duration"];

  doSerializeConfig = true;

  handleApiConfigPump(request);
}

void handleApiStatus(AsyncWebServerRequest *request)
{

  DynamicJsonDocument doc(1024);

  doc["pump"] = digitalRead(PUMP_MOSFET_PIN) == HIGH;

  JsonObject sensors = doc.createNestedObject("sensors");
  long timer = millis();

  long lastUpdate = timer - min({lastDistanceMeasure, lastTemperatureMeasure, lastPhTdsMeasure});

  sensors["lastUpdate"] = lastUpdate;

  addSensorStatus(sensors, F("distance"), F("cm"), lastDistance != __INT_MAX__ ? lastDistance : 0);
  addSensorStatus(sensors, F("volume"), F("L"), lastVolume != __FLT_MAX__ ? lastVolume : 0);
  addSensorStatus(sensors, F("pressure"), F("Pa"), lastPressure != __FLT_MAX__ ? lastPressure : 0);
  addSensorStatus(sensors, F("temperature"), F("°C"), lastTemperature != __FLT_MAX__ ? lastTemperature : 0);
  addSensorStatus(sensors, F("waterTemperature"), F("°C"), lastWaterTemperature != __FLT_MAX__ ? lastWaterTemperature : 0);
  addSensorStatus(sensors, F("ph"), F("pH"), lastPh != __FLT_MAX__ ? lastPh : 0);
  addSensorStatus(sensors, F("phVoltage"), F("V"), lastPhVoltage);
  addSensorStatus(sensors, F("tds"), F("ppm"), lastTds != __FLT_MAX__ ? lastTds : 0);

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
void addSensorStatus(JsonObject jsonObj, String sensorName, String sensorUnit, T lastValue)
{
  JsonObject sensor = jsonObj.createNestedObject(sensorName);
  sensor["unit"] = sensorUnit;
  sensor["value"] = lastValue;
}

void addPumpConfigEntry(JsonObject jsonObj, String name, u_int interval, u_int duration)
{
  JsonObject entry = jsonObj.createNestedObject(name);
  entry["interval"] = interval;
  entry["duration"] = duration;
}