const delay = require("mocker-api/lib/delay");

const DELAY_TIME = 500;

let status = {
  autoMode: false,
  power: 100,
  calibrated: false,
  x: 26.0,
  y: 38.0,
  rotationX: 40,
  rotationY: 50,
  width: 52.0,
  height: 77.0,
};
const proxy = {
  "PUT /api/config": (req, res) => {
    const query = req.query;

    Object.keys(query).map(k => {
      const val = query[k];
      if (val === "true") query[k] = true;
      if (val === "false") query[k] = false;
      if (!isNaN(val)) query[k] = parseInt(val);
    });

    Object.assign(status, query);
    return res.json(status);
  },
  "PUT /api/calibrate": (req, res) => {
    //status.calibrated = true;
    return res.json(status);
  },
  "DELETE /api/calibrate": (req, res) => {
    status.calibrated = false;
    return res.json(status);
  },
  "PUT /api/moveLaser": (req, res) => {
    return res.json({});
  },
  "GET /api/status.json": (req, res) => {
    return res.json({
      pump: true,
      sensors: {
        lastUpdate: 89452,
        distance: {
          unit: "cm",
          value: 7,
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
        tds: {
          unit: "ppm",
          value: 0,
        },
      },
    });
  },
  "GET /api/config.json": (req, res) => {
    return res.json({
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
  "POST /api/config.json": (req, res) => {
    return res.json(req.body);
  },
  "GET /api/wifi.json": (req, res) => {
    return res.json({
      status: "success",
      networks: [
        { ssid: "Foo", rssi: -58, bssid: "18:E8:29:9D:D4:E4", channel: 6, enc: 3 },
        { ssid: "Bar", rssi: -58, bssid: "22:E8:29:9D:D4:E4", channel: 6, enc: 3 },
      ],
    });
  },
  "POST /settings/mqtt.html": (req, res) => {
    res.redirect("/index.html");
  },
  "POST /settings/wifi.html": (req, res) => {
    res.redirect("/index.html");
  },
  "GET /settings/s.js": (req, res) => {
    const p = req.query.p;
    let response;
    if (p === "1") {
      response =
        'function GetV(){var d=document;d.Sf.CS.value="Sesame Street";d.Sf.CP.value="***************";d.Sf.I0.value=0;d.Sf.G0.value=0;d.Sf.S0.value=255;d.Sf.I1.value=0;d.Sf.G1.value=0;d.Sf.S1.value=255;d.Sf.I2.value=0;d.Sf.G2.value=0;d.Sf.S2.value=255;d.Sf.I3.value=0;d.Sf.G3.value=0;d.Sf.S3.value=0;d.Sf.CM.value="hydroponics-cc9bf0";d.Sf.AB.selectedIndex=0;d.Sf.AS.value="HYDROPONICS-AP";d.Sf.AH.checked=0;d.Sf.AP.value="*********";d.Sf.AC.value=1;d.getElementsByClassName("sip")[0].innerHTML="10.0.20.38";d.getElementsByClassName("sip")[1].innerHTML="192.168.4.1";}';
    }
    if (p === "4") {
      response =
        'function GetV(){var d=document;d.Sf.MQ.checked=1;d.Sf.MS.value="";d.Sf.MQPORT.value=1883;d.Sf.MQUSER.value="";d.Sf.MQPASS.value="";d.Sf.MQCID.value="HYDROPONICS-cc9bf0";d.Sf.MD.value="hydroponics/cc9bf0";d.Sf.MG.value="wled/all";}';
    }
    res.type("text/javascript");
    return res.send(response);
  },
};
module.exports = delay(proxy, DELAY_TIME);
