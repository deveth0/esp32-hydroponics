#define HYDROPONICS_DEFINE_GLOBAL_VARS // only in one source file, main.cpp!

#include "main.h"
#include <Arduino.h>

Hydroponics::Hydroponics()
{
  oneWire = OneWire(tempPin);
  dallasTemperature = DallasTemperature(&oneWire);
}

void Hydroponics::reset()
{
  ESP.restart();
}

void Hydroponics::loop()
{
  handleConnection();
  yield();
  handleSensors();

  if (apActive) dnsServer.processNextRequest();
}

void Hydroponics::setup()
{
  Serial.begin(115200);

#if defined(WLED_DEBUG) && (defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3) || ARDUINO_USB_CDC_ON_BOOT)
  delay(2500); // allow CDC USB serial to initialise
#endif

  DEBUG_PRINTLN();
  DEBUG_PRINT(F("---HYDROPONICS "));
  DEBUG_PRINT(versionString);
  DEBUG_PRINT(" ");
  DEBUG_PRINT(VERSION);
  DEBUG_PRINTLN(F(" INIT---"));

  bool fsinit = false;
  DEBUGFS_PRINTLN(F("Mount FS"));
  fsinit = HYDROPONICS_FS.begin(true);
  if (!fsinit)
  {
    DEBUGFS_PRINTLN(F("FS failed!"));
    errorFlag = ERR_FS_BEGIN;
  }

  updateFSInfo();

  // generate module IDs must be done before AP setup
  escapedMac = WiFi.macAddress();
  escapedMac.replace(":", "");
  escapedMac.toLowerCase();

  HYDROPONICS_SET_AP_SSID(); // otherwise it is empty on first boot until config is saved

  DEBUG_PRINTLN(F("Reading config"));
  deserializeConfigFromFS();

  if (strcmp(clientSSID, DEFAULT_CLIENT_SSID) == 0)
    showWelcomePage = true;
  WiFi.persistent(false);

  // fill in unique mdns default
  if (strcmp(cmDNS, "x") == 0)
    sprintf_P(cmDNS, PSTR("hydroponics-%*s"), 6, escapedMac.c_str() + 6);
  if (mqttDeviceTopic[0] == 0)
    sprintf_P(mqttDeviceTopic, PSTR("hydroponics/%*s"), 6, escapedMac.c_str() + 6);
  if (mqttClientID[0] == 0)
    sprintf_P(mqttClientID, PSTR("HYDROPONICS-%*s"), 6, escapedMac.c_str() + 6);

  // HTTP server page init
  initServer();

  dallasTemperature.begin();
  pinMode(phPin, INPUT);
  pinMode(tdsPin, INPUT);
}

void Hydroponics::handleSensors()
{

  timer = millis();
  if (timer - lastTemperatureMeasure >= TemperatureInterval * 1000)
  {
    lastTemperatureMeasure = timer;

    dallasTemperature.requestTemperatures();

    float temperatureC = dallasTemperature.getTempCByIndex(0);
    temperatureC = roundf(temperatureC * 10) / 10;

    if (temperatureC != lastTemperature)
    {
      DEBUGFS_PRINTF("new temperature %f °C\n", temperatureC);
      publishMqtt("temperature", String(temperatureC, 2).c_str());
      lastTemperature = temperatureC;
    }

    float tdsRead = analogRead(tdsPin);
    float averageVoltage = tdsRead * (float)3.3 / 4095.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value

    float compensationCoefficient = 1.0 + 0.02 * (temperatureC - 25.0);                                                                                                                    // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge = averageVoltage / compensationCoefficient;                                                                                                                  // temperature compensation
    float tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; // convert voltage value to tds value

    tdsValue = roundf(tdsValue / 100) * 100;

    if (tdsValue != lastTds)
    {
      DEBUGFS_PRINTF("new tds %f ppm\n", tdsValue);
      publishMqtt("tds", String(tdsValue, 2).c_str());
      lastTds = tdsValue;
    }

    float phVoltage = analogRead(tdsPin) * (5.0 / 4095.0);
    float phValue = mapfloat(phVoltage, 0.0, 5.0, 14, 0);
    phValue = roundf(phValue * 10) / 10;

    if (phValue != lastPh)
    {
      DEBUGFS_PRINTF("new ph %f\n", phValue);
      publishMqtt("ph", String(phValue, 2).c_str());
      lastPh = phValue;
    }
  }
}

