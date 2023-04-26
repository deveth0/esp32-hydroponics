
#ifndef HYDROPONICS_PUMP_H
#define HYDROPONICS_PUMP_H

class PumpHandler
{

public:
  PumpHandler();
  static PumpHandler &instance()
  {
    static PumpHandler instance;
    return instance;
  }

  void handlePump();
  long pumpRunUntil = 0;

private:
  long lastPumpRun = 0;
  long pumpStartTankLevel = 0;
  long timer;

  void disablePump();
  /**
   * Enable pump for x milliseconds
   */
  void enablePump(long duration);
};
#endif