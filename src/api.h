
#ifndef HYDROPONICS_API_H
#define HYDROPONICS_API_H

void initApi();

/**
 * Return json with the current status
 */
void handleApiStatus(AsyncWebServerRequest *request);
void handleApiPumpPOST(AsyncWebServerRequest *request, JsonVariant &json);

/**
 * Read / write sensors config
*/
void handleApiConfigSensors(AsyncWebServerRequest *request);
void handleApiConfigSensorsPOST(AsyncWebServerRequest *request, JsonVariant &json);

/**
 * Read / write pump config
*/
void handleApiConfigPump(AsyncWebServerRequest *request);
void handleApiConfigPumpPOST(AsyncWebServerRequest *request, JsonVariant &json);

void handleWiFiNetworkList(AsyncWebServerRequest *request);

template <class T>
void addSensorStatus(JsonObject jsonObj, String sensorName, String sensorUnit, T lastValue);
void addPumpConfigEntry(JsonObject jsonObj, String name, u_int interval, u_int duration);
#endif