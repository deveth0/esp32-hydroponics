#ifndef HYDROPONICS_H
#define HYDROPONICS_H

#define VERSION 2303200

#include <Arduino.h>
#include <HardwareSerial.h> // ensure we have the correct "Serial" on new MCUs (depends on ARDUINO_USB_MODE and ARDUINO_USB_CDC_ON_BOOT)
#include <WiFi.h>
#include "esp_wifi.h"
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <Wire.h>
#include <SPI.h>

#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>

#include "const.h"

#ifndef CLIENT_SSID
#define CLIENT_SSID DEFAULT_CLIENT_SSID
#endif

#ifndef CLIENT_PASS
#define CLIENT_PASS ""
#endif

#if defined(HYDROPONICS_AP_PASS) && !defined(HYDROPONICS_AP_SSID)
#error HYDROPONICS_AP_PASS is defined but HYDROPONICS_AP_SSID is still the default. \
         Please change HYDROPONICS_AP_SSID to something unique.
#endif

#ifndef HYDROPONICS_AP_SSID
#define HYDROPONICS_AP_SSID DEFAULT_AP_SSID
#endif

#ifndef HYDROPONICS_AP_PASS
#define HYDROPONICS_AP_PASS DEFAULT_AP_PASS
#endif

class Hydroponics
{
public:
  Hydroponics();
  static Hydroponics &instance()
  {
    static Hydroponics instance;
    return instance;
  }

  // boot starts here
  void setup();

  void loop();
  void reset();

};
#endif // MAIN_H
