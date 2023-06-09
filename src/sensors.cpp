
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

float SensorsHandler::getTankVolume()
{
  if (tankWidth != 0 && tankHeight != 0 && tankLength != 0)
    return (tankHeight * tankLength * tankWidth) / 1000;

  return 0;
}

float SensorsHandler::getCurrentTankLevel()
{
  if (lastDistance == __INT_MAX__ || tankHeight == 0)
    return 0;

  float fillHeight = tankHeight - lastDistance;

  return roundf((fillHeight * tankLength * tankWidth) / 1000);
}

void SensorsHandler::handleSensors()
{
  timer = millis();
  if (timer - lastTemperatureMeasure >= temperatureInterval * 1000)
  {
    lastTemperatureMeasure = timer;
    float temperatureC = readWaterTemperatureValue();

    if (temperatureC != lastWaterTemperature)
    {
      // retry if difference is too large to smooth curve
      if (fabs(lastWaterTemperature - temperatureC) > 1)
      {
        temperatureC = readWaterTemperatureValue();
      }

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

  if (timer - lastDistanceMeasure >= distanceInterval * 1000 || PumpHandler::instance().pumpRunning())
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

        lastVolume = getCurrentTankLevel();

        float maxVolume = getTankVolume();

        lastWaterLevel = round((100 * lastVolume) / maxVolume);

        DEBUG_PRINTF("new volume %d L\n", lastVolume);
        publishMqtt("volume", String(lastVolume).c_str());
        publishMqtt("waterLevel", String(lastWaterLevel).c_str());
      }
    }
    else
    {
      sensorsStatus = INVALID_DISTANCE;
    }
  }

  if (timer - lastPhTdsMeasure >= phTdsInterval * 1000)
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
      if (timer - lastPhTdsOnSwitch >= phOnTime * 1000)
      {
        DEBUG_PRINTLN("Finished waiting for ph sensor to heat up");
        lastPhTdsMeasure = timer;

        float phValue = readPhValue();

        if (phValue != lastPh)
        {
          DEBUG_PRINTF("new ph %f\n", phValue);
          publishMqtt("ph", String(phValue, 2).c_str());
          publishMqtt("phVoltage", String(lastPhVoltage, 2).c_str());
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
        pinMode(TDS_PIN, INPUT);
        lastPhTdsOnSwitch = timer;
      }
      // check if the waiting period is over and we can take a measurement
      if (timer - lastPhTdsOnSwitch >= tdsOnTime * 1000)
      {
        DEBUG_PRINTLN("Perform tds measurement");
        lastPhTdsMeasure = timer;

        float tdsValue = readTDSValue();

        if (tdsValue != lastTds)
        {
          DEBUG_PRINTF("new tds %f ppm\n", tdsValue);
          lastTds = tdsValue;
          lastEc = tdsValue * 2;
          publishMqtt("tds", String(tdsValue, 2).c_str());
          publishMqtt("tdsVoltage", String(lastTdsVoltage, 2).c_str());
          publishMqtt("ec", String(lastEc * 2, 2).c_str());
        }
        phMeasure = true;
        digitalWrite(TDS_MOSFET_PIN, LOW);
        pinMode(TDS_PIN, INPUT_PULLDOWN);
      }
    }
  }
}

float SensorsHandler::readPhValue()
{
  float slope = (7.0 - 4.0) / ((phNeutralVoltage - 1500.0) / 3.0 - (phAcidVoltage - 1500.0) / 3.0); // two point: (_neutralVoltage,7.0),(_acidVoltage,4.0)
  float intercept = 7.0 - slope * (phNeutralVoltage - 1500.0) / 3.0;

  lastPhVoltage = readAverage(PH_PIN, numberMeasurements) * ((float)3.3 / 4095.0);

  DEBUG_PRINTF("lastPhVoltage: %f", lastPhVoltage);

  float phValue = slope * ((lastPhVoltage * 1000) - 1500.0) / 3.0 + intercept;
  return roundf((phValue)*10) / 10;
}

float SensorsHandler::readTDSValue()
{
  float tdsRead = readAverage(TDS_PIN, numberMeasurements);

  lastTdsVoltage = tdsRead * ((float)3.3 / 4095.0); // read the analog value more stable by the median filtering algorithm, and convert to voltage value

  float compensationCoefficient = 1.0 + 0.02 * (lastTemperature - 25.0);                                                                                                                 // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVolatge = lastTdsVoltage / compensationCoefficient;                                                                                                                  // temperature compensation
  float tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; // convert voltage value to tds value

  tdsValue = roundf(tdsValue / 10) * 10;
  return tdsValue;
}

float SensorsHandler::readWaterTemperatureValue()
{
  float temperatureC = 0;
  for (int i = 0; i < numberMeasurements; i++)
  {
    dallasTemperature.requestTemperatures();
    temperatureC += dallasTemperature.getTempCByIndex(0);
  }

  temperatureC = roundf((temperatureC / numberMeasurements) * 10) / 10;
  // calibration
  return temperatureC + waterTempAdjustment;
}