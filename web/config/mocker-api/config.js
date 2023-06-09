const delay = require("mocker-api/lib/delay");

const DELAY_TIME = 500;

const proxy = {
  "GET /api/status.json": (req, res) => {
    return res.json({
      pump: {
        status: 200,
        enabled: true,
        running: true,
        runningFor: 4000,
        lastPumpStartTankLevel: 10,
        lastPumpEndTankLevel: 8,
      },
      date: new Date().toISOString(),
      wifiStatus: "Connected",
      mqttStatus: "Disabled",
      ntp: {
        connected: true,
        lastSyncTime: 1905,
        packetSendTime: 1882,
        sunset: "2023-04-28T18:28:00Z",
        sunrise: "2023-04-28T03:49:00Z",
      },
      sensors: {
        lastUpdate: 89452,
        distance: {
          unit: "cm",
          value: 7,
        },
        volume: {
          unit: "L",
          value: 12,
        },
        waterLevel: {
          unit: "%",
          value: 66,
        },
        pressure: {
          unit: "Pa",
          value: 983,
        },
        temperature: {
          unit: "°C",
          value: 22,
        },
        waterTemperature: {
          unit: "°C",
          value: 20.5,
        },
        ph: {
          unit: "pH",
          value: 0,
        },
        phVoltage: {
          unit: "mV",
          value: 1312,
        },
        tds: {
          unit: "ppm",
          value: 250,
        },
        ec: {
          unit: "ms/cm",
          value: 500,
        },
      },
    });
  },
  "GET /api/config/sensors.json": (req, res) => {
    return res.json({
      ph: {
        neutralVoltage: 1500.0,
        acidVoltage: 2032.44,
      },
      temperature: {
        adjustment: 0.0,
      },
      waterTemperature: {
        adjustment: 0.0,
      },
      tank: {
        width: 10.0,
        height: 12.0,
        length: 14.0,
        minWaterLevel: 10,
        maxWaterLevelDifference: 5,
      },
      measurement: {
        numberMeasurements: 25,
        temperatureInterval: 5,
        distanceInterval: 5,
        phTdsInterval: 600,
        phOnTime: 60,
        tdsOnTime: 10,
      },
    });
  },
  "POST /api/config/sensors.json": (req, res) => {
    return res.json(req.body);
  },
  "GET /api/config/time.json": (req, res) => {
    return res.json({
      ntpServer: "0.pool.ntp.org",
      longitude: 12.34,
      latitude: 51.32,
    });
  },
  "POST /api/config/time.json": (req, res) => {
    return res.json(req.body);
  },
  "GET /api/config/pump.json": (req, res) => {
    return res.json({
      pumpEnabled: false,
      pumpConfig: {
        le10: {
          interval: 300,
          duration: 10,
        },
        le15: {
          interval: 200,
          duration: 15,
        },
        le20: {
          interval: 150,
          duration: 20,
        },
        le25: {
          interval: 100,
          duration: 25,
        },
        gt25: {
          interval: 50,
          duration: 30,
        },
      },
    });
  },
  "POST /api/config/pump.json": (req, res) => {
    return res.json(req.body);
  },
  "POST /api/pump.json": (req, res) => {
    return res.json({
      status: 200,
      enabled: true,
      running: true,
      runUntil: new Date(Date.now() + req.body.duration).toISOString(),
    });
  },
  "GET /api/config/wifi.json": (req, res) => {
    return res.json({
      wifi: {
        ssid: "Foobar",
        pwd: "**********",
        staticIp: [0, 0, 0, 0],
        gateway: [0, 0, 0, 0],
        subnet: [0, 0, 0, 0],
      },
      mdns: {
        address: "foo-hydroponics",
      },
      ap: {
        ssid: "hydroponics-ap",
        pwd: "**********",
        hideAp: true,
        channel: 2,
        opensOn: 2,
      },
    });
  },
  "POST /api/config/wifi.json": (req, res) => {
    return res.json(req.body);
  },
  "GET /api/config/mqtt.json": (req, res) => {
    return res.json({
      enabled: true,
      broker: "foobar.com",
      port: 1312,
      user: "foobar",
      pwd: "*******",
      clientId: "12345-hydroponics",
      deviceTopic: "foo-topic",
      groupTopic: "hydroponics/all",
    });
  },
  "POST /api/config/mqtt.json": (req, res) => {
    return res.json(req.body);
  },
  "GET /api/wifiscan.json": (req, res) => {
    if (Math.random() > 0.3) {
      return res.json({
        status: "success",
        networks: [
          { ssid: "Foo", rssi: -58, bssid: "18:E8:29:9D:D4:E4", channel: 6, enc: 3 },
          { ssid: "Bar", rssi: -58, bssid: "22:E8:29:9D:D4:E4", channel: 6, enc: 3 },
        ],
      });
    }
    return res.status(202).json({
      status: "inprogress",
    });
  },
  "POST /settings/mqtt.html": (req, res) => {
    res.redirect("/index.html");
  },
  "POST /settings/wifi.html": (req, res) => {
    res.redirect("/index.html");
  },
};
module.exports = delay(proxy, DELAY_TIME);
