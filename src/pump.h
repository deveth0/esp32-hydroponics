
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

private:
  long lastPumpRun = 0;
  long pumpRunUntil = 0;
  long timer;
};
#endif