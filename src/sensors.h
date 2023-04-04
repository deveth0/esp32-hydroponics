
#ifndef HYDROPONICS_SENSORS_H
#define HYDROPONICS_SENSORS_H

class SensorsHandler
{

public:
  SensorsHandler();
  static SensorsHandler &instance()
  {
    static SensorsHandler instance;
    return instance;
  }

  void initSensors();
  void handleSensors();

private:
  OneWire oneWire;
  DallasTemperature dallasTemperature;
  Adafruit_BMP280 bmp280;
  bool phMeasure;
  long timer;

  float readPhValue();
  float readTDSValue();
};
#endif