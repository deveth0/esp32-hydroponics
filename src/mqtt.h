#ifndef HYDROPONICS_MQTT_H
#define HYDROPONICS_MQTT_H

bool initMqtt();
void publishMqtt(const char *topic, const char *state);
void _createMqttSensor(const String &name, const String &deviceTopic, const String &topic, const String &deviceClass, const String &unitOfMeasurement);
void _createBinaryMqttSensor(const String &name, const String &deviceTopic, const String &topic, const String &deviceClass
);

#endif