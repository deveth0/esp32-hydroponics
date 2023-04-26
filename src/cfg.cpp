#include "main.h"

/*
 * Serializes and parses the cfg.json and wsec.json settings files, stored in internal FS.
 * The structure of the JSON is not to be considered an official API and may change without notice.
 */

// simple macro for ArduinoJSON's or syntax
#define CJSON(a, b) a = b | a

void getStringFromJson(char *dest, const char *src, size_t len)
{
  if (src != nullptr)
    strlcpy(dest, src, len);
}

bool deserializeConfig(JsonObject doc, bool fromFS)
{
  serializeJsonPretty(doc, Serial);

  bool needsSave = false;
  // int rev_major = doc["rev"][0]; // 1
  // int rev_minor = doc["rev"][1]; // 0

  // long vid = doc[F("vid")]; // 2010020

  JsonObject id = doc["id"];
  getStringFromJson(cmDNS, id[F("mdns")], 33);
  getStringFromJson(serverDescription, id[F("name")], 33);

  JsonObject nw_ins_0 = doc["nw"]["ins"][0];
  getStringFromJson(clientSSID, nw_ins_0[F("ssid")], 33);
  // int nw_ins_0_pskl = nw_ins_0[F("pskl")];
  // The WiFi PSK is normally not contained in the regular file for security reasons.
  // If it is present however, we will use it
  getStringFromJson(clientPass, nw_ins_0["psk"], 65);

  JsonArray nw_ins_0_ip = nw_ins_0["ip"];
  JsonArray nw_ins_0_gw = nw_ins_0["gw"];
  JsonArray nw_ins_0_sn = nw_ins_0["sn"];

  for (byte i = 0; i < 4; i++)
  {
    CJSON(staticIP[i], nw_ins_0_ip[i]);
    CJSON(staticGateway[i], nw_ins_0_gw[i]);
    CJSON(staticSubnet[i], nw_ins_0_sn[i]);
  }

  JsonObject ap = doc["ap"];
  getStringFromJson(apSSID, ap[F("ssid")], 33);
  getStringFromJson(apPass, ap["psk"], 65); // normally not present due to security
  // int ap_pskl = ap[F("pskl")];

  CJSON(apChannel, ap[F("chan")]);
  if (apChannel > 13 || apChannel < 1)
    apChannel = 1;

  CJSON(apHide, ap[F("hide")]);
  if (apHide > 1)
    apHide = 1;

  CJSON(apBehavior, ap[F("behav")]);

  /*
  JsonArray ap_ip = ap["ip"];
  for (byte i = 0; i < 4; i++) {
    apIP[i] = ap_ip;
  }
  */

  JsonObject if_mqtt = doc["mqtt"];
  CJSON(mqttEnabled, if_mqtt["en"]);
  getStringFromJson(mqttServer, if_mqtt[F("broker")], 33);
  CJSON(mqttPort, if_mqtt["port"]); // 1883
  getStringFromJson(mqttUser, if_mqtt[F("user")], 41);
  getStringFromJson(mqttPass, if_mqtt["psk"], 65); // normally not present due to security
  getStringFromJson(mqttClientID, if_mqtt[F("cid")], 41);

  getStringFromJson(mqttDeviceTopic, if_mqtt[F("topics")][F("device")], 33);
  getStringFromJson(mqttGroupTopic, if_mqtt[F("topics")][F("group")], 33);

  // pump config
  JsonObject pumpConfig = doc[F("pumpConfig")];
  CJSON(pumpEnabled, pumpConfig["pumpEnabled"]);

  JsonObject le10 = pumpConfig[F("le10")];
  CJSON(pumpLe10Interval, le10["interval"]);
  CJSON(pumpLe10Duration, le10["duration"]);

  JsonObject le15 = pumpConfig[F("le15")];
  CJSON(pumpLe15Interval, le15["interval"]);
  CJSON(pumpLe15Duration, le15["duration"]);

  JsonObject le20 = pumpConfig[F("le20")];
  CJSON(pumpLe20Interval, le20["interval"]);
  CJSON(pumpLe20Duration, le20["duration"]);

  JsonObject le25 = pumpConfig[F("le25")];
  CJSON(pumpLe25Interval, le25["interval"]);
  CJSON(pumpLe25Duration, le25["duration"]);

  JsonObject gt25 = pumpConfig[F("gt25")];
  CJSON(pumpGt25Interval, gt25["interval"]);
  CJSON(pumpGt25Duration, gt25["duration"]);

  // sensors config

  JsonObject sensorsConfig = doc[F("sensors")];

  JsonObject ph = sensorsConfig[F("ph")];
  CJSON(phNeutralVoltage, ph["neutralVoltage"]);
  CJSON(phAcidVoltage, ph["acidVoltage"]);

  JsonObject tempSensor = sensorsConfig[F("temperature")];
  CJSON(tempAdjustment, tempSensor["adjustment"]);

  JsonObject waterTempSensor = sensorsConfig[F("waterTemperature")];
  CJSON(waterTempAdjustment, waterTempSensor["adjustment"]);

  JsonObject tank = sensorsConfig[F("tank")];
  CJSON(tankHeight, tank["height"]);
  CJSON(tankWidth, tank["width"]);
  CJSON(tankLength, tank["length"]);
  CJSON(minWaterLevelCm, tank["minWaterLevelCm"]);
  CJSON(maxWaterLevelDifferenceCm, tank["maxWaterLevelDifferenceCm"]);

  JsonObject measurementConfig = doc[F("measurement")];

  CJSON(numberMeasurements, measurementConfig["numberMeasurements"]);
  CJSON(temperatureInterval, measurementConfig["temperatureInterval"]);
  CJSON(distanceInterval, measurementConfig["distanceInterval"]);
  CJSON(phTdsInterval, measurementConfig["phTdsInterval"]);
  CJSON(phOnTime, measurementConfig["phOnTime"]);
  CJSON(tdsOnTime, measurementConfig["tdsOnTime"]);

  if (fromFS)
    return needsSave;
  // if from /json/cfg
  doReboot = doc[F("rb")] | doReboot;
  return (doc["sv"] | true);
}

