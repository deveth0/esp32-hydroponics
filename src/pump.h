
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
  void manualPumpRun(int duration);
  bool pumpRunning();
  int pumpStartTankLevel();
  int pumpEndTankLevel();
  long pumpRunUntil = 0;

private:
  long lastPumpRun = 0;
  int _pumpStartTankLevel = 0;
  int _pumpEndTankLevel = 0;
  long timer;
  bool _pumpRunning = false;

  void disablePump();
  /**
   * Enable pump for x milliseconds
   */
  void enablePump(long duration);
};
#endif