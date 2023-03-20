#define HYDROPONICS_DEFINE_GLOBAL_VARS // only in one source file, main.cpp!

#include "main.h"
#include <Arduino.h>

Hydroponics::Hydroponics()
{
}

void Hydroponics::reset()
{
  ESP.restart();
}

void Hydroponics::loop()
{
  Serial.println("LED is on");
  yield();
  delay(1000);
}

void Hydroponics::setup()
{
  Serial.begin(115200);
}

