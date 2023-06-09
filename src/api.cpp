#include "main.h"
#include "AsyncJson.h"

void initApi()
{

  server.on("/api/status.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleApiStatus(request); });

  server.addHandler(new AsyncCallbackJsonWebHandler("/api/pump.json", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                    { handleApiPumpPOST(request, json); }));

  server.on("/api/config/sensors.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleApiConfigSensors(request); });
  server.addHandler(new AsyncCallbackJsonWebHandler("/api/config/sensors.json", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                    { handleApiConfigSensorsPOST(request, json); }));

  server.on("/api/config/pump.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleApiConfigPump(request); });

  server.addHandler(new AsyncCallbackJsonWebHandler("/api/config/pump.json", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                    { handleApiConfigPumpPOST(request, json); }));

  server.on("/api/config/time.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleApiConfigTime(request); });
  server.addHandler(new AsyncCallbackJsonWebHandler("/api/config/time.json", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                    { handleApiConfigTimePOST(request, json); }));

  server.on("/api/wifiscan.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleWiFiNetworkList(request); });

  server.on("/api/config/wifi.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleApiConfigWifi(request); });

  server.addHandler(new AsyncCallbackJsonWebHandler("/api/config/wifi.json", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                    { handleApiConfigWifiPOST(request, json); }));

  server.on("/api/config/mqtt.json", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleApiConfigMqtt(request); });

  server.addHandler(new AsyncCallbackJsonWebHandler("/api/config/mqtt.json", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                    { handleApiConfigMqttPOST(request, json); }));
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

  sensor["minWaterLevel"] = minWaterLevelCm;
  sensor["maxWaterLevelDifference"] = maxWaterLevelDifferenceCm;

  sensor = doc.createNestedObject("measurement");
  sensor["numberMeasurements"] = numberMeasurements;
  sensor["temperatureInterval"] = temperatureInterval;
  sensor["distanceInterval"] = distanceInterval;
  sensor["phTdsInterval"] = phTdsInterval;
  sensor["phOnTime"] = phOnTime;
  sensor["tdsOnTime"] = tdsOnTime;

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
  minWaterLevelCm = data["tank"]["minWaterLevel"];
  maxWaterLevelDifferenceCm = data["tank"]["maxWaterLevelDifference"];

  numberMeasurements = data["measurement"]["numberMeasurements"];
  temperatureInterval = data["measurement"]["temperatureInterval"];
  distanceInterval = data["measurement"]["distanceInterval"];
  phTdsInterval = data["measurement"]["phTdsInterval"];
  phOnTime = data["measurement"]["phOnTime"];
  tdsOnTime = data["measurement"]["tdsOnTime"];

  doSerializeConfig = true;

  handleApiConfigSensors(request);
}

void handleApiPumpPOST(AsyncWebServerRequest *request, JsonVariant &json)
{
  StaticJsonDocument<512> submitData = json.as<JsonObject>();

  int duration = submitData["duration"];

  PumpHandler::instance().manualPumpRun(duration);

  DynamicJsonDocument doc(512);

  doc["status"] = pumpStatus;
  doc["enabled"] = pumpEnabled;
  doc["running"] = PumpHandler::instance().pumpRunning();
  doc["runningFor"] = PumpHandler::instance().pumpRunUntil() > 0 ? millis() - PumpHandler::instance().pumpRunUntil() : 0;

  String data;
  serializeJson(doc, data);
  request->send(200, "application/json", data);
}

void handleApiConfigPump(AsyncWebServerRequest *request)
{

  DynamicJsonDocument doc(1024);

  doc["pumpEnabled"] = pumpEnabled;

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

  pumpEnabled = data["pumpEnabled"];
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

void handleApiConfigWifi(AsyncWebServerRequest *request)
{

  DynamicJsonDocument doc(512);

  JsonObject wifiConfig = doc.createNestedObject("wifi");
  wifiConfig["ssid"] = clientSSID;
  byte l = 16;
  char fpass[l + 1]; // fill password field with ***
  fpass[l] = 0;
  memset(fpass, '*', l);
  wifiConfig["pwd"] = fpass;

  JsonArray ipArray = wifiConfig.createNestedArray("staticIp");
  JsonArray gatewayArray = wifiConfig.createNestedArray("gateway");
  JsonArray subnetArray = wifiConfig.createNestedArray("subnet");

  for (int i = 0; i < 4; i++)
  {
    ipArray.add(staticIP[i]);
    gatewayArray.add(staticGateway[i]);
    subnetArray.add(staticSubnet[i]);
  }

  doc["mdns"]["address"] = cmDNS;

  JsonObject apConfig = doc.createNestedObject("ap");
  apConfig["ssid"] = apSSID;
  apConfig["pwd"] = fpass;
  apConfig["hideAp"] = apHide == 1;
  apConfig["channel"] = apChannel;
  apConfig["opensOn"] = apBehavior;

  String data;
  serializeJson(doc, data);
  request->send(200, "application/json", data);
}

void handleApiConfigWifiPOST(AsyncWebServerRequest *request, JsonVariant &json)
{
  StaticJsonDocument<512> data = json.as<JsonObject>();

  JsonObject wifiConfig = data["wifi"];
  strlcpy(clientSSID, wifiConfig["ssid"], 33);
  if (!isAsterisksOnly(wifiConfig["pwd"], 41))
    strlcpy(clientPass, wifiConfig["pwd"], 65);

  JsonArray ipArray = wifiConfig["staticIp"].as<JsonArray>();
  JsonArray gatewayArray = wifiConfig["gateway"].as<JsonArray>();
  JsonArray subnetArray = wifiConfig["subnet"].as<JsonArray>();

  for (int i = 0; i < 4; i++)
  {
    staticIP[i] = ipArray[i];
    staticGateway[i] = gatewayArray[i];
    staticSubnet[i] = subnetArray[i];
  }

  strlcpy(cmDNS, data["mdns"]["address"], 33);

  JsonObject apConfig = data["ap"];
  strlcpy(apSSID, apConfig["ssid"], 33);
  if (!isAsterisksOnly(apConfig["pwd"], 41))
    strlcpy(apPass, apConfig["pwd"], 65);

  apHide = apConfig["hideAp"] == true ? 1 : 0;
  apChannel = apConfig["channel"];
  apBehavior = apConfig["opensOn"];

  doSerializeConfig = true;
  forceReconnect = true;

  handleApiConfigWifi(request);
}

void handleApiConfigMqtt(AsyncWebServerRequest *request)
{

  DynamicJsonDocument doc(512);

  doc["enabled"] = mqttEnabled;
  doc["broker"] = mqttServer;
  doc["port"] = mqttPort;
  doc["user"] = mqttUser;
  byte l = 16;
  char fpass[l + 1]; // fill password field with ***
  fpass[l] = 0;
  memset(fpass, '*', l);
  doc["pwd"] = fpass;
  doc["clientId"] = mqttClientID;
  doc["deviceTopic"] = mqttDeviceTopic;
  doc["groupTopic"] = mqttGroupTopic;

  String data;
  serializeJson(doc, data);
  request->send(200, "application/json", data);
}

void handleApiConfigMqttPOST(AsyncWebServerRequest *request, JsonVariant &json)
{
  StaticJsonDocument<512> data = json.as<JsonObject>();

  mqttEnabled = data["enabled"];
  strlcpy(mqttServer, data["broker"], 33);
  mqttPort = data["port"];
  strlcpy(mqttUser, data["user"], 33);
  if (!isAsterisksOnly(data["pwd"], 41))
    strlcpy(mqttPass, data["pwd"], 65);

  strlcpy(mqttClientID, data["clientId"], 33);
  strlcpy(mqttDeviceTopic, data["deviceTopic"], 33);
  strlcpy(mqttGroupTopic, data["groupTopic"], 33);

  doSerializeConfig = true;

  handleApiConfigMqtt(request);
}

void handleApiStatus(AsyncWebServerRequest *request)
{

  DynamicJsonDocument doc(1536);

  doc["pump"]["status"] = pumpStatus;
  doc["pump"]["enabled"] = pumpEnabled;
  doc["pump"]["running"] = PumpHandler::instance().pumpRunning();
  doc["pump"]["runningFor"] = PumpHandler::instance().pumpRunUntil() > 0 ? millis() - PumpHandler::instance().pumpRunUntil() : 0;
  doc["pump"]["lastPumpStartTankLevel"] = PumpHandler::instance().pumpStartTankLevel();
  doc["pump"]["lastPumpEndTankLevel"] = PumpHandler::instance().pumpEndTankLevel();
  doc["wifiStatus"] = HYDROPONICS_CONNECTED ? F("Connected") : F("Disconnected");
  doc["mqttStatus"] = (!mqttEnabled || mqttServer[0] == 0) ? F("Disabled") : HYDROPONICS_MQTT_CONNECTED ? F("Connected")
                                                                                                        : F("Disconnected");

  char timeString[sizeof "yyyy-mm-ddThh:mm:ssZ"];
  getTimeString(timeString, sizeof(timeString));
  doc["date"] = timeString;
  doc["uptime"] = millis() / 1000 + rolloverMillis * 4294967;

  JsonObject ntp = doc.createNestedObject("ntp");
  ntp["connected"] = ntpConnected;
  getTimeString(ntpLastSyncTime, timeString, sizeof(timeString));
  ntp["lastSyncTime"] = timeString;
  getTimeString(ntpPacketSentTime, timeString, sizeof(timeString));
  ntp["packetSendTime"] = timeString;
  ntp["status"] = ntpStatus;

  if (sunset != 0 && sunrise != 0)
  {
    getTimeString(sunset, timeString, sizeof(timeString));
    ntp["sunset"] = timeString;

    getTimeString(sunrise, timeString, sizeof(timeString));
    ntp["sunrise"] = timeString;
  }

  JsonObject sensors = doc.createNestedObject("sensors");
  sensors["status"] = sensorsStatus;

  long timer = millis();
  long lastUpdate = timer - min({lastDistanceMeasure, lastTemperatureMeasure, lastPhTdsMeasure});

  sensors["lastUpdate"] = lastUpdate;

  addSensorStatus(sensors, F("distance"), F("cm"), lastDistance != __INT_MAX__ ? lastDistance : 0);
  addSensorStatus(sensors, F("volume"), F("L"), lastVolume != __FLT_MAX__ ? lastVolume : 0);
  addSensorStatus(sensors, F("waterLevel"), F("%"), lastWaterLevel != __INT_MAX__ ? lastWaterLevel : 0);
  addSensorStatus(sensors, F("pressure"), F("Pa"), lastPressure != __FLT_MAX__ ? lastPressure : 0);
  addSensorStatus(sensors, F("temperature"), F("°C"), lastTemperature != __FLT_MAX__ ? String(lastTemperature, 2) : "0");
  addSensorStatus(sensors, F("waterTemperature"), F("°C"), lastWaterTemperature != __FLT_MAX__ ? String(lastWaterTemperature, 2) : "0");
  addSensorStatus(sensors, F("ph"), F("pH"), lastPh != __FLT_MAX__ ? lastPh : 0);
  addSensorStatus(sensors, F("phVoltage"), F("V"), String(lastPhVoltage, 2));
  addSensorStatus(sensors, F("tds"), F("ppm"), lastTds != __FLT_MAX__ ? lastTds : 0);
  addSensorStatus(sensors, F("ec"), F("ms/cm"), lastEc != __FLT_MAX__ ? lastEc : 0);

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
    statusCode = 202;
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

void handleApiConfigTime(AsyncWebServerRequest *request)
{
  DynamicJsonDocument doc(1024);

  doc["ntpServer"] = ntpServerName;
  doc["longitude"] = String(longitude, 2);
  doc["latitude"] = String(latitude, 2);

  String data;
  serializeJson(doc, data);
  request->send(200, "application/json", data);
}

void handleApiConfigTimePOST(AsyncWebServerRequest *request, JsonVariant &json)
{
  StaticJsonDocument<512> data = json.as<JsonObject>();

  strlcpy(ntpServerName, data["ntpServer"], 33);

  longitude = data["longitude"];
  latitude = data["latitude"];

  doSerializeConfig = true;

  ntpLastSyncTime = 0; // force new NTP query
  calculateSunriseAndSunset();

  handleApiConfigTime(request);
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