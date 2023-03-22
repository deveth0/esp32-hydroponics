# 1 "C:\\Users\\deveth0\\AppData\\Local\\Temp\\tmpk3bc10hn"
#include <Arduino.h>
# 1 "D:/Develop/esp32-hydroponics/src/hydroponics.ino"
#include "main.h"
void setup();
void loop();
#line 3 "D:/Develop/esp32-hydroponics/src/hydroponics.ino"
void setup() {
  Hydroponics::instance().setup();
}

void loop() {
  Hydroponics::instance().loop();
}