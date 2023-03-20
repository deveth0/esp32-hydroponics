#include "main.h"

void setup() {
  Hydroponics::instance().setup();
}

void loop() {
  Hydroponics::instance().loop();
}
