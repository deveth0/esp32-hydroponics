#ifndef HYDROPONICS_H
#define HYDROPONICS_H

#define VERSION 2303200

#define HYDROPONICS_DEBUG
#define HYDROPONICS_DEBUG_FS

#ifndef HYDROPONICS_WATCHDOG_TIMEOUT
// 3 seconds should be enough to detect a lockup
// define HYDROPONICS_WATCHDOG_TIMEOUT=0 to disable watchdog, default
#define HYDROPONICS_WATCHDOG_TIMEOUT 0
#endif

#include <Arduino.h>
#include <HardwareSerial.h> // ensure we have the correct "Serial" on new MCUs (depends on ARDUINO_USB_MODE and ARDUINO_USB_CDC_ON_BOOT)
#include <WiFi.h>
#include "esp_wifi.h"
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <Wire.h>
#include <SPI.h>
#include <LittleFS.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP280.h>

#include <ArduinoJson.h>
#include <AsyncMqttClient.h>
#include <TimeLib.h>

#include "dependencies/network/Network.h"
#include "dependencies/toki/Toki.h"

#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <DNSServer.h>

#include "const.h"
#include "math.h"
#include "util.h"
#include "improv.h"
#include "cfg.h"
#include "file.h"
#include "mqtt.h"
#include "web.h"
#include "api.h"
#include "sensors.h"
#include "pump.h"
#include "ntp.h"

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

#define HYDROPONICS_FS LittleFS

// GLOBAL VARIABLES
// both declared and defined in header (solution from http://www.keil.com/support/docs/1868.htm)
//
// e.g. byte test = 2 becomes HYDROPONICS_GLOBAL byte test _INIT(2);
//     int arr[]{0,1,2} becomes HYDROPONICS_GLOBAL int arr[] _INIT_N(({0,1,2}));

#ifndef HYDROPONICS_DEFINE_GLOBAL_VARS
#define HYDROPONICS_GLOBAL extern
#define _INIT(x)
#define _INIT_N(x)
#else
#define HYDROPONICS_GLOBAL
#define _INIT(x) = x
// needed to ignore commas in array definitions
#define UNPACK(...) __VA_ARGS__
#define _INIT_N(x) UNPACK x
#endif

#define STRINGIFY(X) #X
#define TOSTRING(X) STRINGIFY(X)

#ifndef HYDROPONICS_VERSION
#define HYDROPONICS_VERSION "dev"
#endif

// Global Variable definitions
HYDROPONICS_GLOBAL char versionString[] _INIT(TOSTRING(HYDROPONICS_VERSION));

// AP default passwords (for maximum security change them!)
HYDROPONICS_GLOBAL char apPass[65] _INIT(HYDROPONICS_AP_PASS);

// internal global variable declarations
// wifi
HYDROPONICS_GLOBAL bool apActive _INIT(false);
HYDROPONICS_GLOBAL bool forceReconnect _INIT(false);
HYDROPONICS_GLOBAL uint32_t lastReconnectAttempt _INIT(0);
HYDROPONICS_GLOBAL bool interfacesInited _INIT(false);
HYDROPONICS_GLOBAL bool wasConnected _INIT(false);

HYDROPONICS_GLOBAL char ntpServerName[33] _INIT("0.pool.ntp.org");

