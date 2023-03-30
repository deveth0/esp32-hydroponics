
#ifndef HYDROPONICS_API_H
#define HYDROPONICS_API_H

/**
 * Return json with the current status
 */
void handleApiStatus(AsyncWebServerRequest *request);

template<class T>
void addSensor(JsonObject jsonObj, String sensorName, String sensorUnit, T lastValue, long lastMeasure);

#endif