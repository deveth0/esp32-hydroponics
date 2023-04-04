
#include "main.h"

SensorsHandler::SensorsHandler()
{
  oneWire = OneWire(TEMP_PIN);
  dallasTemperature = DallasTemperature(&oneWire);
  bmp280 = Adafruit_BMP280();

  if (bmp280.begin(BMP280_ADDRESS, BMP280_CHIPID))
  {
    DEBUG_PRINTLN("BMP280 initalized");
    bmp280Initialized = true;
    bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                       Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                       Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                       Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                       Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }
  else
  {
    bmp280Initialized = false;
    DEBUG_PRINTLN("Could not initalize BMP280");
  }
}

void SensorsHandler::initSensors()
{
  dallasTemperature.begin();
  pinMode(PH_PIN, INPUT);
  pinMode(TDS_PIN, INPUT);
  pinMode(PH_MOSFET_PIN, OUTPUT);
  pinMode(TDS_MOSFET_PIN, OUTPUT);
  pinMode(PUMP_MOSFET_PIN, OUTPUT);

  pinMode(DISTANCE_PIN_TRIGGER, OUTPUT);
  pinMode(DISTANCE_PIN_ECHO, INPUT);
}

void SensorsHandler::handleSensors()
{
  timer = millis();
  if (timer - lastTemperatureMeasure >= TEMPERATURE_INTERVAL * 1000)
  {
    lastTemperatureMeasure = timer;
    float temperatureC = 0;

    for (int i = 0; i < NUMBER_MEASUREMENTS; i++)
    {
      dallasTemperature.requestTemperatures();
      temperatureC += dallasTemperature.getTempCByIndex(0);
    }

    temperatureC = roundf((temperatureC / NUMBER_MEASUREMENTS) * 10) / 10;
    // calibration
    temperatureC = temperatureC + waterTempAdjustment;

    if (temperatureC != lastWaterTemperature)
    {
      DEBUG_PRINTF("new water temperature %f °C\n", temperatureC);
      publishMqtt("water", String(temperatureC, 2).c_str());
      lastWaterTemperature = temperatureC;
    }

    if (bmp280Initialized)
    {

      temperatureC = roundf((bmp280.readTemperature() * 10) / 10);
      temperatureC = temperatureC + tempAdjustment;
      float pressure = roundf((bmp280.readPressure() * 0.1) / 10);
      if (temperatureC != lastTemperature)
      {
        DEBUG_PRINTF("new temperature %f °C\n", temperatureC);
        publishMqtt("temperature", String(temperatureC, 2).c_str());
        lastTemperature = temperatureC;
      }
      if (pressure != lastPressure)
      {
        DEBUG_PRINTF("new pressure %f Pa\n", pressure);
        publishMqtt("pressure", String(pressure, 2).c_str());
        lastPressure = pressure;
      }
    }
  }

  if (timer - lastDistanceMeasure >= DISTANCE_INTERVAL * 1000)
  {
    lastDistanceMeasure = timer;

    digitalWrite(DISTANCE_PIN_TRIGGER, LOW);
    delayMicroseconds(2);

    digitalWrite(DISTANCE_PIN_TRIGGER, HIGH);
    delayMicroseconds(10);

    digitalWrite(DISTANCE_PIN_TRIGGER, LOW);

    u_long duration = pulseIn(DISTANCE_PIN_ECHO, HIGH);
    u_int distance = (duration * .0343) / 2;

    if (distance > 0 && distance < DISTANCE_MAX && distance != lastDistance)
    {
      DEBUG_PRINTF("new distance %d cm\n", distance);
      publishMqtt("distance", String(distance).c_str());
      lastDistance = distance;

      if (tankWidth != 0 && tankHeight != 0 && tankLength != 0)
      {
        float fillHeight = tankHeight - lastDistance;

        lastVolume = (fillHeight * tankLength * tankWidth) / 1000;
        lastVolume = roundf(lastVolume);

        DEBUG_PRINTF("new volume %d L\n", lastVolume);
        publishMqtt("volume", String(lastVolume).c_str());
      }
    }
  }

  if (timer - lastPhTdsMeasure >= PH_TDS_INTERVAL * 1000)
  {
    if (phMeasure)
    {
      // check, if the mosfet was not activated yet
      if (digitalRead(PH_MOSFET_PIN) == LOW)
      {
        DEBUG_PRINTLN("Activating ph mosfet");
        digitalWrite(PH_MOSFET_PIN, HIGH);

        lastPhTdsOnSwitch = timer;
      }
      // check if the waiting period is over and we can take a measurement
      if (timer - lastPhTdsOnSwitch >= PH_ON_TIME * 1000)
      {
        DEBUG_PRINTLN("Finished waiting for ph sensor to heat up");
        lastPhTdsMeasure = timer;

        float phValue = readPhValue();

        if (phValue != lastPh)
        {
          DEBUG_PRINTF("new ph %f\n", phValue);
          publishMqtt("ph", String(phValue, 2).c_str());
          lastPh = phValue;
        }

        // cleanup and deactivate ph mosfet
        phMeasure = false;
        digitalWrite(PH_MOSFET_PIN, LOW);
      }
    }
    else
    {
      // check, if the mosfet was not activated yet
      if (digitalRead(TDS_MOSFET_PIN) == LOW)
      {
        DEBUG_PRINTLN("Activating tds mosfet");
        digitalWrite(TDS_MOSFET_PIN, HIGH);
        lastPhTdsOnSwitch = timer;
      }
      // check if the waiting period is over and we can take a measurement
      if (timer - lastPhTdsOnSwitch >= TDS_ON_TIME * 1000)
      {
        DEBUG_PRINTLN("Perform tds measurement");
        lastPhTdsMeasure = timer;

        float tdsValue = readTDSValue();

        if (tdsValue != lastTds)
        {
          DEBUG_PRINTF("new tds %f ppm\n", tdsValue);
          publishMqtt("tds", String(tdsValue, 2).c_str());
          lastTds = tdsValue;
        }
        phMeasure = true;
        digitalWrite(TDS_MOSFET_PIN, LOW);
      }
    }
  }
}

float SensorsHandler::readPhValue()
{
  float slope = (7.0 - 4.0) / ((phNeutralVoltage - 1500.0) / 3.0 - (phAcidVoltage - 1500.0) / 3.0); // two point: (_neutralVoltage,7.0),(_acidVoltage,4.0)
  float intercept = 7.0 - slope * (phNeutralVoltage - 1500.0) / 3.0;

  lastPhVoltage = readAverage(PH_PIN, NUMBER_MEASUREMENTS) * ((float)3.3 / 4095.0);

  DEBUG_PRINTF("lastPhVoltage: %f", lastPhVoltage);

  float phValue = slope * ((lastPhVoltage * 1000) - 1500.0) / 3.0 + intercept;
  return roundf((phValue)*10) / 10;
}

float SensorsHandler::readTDSValue()
{
  float tdsRead = readAverage(TDS_PIN, NUMBER_MEASUREMENTS);

  float averageVoltage = tdsRead * ((float)3.3 / 4095.0); // read the analog value more stable by the median filtering algorithm, and convert to voltage value

  float compensationCoefficient = 1.0 + 0.02 * (lastTemperature - 25.0);                                                                                                                 // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVolatge = averageVoltage / compensationCoefficient;                                                                                                                  // temperature compensation
  float tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; // convert voltage value to tds value

  tdsValue = roundf(tdsValue / 100) * 100;
  return tdsValue;
}