// WiFi CONFIG (all these can be changed via web UI, no need to set them here)
HYDROPONICS_GLOBAL char clientSSID[33] _INIT(CLIENT_SSID);
HYDROPONICS_GLOBAL char clientPass[65] _INIT(CLIENT_PASS);
HYDROPONICS_GLOBAL char cmDNS[33] _INIT("x");                            // mDNS address (placeholder, is replaced by wledXXXXXX.local)
HYDROPONICS_GLOBAL char apSSID[33] _INIT("");                            // AP off by default (unless setup)
HYDROPONICS_GLOBAL byte apChannel _INIT(1);                              // 2.4GHz WiFi AP channel (1-13)
HYDROPONICS_GLOBAL byte apHide _INIT(0);                                 // hidden AP SSID
HYDROPONICS_GLOBAL byte apBehavior _INIT(AP_BEHAVIOR_BOOT_NO_CONN);      // access point opens when no connection after boot by default
HYDROPONICS_GLOBAL IPAddress staticIP _INIT_N(((0, 0, 0, 0)));           // static IP of ESP
HYDROPONICS_GLOBAL IPAddress staticGateway _INIT_N(((0, 0, 0, 0)));      // gateway (router) IP
HYDROPONICS_GLOBAL IPAddress staticSubnet _INIT_N(((255, 255, 255, 0))); // most common subnet in home networks

// dns server
HYDROPONICS_GLOBAL DNSServer dnsServer;

enum NTPStatus
{
  SYNCED = 100,
  NTP_PACKET_SEND = 200,
  INVALID_NTP_PACKET = 201,
  INVALID_DEP_SEC = 202
};
// network time
HYDROPONICS_GLOBAL bool ntpConnected _INIT(false);
HYDROPONICS_GLOBAL time_t localTime _INIT(0);
HYDROPONICS_GLOBAL unsigned long ntpLastSync _INIT(999000000L);
HYDROPONICS_GLOBAL unsigned long ntpPacketSent _INIT(999000000L);
HYDROPONICS_GLOBAL time_t long ntpLastSyncTime _INIT(0);
HYDROPONICS_GLOBAL time_t long ntpPacketSentTime _INIT(0);
HYDROPONICS_GLOBAL IPAddress ntpServerIP;
HYDROPONICS_GLOBAL uint16_t ntpLocalPort _INIT(2390);
HYDROPONICS_GLOBAL uint16_t rolloverMillis _INIT(0);
HYDROPONICS_GLOBAL float longitude _INIT(0.0);
HYDROPONICS_GLOBAL float latitude _INIT(0.0);
HYDROPONICS_GLOBAL time_t sunrise _INIT(0);
HYDROPONICS_GLOBAL time_t sunset _INIT(0);
HYDROPONICS_GLOBAL Toki toki _INIT(Toki());
HYDROPONICS_GLOBAL NTPStatus ntpStatus;

HYDROPONICS_GLOBAL WiFiUDP ntpUdp;

// timer
HYDROPONICS_GLOBAL byte lastTimerMinute _INIT(0);

// Temp buffer
HYDROPONICS_GLOBAL char *obuf;
HYDROPONICS_GLOBAL uint16_t olen _INIT(0);

// General filesystem
HYDROPONICS_GLOBAL size_t fsBytesUsed _INIT(0);
HYDROPONICS_GLOBAL size_t fsBytesTotal _INIT(0);
HYDROPONICS_GLOBAL unsigned long presetsModifiedTime _INIT(0L);
HYDROPONICS_GLOBAL JsonDocument *fileDoc;
HYDROPONICS_GLOBAL bool doCloseFile _INIT(false);

HYDROPONICS_GLOBAL byte errorFlag _INIT(0);

HYDROPONICS_GLOBAL bool doSerializeConfig _INIT(false); // flag to initiate saving of config
HYDROPONICS_GLOBAL bool doReboot _INIT(false);          // flag to initiate reboot from async handlers
HYDROPONICS_GLOBAL bool doPublishMqtt _INIT(false);

// network
HYDROPONICS_GLOBAL String escapedMac;

// improv
HYDROPONICS_GLOBAL byte improvActive _INIT(0); // 0: no improv packet received, 1: improv active, 2: provisioning
HYDROPONICS_GLOBAL byte improvError _INIT(0);

