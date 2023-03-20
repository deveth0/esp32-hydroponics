#ifndef HYDROPONICS_H
#define HYDROPONICS_H

#define VERSION 2303200

#define HYDROPONICS__DEBUG

#include <Arduino.h>
#include <HardwareSerial.h> // ensure we have the correct "Serial" on new MCUs (depends on ARDUINO_USB_MODE and ARDUINO_USB_CDC_ON_BOOT)
#include <WiFi.h>
#include "esp_wifi.h"
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <Wire.h>
#include <SPI.h>

#include <ArduinoJson.h>
#include <AsyncMqttClient.h>

#include "dependencies/network/Network.h"

#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>

#include "const.h"
#include "util.h"
#include "improv.h"
#include "cfg.h"

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

// GLOBAL VARIABLES
// both declared and defined in header (solution from http://www.keil.com/support/docs/1868.htm)
//
//e.g. byte test = 2 becomes HYDROPONICS_GLOBAL byte test _INIT(2);
//     int arr[]{0,1,2} becomes HYDROPONICS_GLOBAL int arr[] _INIT_N(({0,1,2}));

#ifndef HYDROPONICS_DEFINE_GLOBAL_VARS
# define HYDROPONICS_GLOBAL extern
# define _INIT(x)
# define _INIT_N(x)
#else
# define HYDROPONICS_GLOBAL
# define _INIT(x) = x
//needed to ignore commas in array definitions
#define UNPACK( ... ) __VA_ARGS__
#define _INIT_N(x) UNPACK x
#endif


// AP and OTA default passwords (for maximum security change them!)
HYDROPONICS_GLOBAL char apPass[65]  _INIT(HYDROPONICS_AP_PASS);
HYDROPONICS_GLOBAL char otaPass[33] _INIT(DEFAULT_OTA_PASS);

// internal global variable declarations
// wifi
HYDROPONICS_GLOBAL bool apActive _INIT(false);
HYDROPONICS_GLOBAL bool forceReconnect _INIT(false);
HYDROPONICS_GLOBAL uint32_t lastReconnectAttempt _INIT(0);
HYDROPONICS_GLOBAL bool interfacesInited _INIT(false);
HYDROPONICS_GLOBAL bool wasConnected _INIT(false);

// WiFi CONFIG (all these can be changed via web UI, no need to set them here)
HYDROPONICS_GLOBAL char clientSSID[33] _INIT(CLIENT_SSID);
HYDROPONICS_GLOBAL char clientPass[65] _INIT(CLIENT_PASS);
HYDROPONICS_GLOBAL char cmDNS[33] _INIT("x");                             // mDNS address (placeholder, is replaced by wledXXXXXX.local)
HYDROPONICS_GLOBAL char apSSID[33] _INIT("");                             // AP off by default (unless setup)
HYDROPONICS_GLOBAL byte apChannel _INIT(1);                               // 2.4GHz WiFi AP channel (1-13)
HYDROPONICS_GLOBAL byte apHide    _INIT(0);                               // hidden AP SSID
HYDROPONICS_GLOBAL byte apBehavior _INIT(AP_BEHAVIOR_BOOT_NO_CONN);       // access point opens when no connection after boot by default
HYDROPONICS_GLOBAL IPAddress staticIP      _INIT_N(((  0,   0,  0,  0))); // static IP of ESP
HYDROPONICS_GLOBAL IPAddress staticGateway _INIT_N(((  0,   0,  0,  0))); // gateway (router) IP
HYDROPONICS_GLOBAL IPAddress staticSubnet  _INIT_N(((255, 255, 255, 0))); // most common subnet in home networks


// dns server
HYDROPONICS_GLOBAL DNSServer dnsServer;

// network time
HYDROPONICS_GLOBAL bool ntpConnected _INIT(false);
HYDROPONICS_GLOBAL time_t localTime _INIT(0);
HYDROPONICS_GLOBAL unsigned long ntpLastSyncTime _INIT(999000000L);
HYDROPONICS_GLOBAL unsigned long ntpPacketSentTime _INIT(999000000L);
HYDROPONICS_GLOBAL IPAddress ntpServerIP;
HYDROPONICS_GLOBAL uint16_t ntpLocalPort _INIT(2390);
HYDROPONICS_GLOBAL uint16_t rolloverMillis _INIT(0);

// network
HYDROPONICS_GLOBAL bool udpConnected _INIT(false);
HYDROPONICS_GLOBAL String escapedMac;

//improv
HYDROPONICS_GLOBAL byte improvActive _INIT(0); //0: no improv packet received, 1: improv active, 2: provisioning
HYDROPONICS_GLOBAL byte improvError _INIT(0);

// mqtt
HYDROPONICS_GLOBAL unsigned long lastMqttReconnectAttempt _INIT(0);  // used for other periodic tasks too
HYDROPONICS_GLOBAL AsyncMqttClient *mqtt _INIT(NULL);
HYDROPONICS_GLOBAL bool mqttEnabled _INIT(false);
HYDROPONICS_GLOBAL char mqttStatusTopic[40] _INIT("");            // this must be global because of async handlers
HYDROPONICS_GLOBAL char mqttDeviceTopic[33] _INIT("");            // main MQTT topic (individual per device, default is wled/mac)
HYDROPONICS_GLOBAL char mqttGroupTopic[33] _INIT("wled/all");     // second MQTT topic (for example to group devices)
HYDROPONICS_GLOBAL char mqttServer[33] _INIT("");                 // both domains and IPs should work (no SSL)
HYDROPONICS_GLOBAL char mqttUser[41] _INIT("");                   // optional: username for MQTT auth
HYDROPONICS_GLOBAL char mqttPass[65] _INIT("");                   // optional: password for MQTT auth
HYDROPONICS_GLOBAL char mqttClientID[41] _INIT("");               // override the client ID
HYDROPONICS_GLOBAL uint16_t mqttPort _INIT(1883);
#define HYDROPONICS_MQTT_CONNECTED (mqtt != nullptr && mqtt->connected())

// ui style
HYDROPONICS_GLOBAL bool showWelcomePage _INIT(false);

// User Interface CONFIG
#ifndef SERVERNAME
HYDROPONICS_GLOBAL char serverDescription[33] _INIT("Hydroponics");  // Name of module - use default
#else
HYDROPONICS_GLOBAL char serverDescription[33] _INIT(SERVERNAME);  // use predefined name
#endif

// server library objects
HYDROPONICS_GLOBAL AsyncWebServer server _INIT_N(((80)));

// udp interface objects
HYDROPONICS_GLOBAL WiFiUDP notifierUdp;
HYDROPONICS_GLOBAL WiFiUDP ntpUdp;

HYDROPONICS_GLOBAL uint16_t udpPort    _INIT(21324); // WLED notifier default port

#define HYDROPONICS_WIFI_CONFIGURED (strlen(clientSSID) >= 1 && strcmp(clientSSID, DEFAULT_CLIENT_SSID) != 0)

#define HYDROPONICS_CONNECTED (WiFi.status() == WL_CONNECTED)


#define HYDROPONICS_SET_AP_SSID() do { \
    strcpy_P(apSSID, PSTR(HYDROPONICS_AP_SSID)); \
  } while(0)


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

};
#endif // MAIN_H
