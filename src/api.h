
#ifndef HYDROPONICS_API_H
#define HYDROPONICS_API_H



void initApi();

/**
 * Return json with the current status
 */
void handleApiStatus(AsyncWebServerRequest *request);

void handleWiFiNetworkList(AsyncWebServerRequest *request);

template<class T>
void addSensor(JsonObject jsonObj, String sensorName, String sensorUnit, T lastValue);

#endif