#ifndef HYDROPONICS_MQTT_H
#define HYDROPONICS_MQTT_H

bool initMqtt();
void publishMqtt(const char *topic, const char* state);
void _createMqttSensor(const String &name, const String &topic, const String &deviceClass, const String &unitOfMeasurement);

#endif