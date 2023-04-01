
#include "main.h"

PumpHandler::PumpHandler()
{
}

void PumpHandler::handlePump()
{
  if (lastTemperature == __FLT_MAX__)
    return;
  // minutes since last run
  timer = (millis() - lastPumpRun) / 60000;

  uint interval;
  uint duration;

  if (lastTemperature <= 10)
  {
    interval = pumpLe10Interval;
    duration = pumpLe10Duration;
  }
  else if (lastTemperature <= 15)
  {
    interval = pumpLe15Interval;
    duration = pumpLe15Duration;
  }
  else if (lastTemperature <= 20)
  {
    interval = pumpLe20Interval;
    duration = pumpLe20Duration;
  }
  else if (lastTemperature <= 25)
  {
    interval = pumpLe25Interval;
    duration = pumpLe25Duration;
  }
  else
  {
    interval = pumpGt25Interval;
    duration = pumpGt25Duration;
  }

  if (interval > 0 && timer > interval)
  {
    DEBUG_PRINTF("Last pump run %d minutes ago, starting for %d minutes...\n", timer, duration);
    lastPumpRun = millis();
    pumpRunUntil = lastPumpRun + duration * 60000;
    digitalWrite(PUMP_MOSFET_PIN, HIGH);
    publishMqtt("pump", "ON");
  }

  if (pumpRunUntil > 0 && pumpRunUntil <= millis())
  {
    DEBUG_PRINTF("Stopping pump");
    digitalWrite(PUMP_MOSFET_PIN, LOW);
    publishMqtt("pump", "OFF");
    pumpRunUntil = 0;
  }
}
