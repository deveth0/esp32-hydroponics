
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

  float getTankVolume();
  float getCurrentTankLevel();

private:
  OneWire oneWire;
  DallasTemperature dallasTemperature;
  Adafruit_BMP280 bmp280;
  bool phMeasure;
  long timer;


  float readWaterTemperatureValue();
  float readPhValue();
  float readTDSValue();
};
#endif