
#ifndef HYDROPONICS_API_H
#define HYDROPONICS_API_H

void initApi();

/**
 * Return json with the current status
 */
void handleApiStatus(AsyncWebServerRequest *request);
void handleApiConfig(AsyncWebServerRequest *request);
void handleApiConfigPOST(AsyncWebServerRequest *request, JsonVariant &json);

void handleWiFiNetworkList(AsyncWebServerRequest *request);

template <class T>
void addSensorStatus(JsonObject jsonObj, String sensorName, String sensorUnit, T lastValue);
void addPumpConfigEntry(JsonObject jsonObj, String name, u_int interval, u_int duration);
#endif