void deserializeConfigFromFS()
{
  bool success = deserializeConfigSec();
  if (!success)
  { // if file does not exist, try reading from EEPROM
#ifdef HYDROPONICS_ADD_EEPROM_SUPPORT
    deEEPSettings();
    return;
#endif
  }

  if (!requestJSONBufferLock(1))
    return;

  success = readObjectFromFile("/cfg.json", nullptr, &doc);
  if (!success)
  { // if file does not exist, optionally try reading from EEPROM and then save defaults to FS
    DEBUG_PRINTLN("Could not read cfg.json");
    releaseJSONBufferLock();
#ifdef HYDROPONICS_ADD_EEPROM_SUPPORT
    deEEPSettings();
#endif

    // save default values to /cfg.json
    serializeConfig();
    return;
  }

  // NOTE: This routine deserializes *and* applies the configuration
  bool needsSave = deserializeConfig(doc.as<JsonObject>(), true);
  releaseJSONBufferLock();

  if (needsSave)
    serializeConfig(); // usermods required new parameters
}

void serializeConfig()
{
  serializeConfigSec();

  if (!requestJSONBufferLock(2))
    return;

  JsonArray rev = doc.createNestedArray("rev");
  rev.add(1); // major settings revision
  rev.add(0); // minor settings revision

  doc[F("vid")] = VERSION;

  JsonObject id = doc.createNestedObject("id");
  id[F("mdns")] = cmDNS;
  id[F("name")] = serverDescription;

  JsonObject nw = doc.createNestedObject("nw");

  JsonArray nw_ins = nw.createNestedArray("ins");

  JsonObject nw_ins_0 = nw_ins.createNestedObject();
  nw_ins_0[F("ssid")] = clientSSID;
  nw_ins_0[F("pskl")] = strlen(clientPass);

  JsonArray nw_ins_0_ip = nw_ins_0.createNestedArray("ip");
  JsonArray nw_ins_0_gw = nw_ins_0.createNestedArray("gw");
  JsonArray nw_ins_0_sn = nw_ins_0.createNestedArray("sn");

  for (byte i = 0; i < 4; i++)
  {
    nw_ins_0_ip.add(staticIP[i]);
    nw_ins_0_gw.add(staticGateway[i]);
    nw_ins_0_sn.add(staticSubnet[i]);
  }

  JsonObject ap = doc.createNestedObject("ap");
  ap[F("ssid")] = apSSID;
  ap[F("pskl")] = strlen(apPass);
  ap[F("chan")] = apChannel;
  ap[F("hide")] = apHide;
  ap[F("behav")] = apBehavior;

  JsonArray ap_ip = ap.createNestedArray("ip");
  ap_ip.add(4);
  ap_ip.add(3);
  ap_ip.add(2);
  ap_ip.add(1);

  JsonObject if_mqtt = doc.createNestedObject("mqtt");
  if_mqtt["en"] = mqttEnabled;
  if_mqtt[F("broker")] = mqttServer;
  if_mqtt["port"] = mqttPort;
  if_mqtt[F("user")] = mqttUser;
  if_mqtt[F("pskl")] = strlen(mqttPass);
  if_mqtt[F("cid")] = mqttClientID;

  JsonObject if_mqtt_topics = if_mqtt.createNestedObject(F("topics"));
  if_mqtt_topics[F("device")] = mqttDeviceTopic;
  if_mqtt_topics[F("group")] = mqttGroupTopic;

  JsonObject pumpConfig = doc.createNestedObject("pumpConfig");
  pumpConfig["pumpEnabled"] = pumpEnabled;

  JsonObject le10 = pumpConfig.createNestedObject("le10");
  le10["interval"] = pumpLe10Interval;
  le10["duration"] = pumpLe10Duration;

  JsonObject le15 = pumpConfig.createNestedObject("le15");
  le15["interval"] = pumpLe15Interval;
  le15["duration"] = pumpLe15Duration;

  JsonObject le20 = pumpConfig.createNestedObject("le20");
  le20["interval"] = pumpLe20Interval;
  le20["duration"] = pumpLe20Duration;

  JsonObject le25 = pumpConfig.createNestedObject("le25");
  le25["interval"] = pumpLe25Interval;
  le25["duration"] = pumpLe25Duration;

  JsonObject gt25 = pumpConfig.createNestedObject("gt25");
  gt25["interval"] = pumpGt25Interval;
  gt25["duration"] = pumpGt25Duration;

  // sensors config
  JsonObject sensorsConfig = doc.createNestedObject("sensors");

  JsonObject ph = sensorsConfig.createNestedObject("ph");
  ph["neutralVoltage"] = phNeutralVoltage;
  ph["acidVoltage"] = phAcidVoltage;

  JsonObject tempSensor = sensorsConfig.createNestedObject("temperature");
  tempSensor["adjustment"] = tempAdjustment;
  JsonObject waterTempSensor = sensorsConfig.createNestedObject("waterTemperature");
  waterTempSensor["adjustment"] = waterTempAdjustment;
  JsonObject tank = sensorsConfig.createNestedObject("tank");
  tank["width"] = tankWidth;
  tank["height"] = tankHeight;
  tank["length"] = tankLength;
  tank["minWaterLevelCm"] = minWaterLevelCm;
  tank["maxWaterLevelDifferenceCm"] = maxWaterLevelDifferenceCm;

  JsonObject measurement = doc.createNestedObject("measurement");

  measurement["numberMeasurements"] = numberMeasurements;
  measurement["temperatureInterval"] = temperatureInterval;
  measurement["distanceInterval"] = distanceInterval;
  measurement["phTdsInterval"] = phTdsInterval;
  measurement["phOnTime"] = phOnTime;
  measurement["tdsOnTime"] = tdsOnTime;

  serializeJsonPretty(doc, Serial);

  File f = HYDROPONICS_FS.open("/cfg.json", "w");
  if (f)
    serializeJson(doc, f);
  f.close();
  releaseJSONBufferLock();

  doSerializeConfig = false;
}