// mqtt
HYDROPONICS_GLOBAL unsigned long lastMqttReconnectAttempt _INIT(0); // used for other periodic tasks too
HYDROPONICS_GLOBAL AsyncMqttClient *mqtt _INIT(NULL);
HYDROPONICS_GLOBAL bool mqttEnabled _INIT(false);
HYDROPONICS_GLOBAL char mqttStatusTopic[40] _INIT("");               // this must be global because of async handlers
HYDROPONICS_GLOBAL char mqttDeviceTopic[33] _INIT("");               // main MQTT topic (individual per device, default is wled/mac)
HYDROPONICS_GLOBAL char mqttGroupTopic[33] _INIT("hydroponics/all"); // second MQTT topic (for example to group devices)
HYDROPONICS_GLOBAL char mqttServer[33] _INIT("");                    // both domains and IPs should work (no SSL)
HYDROPONICS_GLOBAL char mqttUser[41] _INIT("");                      // optional: username for MQTT auth
HYDROPONICS_GLOBAL char mqttPass[65] _INIT("");                      // optional: password for MQTT auth
HYDROPONICS_GLOBAL char mqttClientID[41] _INIT("");                  // override the client ID
HYDROPONICS_GLOBAL uint16_t mqttPort _INIT(1883);
#define HYDROPONICS_MQTT_CONNECTED (mqtt != nullptr && mqtt->connected())


enum SensorsStatus {
  INVALID_DISTANCE = 500,
};
HYDROPONICS_GLOBAL SensorsStatus sensorsStatus;
HYDROPONICS_GLOBAL bool pumpEnabled _INIT(true);
HYDROPONICS_GLOBAL u_int minWaterLevelCm _INIT(5);           // min water level in cm
HYDROPONICS_GLOBAL u_int maxWaterLevelDifferenceCm _INIT(1); // max difference between starting to pump to current level before emergency shutdown
HYDROPONICS_GLOBAL u_int pumpLe10Interval _INIT(0);
HYDROPONICS_GLOBAL u_int pumpLe15Interval _INIT(0);
HYDROPONICS_GLOBAL u_int pumpLe20Interval _INIT(0);
HYDROPONICS_GLOBAL u_int pumpLe25Interval _INIT(0);
HYDROPONICS_GLOBAL u_int pumpGt25Interval _INIT(0);
HYDROPONICS_GLOBAL u_int pumpLe10Duration _INIT(0);
HYDROPONICS_GLOBAL u_int pumpLe15Duration _INIT(0);
HYDROPONICS_GLOBAL u_int pumpLe20Duration _INIT(0);
HYDROPONICS_GLOBAL u_int pumpLe25Duration _INIT(0);
HYDROPONICS_GLOBAL u_int pumpGt25Duration _INIT(0);

enum PumpStatus
{
  NIGHTTIME = 100,
  SCHEDULED_RUN = 200,
  SCHEDULED_STOP = 201,
  MANUAL_RUN = 202,
  UNKNOWN_TANK_VOLUME = 500,
  EMERGENCY_PUMP_STOP = 501,
  NOT_ENOUGH_WATER_STOP = 502
};

HYDROPONICS_GLOBAL PumpStatus pumpStatus;

// ui style
HYDROPONICS_GLOBAL bool showWelcomePage _INIT(false);

// User Interface CONFIG
HYDROPONICS_GLOBAL char serverDescription[33] _INIT("Hydroponics"); // Name of module - use default

// server library objects
HYDROPONICS_GLOBAL AsyncWebServer server _INIT_N(((80)));

HYDROPONICS_GLOBAL String messageHead, messageSub;
HYDROPONICS_GLOBAL byte optionType;

HYDROPONICS_GLOBAL bool bmp280Initialized;

#define HYDROPONICS_WIFI_CONFIGURED (strlen(clientSSID) >= 1 && strcmp(clientSSID, DEFAULT_CLIENT_SSID) != 0)

#define HYDROPONICS_CONNECTED (WiFi.status() == WL_CONNECTED)

