#include "main.h"

void handleApiStatus(AsyncWebServerRequest *request)
{

  DynamicJsonDocument doc(1024);

  JsonObject sensors = doc.createNestedObject("sensors");
  long timer = millis();

  addSensor(sensors, F("distance"), F("cm"), lastDistance, timer - lastDistanceMeasure);
  addSensor(sensors, F("pressure"), F("Pa"), lastPressure, timer - lastTemperatureMeasure);
  addSensor(sensors, F("temperature"), F("°C"), lastTemperature, timer - lastTemperatureMeasure);
  addSensor(sensors, F("waterTemperature"), F("°C"), lastWaterTemperature, timer - lastTemperatureMeasure);
  addSensor(sensors, F("ph"), F("pH"), lastPh, timer - lastPhTdsMeasure);
  addSensor(sensors, F("tds"), F("ppm"), lastTds, timer - lastPhTdsMeasure);

  String data;
  serializeJson(doc, data);
  request->send(200, "application/json", data);
}

template <class T>
void addSensor(JsonObject jsonObj, String sensorName, String sensorUnit, T lastValue, long lastMeasure)
{
  JsonObject sensor = jsonObj.createNestedObject(sensorName);
  sensor["unit"] = sensorUnit;
  sensor["value"] = lastValue;
  sensor["lastMeasure"] = lastMeasure;
}