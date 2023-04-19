#include "main.h"

/*
 * MQTT communication protocol for home automation
 */

#define MQTT_KEEP_ALIVE_TIME 60 // contact the MQTT broker every 60 seconds

void parseMQTTBriPayload(char *payload)
{
}

void onMqttConnect(bool sessionPresent)
{
  DEBUG_PRINTLN("onMqttConnect");
  //(re)subscribe to required topics
  char subuf[38];

  if (mqttDeviceTopic[0] != 0)
  {
    strlcpy(subuf, mqttDeviceTopic, 33);
    mqtt->subscribe(subuf, 0);
    strcat_P(subuf, PSTR("/col"));
    mqtt->subscribe(subuf, 0);
    strlcpy(subuf, mqttDeviceTopic, 33);
    strcat_P(subuf, PSTR("/api"));
    mqtt->subscribe(subuf, 0);
  }

  if (mqttGroupTopic[0] != 0)
  {
    strlcpy(subuf, mqttGroupTopic, 33);
    mqtt->subscribe(subuf, 0);
    strcat_P(subuf, PSTR("/col"));
    mqtt->subscribe(subuf, 0);
    strlcpy(subuf, mqttGroupTopic, 33);
    strcat_P(subuf, PSTR("/api"));
    mqtt->subscribe(subuf, 0);
  }

  doPublishMqtt = true;
  DEBUG_PRINTLN(F("MQTT ready"));

  _createMqttSensor(F("Water"), mqttDeviceTopic, "water", "temperature", F("°C"));
  _createMqttSensor(F("Temperature"), mqttDeviceTopic, "temperature", "temperature", F("°C"));
  _createMqttSensor(F("Distance"), mqttDeviceTopic, "distance", "distance", F("cm"));
  _createMqttSensor(F("Volume"), mqttDeviceTopic, "volume", "water", F("L"));
  _createMqttSensor(F("Pressure"), mqttDeviceTopic, "pressure", "pressure", F("Pa"));
  _createMqttSensor(F("TDS"), mqttDeviceTopic, "tds", "", F("ppm"));
  _createMqttSensor(F("EC"), mqttDeviceTopic, "ec", "", F("uS/cm"));
  _createMqttSensor(F("TDSVoltage"), mqttDeviceTopic, "tdsVoltage", "voltage", F("V"));
  _createMqttSensor(F("PH"), mqttDeviceTopic, "ph", "", F("pH"));
  _createMqttSensor(F("PHVoltage"), mqttDeviceTopic, "phVoltage", "voltage", F("V"));
  _createBinaryMqttSensor(F("Pump"), mqttDeviceTopic, "pump", "running");
}

// Create an MQTT Sensor for Home Assistant Discovery purposes, this includes a pointer to the topic that is published to in the Loop.
void _createMqttSensor(const String &name, const String &deviceTopic, const String &topic, const String &deviceClass, const String &unitOfMeasurement)
{
  String mqttTopic = deviceTopic + "/" + topic;
  String t = String(F("homeassistant/sensor/")) + mqttClientID + F("/") + name + F("/config");

  StaticJsonDocument<600> doc;

  doc[F("name")] = String(serverDescription) + " " + name;
  doc[F("state_topic")] = mqttTopic;
  doc[F("unique_id")] = String(mqttClientID) + name;
  if (unitOfMeasurement != "")
    doc[F("unit_of_measurement")] = unitOfMeasurement;
  if (deviceClass != "")
    doc[F("device_class")] = deviceClass;

  JsonObject device = doc.createNestedObject(F("device")); // attach the sensor to the same device
  device[F("name")] = serverDescription;
  device[F("identifiers")] = "hydroponics-sensor-" + String(mqttClientID);
  device[F("manufacturer")] = F("deveth0");
  device[F("model")] = F("esp32-hydroponics");
  device[F("sw_version")] = versionString;

  String temp;
  serializeJson(doc, temp);
  DEBUG_PRINTLN(t);
  DEBUG_PRINTLN(temp);

  mqtt->publish(t.c_str(), 0, true, temp.c_str());
}

// Create an MQTT Sensor for Home Assistant Discovery purposes, this includes a pointer to the topic that is published to in the Loop.
void _createBinaryMqttSensor(const String &name, const String &deviceTopic, const String &topic, const String &deviceClass)
{
  String mqttTopic = deviceTopic + "/" + topic;
  String t = String(F("homeassistant/binary_sensor/")) + mqttClientID + F("/") + name + F("/config");

  StaticJsonDocument<600> doc;

  doc[F("name")] = String(serverDescription) + " " + name;
  doc[F("state_topic")] = mqttTopic;
  doc[F("unique_id")] = String(mqttClientID) + name;
  if (deviceClass != "")
    doc[F("device_class")] = deviceClass;

  JsonObject device = doc.createNestedObject(F("device")); // attach the sensor to the same device
  device[F("name")] = serverDescription;
  device[F("identifiers")] = "hydroponics-sensor-" + String(mqttClientID);
  device[F("manufacturer")] = F("deveth0");
  device[F("model")] = F("esp32-hydroponics");
  device[F("sw_version")] = versionString;

  String temp;
  serializeJson(doc, temp);
  DEBUG_PRINTLN(t);
  DEBUG_PRINTLN(temp);

  mqtt->publish(t.c_str(), 0, true, temp.c_str());
}

void publishMqtt(const char *topic, const char *state)
{

  // Check if MQTT Connected, otherwise it will crash the 8266
  if (HYDROPONICS_MQTT_CONNECTED)
  {
    char subuf[128];
    snprintf_P(subuf, 127, PSTR("%s/%s"), mqttDeviceTopic, topic);
    DEBUG_PRINTF("Publish mqtt %s - %s\n", subuf, state);
    mqtt->publish(subuf, 0, false, state);
  }
  else
  {
    DEBUG_PRINTLN("MQTT not connected");
  }
}

bool initMqtt()
{
  DEBUG_PRINTLN("initMqtt");
  if (!mqttEnabled || mqttServer[0] == 0 || !HYDROPONICS_CONNECTED)
  {
    DEBUG_PRINTLN("initMqtt failed");
    return false;
  }

  if (mqtt == nullptr)
  {
    mqtt = new AsyncMqttClient();
    mqtt->onConnect(onMqttConnect);
  }
  if (mqtt->connected())
  {
    DEBUG_PRINTLN("mqtt connected");
    return true;
  }

  DEBUG_PRINTLN(F("Reconnecting MQTT"));
  IPAddress mqttIP;
  if (mqttIP.fromString(mqttServer)) // see if server is IP or domain
  {
    mqtt->setServer(mqttIP, mqttPort);
  }
  else
  {
    mqtt->setServer(mqttServer, mqttPort);
  }
  mqtt->setClientId(mqttClientID);
  if (mqttUser[0] && mqttPass[0])
    mqtt->setCredentials(mqttUser, mqttPass);

  mqtt->setKeepAlive(MQTT_KEEP_ALIVE_TIME);
  mqtt->connect();
  return true;
}