#define HYDROPONICS_SET_AP_SSID()                \
  do                                             \
  {                                              \
    strcpy_P(apSSID, PSTR(HYDROPONICS_AP_SSID)); \
  } while (0)

// global ArduinoJson buffer
HYDROPONICS_GLOBAL StaticJsonDocument<JSON_BUFFER_SIZE> doc;
HYDROPONICS_GLOBAL volatile uint8_t jsonBufferLock _INIT(0);

// Track previous sensor values
HYDROPONICS_GLOBAL u_int lastDistance _INIT(__INT_MAX__);
HYDROPONICS_GLOBAL float lastVolume _INIT(__FLT_MAX__);
HYDROPONICS_GLOBAL float lastTemperature _INIT(__FLT_MAX__);
HYDROPONICS_GLOBAL float lastPressure _INIT(__FLT_MAX__);
HYDROPONICS_GLOBAL float lastWaterTemperature _INIT(__FLT_MAX__);
HYDROPONICS_GLOBAL float lastPh _INIT(__FLT_MAX__);
HYDROPONICS_GLOBAL float lastPhVoltage _INIT(0);
HYDROPONICS_GLOBAL float lastTds _INIT(__FLT_MAX__);
HYDROPONICS_GLOBAL float lastTdsVoltage _INIT(0);
HYDROPONICS_GLOBAL float lastEc _INIT(__FLT_MAX__);
HYDROPONICS_GLOBAL float lastWaterLevel _INIT(0);
HYDROPONICS_GLOBAL long lastDistanceMeasure _INIT(0);
HYDROPONICS_GLOBAL long lastTemperatureMeasure _INIT(0);
HYDROPONICS_GLOBAL long lastPhTdsMeasure _INIT(0);
HYDROPONICS_GLOBAL long lastPhTdsOnSwitch _INIT(0);

// adjustment factors for calibration
HYDROPONICS_GLOBAL float tempAdjustment _INIT(0);
HYDROPONICS_GLOBAL float waterTempAdjustment _INIT(0);
// voltages for ph calibration
HYDROPONICS_GLOBAL float phNeutralVoltage _INIT(1380.0);
HYDROPONICS_GLOBAL float phAcidVoltage _INIT(1855);

HYDROPONICS_GLOBAL float tankWidth _INIT(0);
HYDROPONICS_GLOBAL float tankLength _INIT(0);
HYDROPONICS_GLOBAL float tankHeight _INIT(0);

HYDROPONICS_GLOBAL u_int numberMeasurements _INIT(20);
HYDROPONICS_GLOBAL u_int temperatureInterval _INIT(5);
HYDROPONICS_GLOBAL u_int distanceInterval _INIT(5);
HYDROPONICS_GLOBAL u_int phTdsInterval _INIT(600);
HYDROPONICS_GLOBAL u_int phOnTime _INIT(60);
HYDROPONICS_GLOBAL u_int tdsOnTime _INIT(10);

#define DEBUGOUT Serial

#ifdef HYDROPONICS_DEBUG
#include <rom/rtc.h>
#define DEBUG_PRINT(x) DEBUGOUT.print(x)
#define DEBUG_PRINTLN(x) DEBUGOUT.println(x)
#define DEBUG_PRINTF(x...) DEBUGOUT.printf(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(x...)
#endif

#ifdef HYDROPONICS_DEBUG_FS
#define DEBUGFS_PRINT(x) DEBUGOUT.print(x)
#define DEBUGFS_PRINTLN(x) DEBUGOUT.println(x)
#define DEBUGFS_PRINTF(x...) DEBUGOUT.printf(x)
#else
#define DEBUGFS_PRINT(x)
#define DEBUGFS_PRINTLN(x)
#define DEBUGFS_PRINTF(x...)
#endif

// macro to convert F to const
#define SET_F(x) (const char *)F(x)

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

  void handleConnection();

  void initAP(bool resetAP = false);
  void initConnection();
  void onWifiConnected();
};
#endif // MAIN_H
