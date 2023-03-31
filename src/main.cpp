#define HYDROPONICS_DEFINE_GLOBAL_VARS // only in one source file, main.cpp!

#include "main.h"
#include <Arduino.h>

Hydroponics::Hydroponics()
{
  oneWire = OneWire(TEMP_PIN);
  dallasTemperature = DallasTemperature(&oneWire);
  bmp280 = Adafruit_BMP280();

  if (bmp280.begin(BMP280_ADDRESS, BMP280_CHIPID))
  {
    DEBUG_PRINTLN("BMP280 initalized");
    bmp280Initialized = true;
    bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                       Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                       Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                       Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                       Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }
  else
  {
    bmp280Initialized = false;
    DEBUG_PRINTLN("Could not initalize BMP280");
  }
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

  if (apActive)
    dnsServer.processNextRequest();

  if (doSerializeConfig)
    serializeConfig();

  if (doReboot) // if busses have to be inited & saved, wait until next iteration
    reset();

  if (doCloseFile)
  {
    closeFile();
    yield();
  }
}

void Hydroponics::setup()
{
  Serial.begin(115200);

#if defined(HYDROPONICS_DEBUG) && (defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3) || ARDUINO_USB_CDC_ON_BOOT)
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
  initApi();

  dallasTemperature.begin();
  pinMode(PH_PIN, INPUT);
  pinMode(TDS_PIN, INPUT);
  pinMode(PH_MOSFET_PIN, OUTPUT);
  pinMode(TDS_MOSFET_PIN, OUTPUT);
  pinMode(PUMP_MOSFET_PIN, OUTPUT);

  pinMode(DISTANCE_PIN_TRIGGER, OUTPUT);
  pinMode(DISTANCE_PIN_ECHO, INPUT);
}

void Hydroponics::handleSensors()
{

  timer = millis();
  if (timer - lastTemperatureMeasure >= TEMPERATURE_INTERVAL * 1000)
  {
    lastTemperatureMeasure = timer;
    float temperatureC = 0;

    for (int i = 0; i < NUMBER_MEASUREMENTS; i++)
    {
      dallasTemperature.requestTemperatures();
      temperatureC += dallasTemperature.getTempCByIndex(0);
    }

    temperatureC = roundf((temperatureC / NUMBER_MEASUREMENTS) * 10) / 10;

    if (temperatureC != lastWaterTemperature)
    {
      DEBUG_PRINTF("new water temperature %f °C\n", temperatureC);
      publishMqtt("water", String(temperatureC, 2).c_str());
      lastWaterTemperature = temperatureC;
    }

    if (bmp280Initialized)
    {

      temperatureC = roundf((bmp280.readTemperature() * 10) / 10);
      float pressure = roundf((bmp280.readPressure() * 0.1) / 10);
      if (temperatureC != lastTemperature)
      {
        DEBUG_PRINTF("new temperature %f °C\n", temperatureC);
        publishMqtt("temperature", String(temperatureC, 2).c_str());
        lastTemperature = temperatureC;
      }
      if (pressure != lastPressure)
      {
        DEBUG_PRINTF("new pressure %f Pa\n", pressure);
        publishMqtt("pressure", String(pressure, 2).c_str());
        lastPressure = pressure;
      }
    }
  }

  if (timer - lastDistanceMeasure >= DISTANCE_INTERVAL * 1000)
  {
    lastDistanceMeasure = timer;

    digitalWrite(DISTANCE_PIN_TRIGGER, LOW);
    delayMicroseconds(2);

    digitalWrite(DISTANCE_PIN_TRIGGER, HIGH);
    delayMicroseconds(10);

    digitalWrite(DISTANCE_PIN_TRIGGER, LOW);

    u_long duration = pulseIn(DISTANCE_PIN_ECHO, HIGH);
    u_int distance = (duration * .0343) / 2;

    if (distance > 0 && distance < DISTANCE_MAX && distance != lastDistance)
    {
      DEBUG_PRINTF("new distance %d cm\n", distance);
      publishMqtt("distance", String(distance).c_str());
      lastDistance = distance;
    }
  }

  if (timer - lastPhTdsMeasure >= PH_TDS_INTERVAL * 1000)
  {
    if (phMeasure)
    {
      // check, if the mosfet was not activated yet
      if (digitalRead(PH_MOSFET_PIN) == LOW)
      {
        DEBUG_PRINTLN("Activating ph mosfet");
        digitalWrite(PH_MOSFET_PIN, HIGH);
        digitalWrite(PUMP_MOSFET_PIN, HIGH);

        lastPhTdsOnSwitch = timer;
      }
      // check if the waiting period is over and we can take a measurement
      if (timer - lastPhTdsOnSwitch >= PH_ON_TIME * 1000)
      {
        DEBUG_PRINTLN("Finished waiting for ph sensor to heat up");
        lastPhTdsMeasure = timer;

        float phValue = readAverage(PH_PIN, NUMBER_MEASUREMENTS) * (float)5.0 / 4095.0;
        phValue = roundf((phValue)*10) / 10;

        if (phValue != lastPh)
        {
          DEBUG_PRINTF("new ph %f\n", phValue);
          publishMqtt("ph", String(phValue, 2).c_str());
          lastPh = phValue;
        }

        // cleanup and deactivate ph mosfet
        phMeasure = false;
        digitalWrite(PH_MOSFET_PIN, LOW);
        digitalWrite(PUMP_MOSFET_PIN, LOW);
      }
    }
    else
    {
      // check, if the mosfet was not activated yet
      if (digitalRead(TDS_MOSFET_PIN) == LOW)
      {
        DEBUG_PRINTLN("Activating tds mosfet");
        digitalWrite(TDS_MOSFET_PIN, HIGH);
        lastPhTdsOnSwitch = timer;
      }
      // check if the waiting period is over and we can take a measurement
      if (timer - lastPhTdsOnSwitch >= TDS_ON_TIME * 1000)
      {
        DEBUG_PRINTLN("Perform tds measurement");
        lastPhTdsMeasure = timer;

        float tdsRead = readAverage(TDS_PIN, NUMBER_MEASUREMENTS);

        float averageVoltage = tdsRead * (float)3.3 / 4095.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value

        float compensationCoefficient = 1.0 + 0.02 * (lastTemperature - 25.0);                                                                                                                 // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
        float compensationVolatge = averageVoltage / compensationCoefficient;                                                                                                                  // temperature compensation
        float tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; // convert voltage value to tds value

        tdsValue = roundf(tdsValue / 100) * 100;

        if (tdsValue != lastTds)
        {
          DEBUG_PRINTF("new tds %f ppm\n", tdsValue);
          publishMqtt("tds", String(tdsValue, 2).c_str());
          lastTds = tdsValue;
        }
        phMeasure = true;
        digitalWrite(TDS_MOSFET_PIN, LOW);
      }
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

  DEBUGFS_PRINTF("clientSSID: %s\n", clientSSID);
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