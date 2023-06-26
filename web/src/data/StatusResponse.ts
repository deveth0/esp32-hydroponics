export interface Sensor {
  unit: string;
  value: number;
}

export enum ConnectionStatus {
  CONNECTED = "Connected",
  DISCONNECTED = "Disconnected",
  DISABLED = "Disabled",
}

export enum PumpStatus {
  UNKNOWN = 0,
  NIGHTTIME = 100,
  SCHEDULED_RUN = 200,
  SCHEDULED_STOP = 201,
  MANUAL_RUN = 202,
  UNKNOWN_TANK_VOLUME = 500,
  EMERGENCY_PUMP_STOP = 501,
  NOT_ENOUGH_WATER_STOP = 502,
}

export interface PumpStatusResponse {
  status: PumpStatus;
  enabled: boolean;
  running: boolean;
  runningFor: number;
  lastPumpStartTankLevel: number;
  lastPumpEndTankLevel: number;
}

export interface StatusResponse {
  date: number;
  pump: PumpStatusResponse;
  wifiStatus: ConnectionStatus;
  mqttStatus: ConnectionStatus;
  ntp: {
    connected: boolean;
    lastSyncTime: number;
    packetSendTime: number;
    sunset?: string;
    sunrise?: string;
  };
  sensors: {
    lastUpdate: number;
    distance: Sensor;
    waterLevel: Sensor;
    pressure: Sensor;
    temperature: Sensor;
    waterTemperature: Sensor;
    ph: Sensor;
    phVoltage: Sensor;
    tds: Sensor;
    ec: Sensor;
    volume: Sensor;
  };
}
