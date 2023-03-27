const delay = require('mocker-api/lib/delay');

const DELAY_TIME = 500;

let status = {
  autoMode: false,
  power: 100,
  calibrated: false,
  x: 26.00,
  y: 38.00,
  rotationX: 40,
  rotationY: 50,
  width: 52.00,
  height: 77.00
}
const proxy = {
  'PUT /api/config': (req, res) => {
    const query = req.query;

    Object.keys(query).map(k => {
      const val = query[k];
      if (val === 'true') query[k] = true;
      if (val === 'false') query[k] = false;
      if (!isNaN(val)) query[k] = parseInt(val);
    });

    Object.assign(status, query);
    return res.json(status);
  },
  'PUT /api/calibrate': (req, res) => {
    //status.calibrated = true;
    return res.json(status);
  },
  'DELETE /api/calibrate': (req, res) => {
    status.calibrated = false;
    return res.json(status);
  },
  'PUT /api/moveLaser': (req, res) => {
    return res.json({});
  },
  'GET /api/status': (req, res) => {
    return res.json({
      autoMode: false,
      power: 100,
      calibrated: Math.random() < 0.5,
      x: 26.00,
      y: 38.00,
      rotationX: 40,
      rotationY: 50,
      width: 52.00,
      height: 77.00
    });
  },
  'GET /settings/s.js': (req, res) => {
    const p = req.query.p;
    let response;
    if(p === "1"){
     response = 'function GetV(){var d=document;d.Sf.CS.value="Sesame Street";d.Sf.CP.value="***************";d.Sf.I0.value=0;d.Sf.G0.value=0;d.Sf.S0.value=255;d.Sf.I1.value=0;d.Sf.G1.value=0;d.Sf.S1.value=255;d.Sf.I2.value=0;d.Sf.G2.value=0;d.Sf.S2.value=255;d.Sf.I3.value=0;d.Sf.G3.value=0;d.Sf.S3.value=0;d.Sf.CM.value="hydroponics-cc9bf0";d.Sf.AB.selectedIndex=0;d.Sf.AS.value="HYDROPONICS-AP";d.Sf.AH.checked=0;d.Sf.AP.value="*********";d.Sf.AC.value=1;d.getElementsByClassName("sip")[0].innerHTML="10.0.20.38";d.getElementsByClassName("sip")[1].innerHTML="192.168.4.1";}';
    }
    if(p === "4"){
      response = 'function GetV(){var d=document;d.Sf.MQ.checked=1;d.Sf.MS.value="";d.Sf.MQPORT.value=1883;d.Sf.MQUSER.value="";d.Sf.MQPASS.value="";d.Sf.MQCID.value="HYDROPONICS-cc9bf0";d.Sf.MD.value="hydroponics/cc9bf0";d.Sf.MG.value="wled/all";}';
    }
    res.type("text/javascript")
    return res.send(response);
  }
};
module.exports = delay(proxy, DELAY_TIME);