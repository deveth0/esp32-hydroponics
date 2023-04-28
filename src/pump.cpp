
#include "main.h"

PumpHandler::PumpHandler()
{
}

bool PumpHandler::pumpRunning(){
  return _pumpRunning;
}

void PumpHandler::manualPumpRun(int duration){
  pumpStatus = MANUAL_RUN;
  enablePump(duration);
}

void PumpHandler::handlePump()
{

  if (!pumpEnabled)
  {
    // ensure pump is stopped
    digitalWrite(PUMP_MOSFET_PIN, LOW);
    _pumpRunning = false;
    return;
  }
  float fillHeight = tankHeight - lastDistance;

  // check stop states
  if (_pumpRunning)
  {
    if (lastDistance == __INT_MAX__ || SensorsHandler::instance().getTankVolume() == 0)
    {
      // no clue about the tank, disable pump
      pumpStatus = UNKNOWN_TANK_VOLUME;
      disablePump();
      return;
    }
    if (fillHeight < minWaterLevelCm)
    {
      // not enough water
      disablePump();
      pumpStatus = NOT_ENOUGH_WATER_STOP;
      pumpEnabled = false;
      return;
    }
  }

  // emptying run, no need to do anything
  if(pumpRunUntil == -1 ) return;

  if (pumpRunUntil > 0)
  {
    if (pumpRunUntil <= millis())
    {
      disablePump();
      pumpStatus = SCHEDULED_STOP;
      return;
    }

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
  _pumpRunning = false;
  publishMqtt("pump", "OFF");
  pumpRunUntil = 0;
}

void PumpHandler::enablePump(long duration) 
{
  if (lastDistance == __INT_MAX__ || SensorsHandler::instance().getTankVolume() == 0)
  {
    // no clue about the tank
    pumpStatus = UNKNOWN_TANK_VOLUME;
    return;
  }

  float fillHeight = tankHeight - lastDistance;

  if (fillHeight < minWaterLevelCm)
  {
    // not enough water to start
    pumpStatus = NOT_ENOUGH_WATER_STOP;
    return;
  }

  lastPumpRun = millis();
  if (duration == -1)
  {
    pumpRunUntil = -1;
  }
  else
  {
    pumpRunUntil = lastPumpRun + duration;
  }
  pumpStartTankLevel = fillHeight;
  digitalWrite(PUMP_MOSFET_PIN, HIGH);
  _pumpRunning = true;
  publishMqtt("pump", "ON");
}