void Hydroponics::handleConnection()
{
  static byte stacO = 0;
  static uint32_t lastHeap = UINT32_MAX;
  static unsigned long heapTime = 0;
  unsigned long now = millis();

  if (now < 2000 && (!HYDROPONICS_WIFI_CONFIGURED || apBehavior == AP_BEHAVIOR_ALWAYS))
    return;

  if (lastReconnectAttempt == 0)
  {
    DEBUG_PRINTLN(F("lastReconnectAttempt == 0"));
    initConnection();
    return;
  }

  // reconnect WiFi to clear stale allocations if heap gets too low
  if (now - heapTime > 5000)
  {
    uint32_t heap = ESP.getFreeHeap();
    if (heap < MIN_HEAP_SIZE && lastHeap < MIN_HEAP_SIZE)
    {
      DEBUG_PRINT(F("Heap too low! "));
      DEBUG_PRINTLN(heap);
      forceReconnect = true;
    }
    lastHeap = heap;
    heapTime = now;
  }

  byte stac = 0;
  if (apActive)
  {
    wifi_sta_list_t stationList;
    esp_wifi_ap_get_sta_list(&stationList);
    stac = stationList.num;

    if (stac != stacO)
    {
      stacO = stac;
      DEBUG_PRINT(F("Connected AP clients: "));
      DEBUG_PRINTLN(stac);
      if (!HYDROPONICS_CONNECTED && HYDROPONICS_WIFI_CONFIGURED)
      { // trying to connect, but not connected
        if (stac)
          WiFi.disconnect(); // disable search so that AP can work
        else
          initConnection(); // restart search
      }
    }
  }
  if (forceReconnect)
  {
    DEBUG_PRINTLN(F("Forcing reconnect."));
    initConnection();
    interfacesInited = false;
    forceReconnect = false;
    wasConnected = false;
    return;
  }
  if (!Network.isConnected())
  {
    if (interfacesInited)
    {
      DEBUG_PRINTLN(F("Disconnected!"));
      interfacesInited = false;
      initConnection();
    }
    // send improv failed 6 seconds after second init attempt (24 sec. after provisioning)
    if (improvActive > 2 && now - lastReconnectAttempt > 6000)
    {
      sendImprovStateResponse(0x03, true);
      improvActive = 2;
    }
    if (now - lastReconnectAttempt > ((stac) ? 300000 : 18000) && HYDROPONICS_WIFI_CONFIGURED)
    {
      if (improvActive == 2)
        improvActive = 3;
      DEBUG_PRINTLN(F("Last reconnect too old."));
      initConnection();
    }
    if (!apActive && now - lastReconnectAttempt > 12000 && (!wasConnected || apBehavior == AP_BEHAVIOR_NO_CONN))
    {
      DEBUG_PRINTLN(F("Not connected AP."));
      initAP();
    }
  }
  else if (!interfacesInited)
  { // newly connected
    DEBUG_PRINTLN("");
    DEBUG_PRINT(F("Connected! IP address: "));
    DEBUG_PRINTLN(Network.localIP());
    if (improvActive)
    {
      if (improvError == 3)
        sendImprovStateResponse(0x00, true);
      sendImprovStateResponse(0x04);
      if (improvActive > 1)
        sendImprovRPCResponse(0x01);
    }
    initInterfaces();

    lastMqttReconnectAttempt = 0; // force immediate update

    // shut down AP
    if (apBehavior != AP_BEHAVIOR_ALWAYS && apActive)
    {
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      apActive = false;
      DEBUG_PRINTLN(F("Access point disabled (handle)."));
    }
  }
}

void Hydroponics::initInterfaces()
{

  // Set up mDNS responder:
  if (strlen(cmDNS) > 0)
  {
    // "end" must be called before "begin" is called a 2nd time
    // see https://github.com/esp8266/Arduino/issues/7213
    MDNS.end();
    MDNS.begin(cmDNS);

    DEBUG_PRINTLN(F("mDNS started"));
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("wled", "tcp", 80);
    MDNS.addServiceTxt("wled", "tcp", "mac", escapedMac.c_str());
  }
  server.begin();

  if (udpPort > 0 && udpPort != ntpLocalPort)
  {
    udpConnected = notifierUdp.begin(udpPort);
  }

  if (ntpEnabled)
    ntpConnected = ntpUdp.begin(ntpLocalPort);

  initMqtt();

  interfacesInited = true;
  wasConnected = true;
}

void Hydroponics::initAP(bool resetAP)
{
  if (apBehavior == AP_BEHAVIOR_BUTTON_ONLY && !resetAP)
    return;

  if (resetAP)
  {
    HYDROPONICS_SET_AP_SSID();
    strcpy_P(apPass, PSTR(HYDROPONICS_AP_PASS));
  }
  DEBUG_PRINT(F("Opening access point "));
  DEBUG_PRINTLN(apSSID);
  WiFi.softAPConfig(IPAddress(4, 3, 2, 1), IPAddress(4, 3, 2, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID, apPass, apChannel, apHide);

  if (!apActive) // start captive portal if AP active
  {
    DEBUG_PRINTLN(F("Init AP interfaces"));
    server.begin();
    if (udpPort > 0 && udpPort != ntpLocalPort)
    {
      udpConnected = notifierUdp.begin(udpPort);
    }

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
  }
  apActive = true;
}

void Hydroponics::initConnection()
{

  WiFi.disconnect(true); // close old connections

  if (staticIP[0] != 0 && staticGateway[0] != 0)
  {
    WiFi.config(staticIP, staticGateway, staticSubnet, IPAddress(1, 1, 1, 1));
  }
  else
  {
    WiFi.config(IPAddress((uint32_t)0), IPAddress((uint32_t)0), IPAddress((uint32_t)0));
  }

  lastReconnectAttempt = millis();

  if (!HYDROPONICS_WIFI_CONFIGURED)
  {
    DEBUG_PRINTLN(F("No connection configured."));
    if (!apActive)
      initAP(); // instantly go to ap mode
    return;
  }
  else if (!apActive)
  {
    if (apBehavior == AP_BEHAVIOR_ALWAYS)
    {
      DEBUG_PRINTLN(F("Access point ALWAYS enabled."));
      initAP();
    }
    else
    {
      DEBUG_PRINTLN(F("Access point disabled (init)."));
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_STA);
    }
  }
  showWelcomePage = false;

  DEBUG_PRINT(F("Connecting to "));
  DEBUG_PRINT(clientSSID);
  DEBUG_PRINTLN("...");

  // convert the "serverDescription" into a valid DNS hostname (alphanumeric)
  char hostname[25];
  prepareHostname(hostname);

  WiFi.begin(clientSSID, clientPass);

  WiFi.setSleep(false);
  WiFi.setHostname(hostname);
}