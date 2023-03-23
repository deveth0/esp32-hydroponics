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

  char mqttTemperatureTopic[128];
  char mqttHumidityTopic[128];
  char mqttPressureTopic[128];
  char mqttHeatIndexTopic[128];
  char mqttDewPointTopic[128];
  snprintf_P(mqttTemperatureTopic, 127, PSTR("%s/temperature"), mqttDeviceTopic);
  snprintf_P(mqttPressureTopic, 127, PSTR("%s/pressure"), mqttDeviceTopic);
  snprintf_P(mqttHumidityTopic, 127, PSTR("%s/humidity"), mqttDeviceTopic);
  snprintf_P(mqttHeatIndexTopic, 127, PSTR("%s/heat_index"), mqttDeviceTopic);
  snprintf_P(mqttDewPointTopic, 127, PSTR("%s/dew_point"), mqttDeviceTopic);

  _createMqttSensor(F("Temperature"), mqttTemperatureTopic, "temperature", F("°C"));
}

// Create an MQTT Sensor for Home Assistant Discovery purposes, this includes a pointer to the topic that is published to in the Loop.
void _createMqttSensor(const String &name, const String &topic, const String &deviceClass, const String &unitOfMeasurement)
{
  String t = String(F("homeassistant/sensor/")) + mqttClientID + F("/") + name + F("/config");

  StaticJsonDocument<600> doc;

  doc[F("name")] = String(serverDescription) + " " + name;
  doc[F("state_topic")] = topic;
  doc[F("unique_id")] = String(mqttClientID) + name;
  if (unitOfMeasurement != "")
    doc[F("unit_of_measurement")] = unitOfMeasurement;
  if (deviceClass != "")
    doc[F("device_class")] = deviceClass;
  doc[F("expire_after")] = 1800;

  JsonObject device = doc.createNestedObject(F("device")); // attach the sensor to the same device
  device[F("name")] = serverDescription;
  device[F("identifiers")] = "wled-sensor-" + String(mqttClientID);
  device[F("manufacturer")] = F("WLED");
  device[F("model")] = F("FOSS");
  device[F("sw_version")] = versionString;

  String temp;
  serializeJson(doc, temp);
  DEBUG_PRINTLN(t);
  DEBUG_PRINTLN(temp);

  mqtt->publish(t.c_str(), 0, true, temp.c_str());
}

void publishMqtt(const char *topic, const char* state) {
      DEBUG_PRINTF("Publish mqtt %s\n", state);
      //Check if MQTT Connected, otherwise it will crash the 8266
      if (HYDROPONICS_MQTT_CONNECTED){
        char subuf[128];
        snprintf_P(subuf, 127, PSTR("%s/%s"), mqttDeviceTopic, topic);
        mqtt->publish(subuf, 0, false, state);
      } else {
        DEBUG_PRINTLN("MQTT not connected");
      }
    }

// HA autodiscovery was removed in favor of the native integration in HA v0.102.0
bool initMqtt()
{
  if (!mqttEnabled || mqttServer[0] == 0 || !HYDROPONICS_CONNECTED)
    return false;

  if (mqtt == nullptr)
  {
    mqtt = new AsyncMqttClient();
    mqtt->onConnect(onMqttConnect);
  }
  if (mqtt->connected())
    return true;

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
