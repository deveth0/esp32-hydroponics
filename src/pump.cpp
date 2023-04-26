
#include "main.h"

PumpHandler::PumpHandler()
{
}

void PumpHandler::handlePump()
{

  if (!pumpEnabled)
  {
    // ensure pump is stopped
    digitalWrite(PUMP_MOSFET_PIN, LOW);
    return;
  }

  if (pumpRunUntil > 0)
  {
    if (pumpRunUntil <= millis())
    {
      disablePump();
      pumpStatus = SCHEDULED_STOP;
      return;
    }
    if (lastDistance == __INT_MAX__ || SensorsHandler::instance().getTankVolume() == 0)
    {
      // no clue about the tank, disable pump
      pumpStatus = UNKNOWN_TANK_VOLUME;
      disablePump();
      return;
    }
    float fillHeight = tankHeight - lastDistance;
    if (fillHeight <= pumpStartTankLevel - maxWaterLevelDifferenceCm)
    {
      // water level falls to rapidly
      disablePump();
      pumpStatus = EMERGENCY_PUMP_STOP;
      // disable pump until next restart
      pumpEnabled = false;
      return;
    }

    return;
  }

  if (lastTemperature == __FLT_MAX__)
  {
    // no temperature available
    return;
  }

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
    pumpStatus = SCHEDULED_RUN;
    enablePump(duration * 60000);
  }
}

void PumpHandler::disablePump()
{
  digitalWrite(PUMP_MOSFET_PIN, LOW);
  publishMqtt("pump", "OFF");
  pumpRunUntil = 0;
}

void PumpHandler::enablePump(long duration)
{
  if (lastDistance == __INT_MAX__ || SensorsHandler::instance().getTankVolume() == 0)
  {
    // no clue about the tank
    return;
  }

  float fillHeight = tankHeight - lastDistance;

  if (fillHeight < minWaterLevelCm)
  {
    // not enough water
    return;
  }

  lastPumpRun = millis();
  pumpRunUntil = lastPumpRun + duration;
  pumpStartTankLevel = fillHeight;
  digitalWrite(PUMP_MOSFET_PIN, HIGH);
  publishMqtt("pump", "ON");
}