// settings in /wsec.json, not accessible via webserver, for passwords and tokens
bool deserializeConfigSec()
{
  if (!requestJSONBufferLock(3))
    return false;

  bool success = readObjectFromFile("/wsec.json", nullptr, &doc);
  if (!success)
  {
    releaseJSONBufferLock();
    return false;
  }

  JsonObject nw_ins_0 = doc["nw"]["ins"][0];
  getStringFromJson(clientPass, nw_ins_0["psk"], 65);

  JsonObject ap = doc["ap"];
  getStringFromJson(apPass, ap["psk"], 65);

  JsonObject if_mqtt = doc["mqtt"];
  getStringFromJson(mqttPass, if_mqtt["psk"], 65);

  releaseJSONBufferLock();

  serializeJsonPretty(doc, Serial);
  return true;
}

void serializeConfigSec()
{
  DEBUG_PRINTLN(F("Writing settings to /wsec.json..."));

  if (!requestJSONBufferLock(4))
    return;

  JsonObject nw = doc.createNestedObject("nw");

  JsonArray nw_ins = nw.createNestedArray("ins");

  JsonObject nw_ins_0 = nw_ins.createNestedObject();
  nw_ins_0["psk"] = clientPass;

  JsonObject ap = doc.createNestedObject("ap");
  ap["psk"] = apPass;

  JsonObject if_mqtt = doc.createNestedObject("mqtt");
  if_mqtt["psk"] = mqttPass;

  File f = HYDROPONICS_FS.open("/wsec.json", "w");
  if (f)
    serializeJson(doc, f);
  f.close();
  releaseJSONBufferLock();
}
