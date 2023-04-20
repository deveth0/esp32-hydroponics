import { customElement, state } from "lit/decorators.js";
import { html, LitElement } from "lit";
import { apiFetch } from "../../util";

interface Sensor {
  unit: string;
  value: number;
}

enum ConnectionStatus {
  CONNECTED = "Connected",
  DISCONNECTED = "Disconnected",
  DISABLED = "Disabled",
}

interface StatusResponse {
  pump: boolean;
  wifiStatus: ConnectionStatus;
  mqttStatus: ConnectionStatus;
  sensors: {
    lastUpdate: number;
    distance: Sensor;
    pressure: Sensor;
    temperature: Sensor;
    waterTemperature: Sensor;
    ph: Sensor;
    phVoltage: Sensor;
    tds: Sensor;
    volume: Sensor;
  };
}

@customElement("hyd-m-status-card")
export class StatusCard extends LitElement {
  @state()
  private _statusResponse: StatusResponse;

  createRenderRoot() {
    return this; // turn off shadow dom to access external styles
  }

  connectedCallback() {
    super.connectedCallback();
    this.fetchData();
    setInterval(this.fetchData.bind(this), 10000);
  }

  fetchData() {
    apiFetch<StatusResponse>("/api/status.json")
      .then(response => {
        this._statusResponse = response;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }

  render() {
    if (this._statusResponse === undefined) {
      return;
    }
    const fanIcon = this._statusResponse.pump ? "#fan_on" : "#fan_off";
    let wifiIcon;
    switch (this._statusResponse.wifiStatus) {
      case ConnectionStatus.CONNECTED:
        wifiIcon = "#wifi_on";
        break;
      case ConnectionStatus.DISABLED:
        wifiIcon = "#wifi_off";
        break;
      case ConnectionStatus.DISCONNECTED:
        wifiIcon = "#wifi_bad";
        break;
    }
    let mqttIcon;
    switch (this._statusResponse.mqttStatus) {
      case ConnectionStatus.CONNECTED:
        mqttIcon = "#wifi_on";
        break;
      case ConnectionStatus.DISABLED:
        mqttIcon = "#wifi_off";
        break;
      case ConnectionStatus.DISCONNECTED:
        mqttIcon = "#wifi_bad";
        break;
    }
    return html` <div class="m-4 border rounded p-4">
      <span>Status (last update: ${this._statusResponse.sensors.lastUpdate / 1000} s)</span>
      <div class="flex">
        <svg class="h-6" viewBox="0 96 960 960">
          <use href="${fanIcon}"></use>
        </svg>
        Pump: ${this._statusResponse.pump}
      </div>
      <div class="flex">
        <svg class="h-6" viewBox="0 96 960 960">
          <use href="${wifiIcon}"></use>
        </svg>
        Wifi: ${this._statusResponse.wifiStatus}
      </div>
      <div class="flex">
        <svg class="h-6" viewBox="0 96 960 960">
          <use href="${mqttIcon}"></use>
        </svg>
        Mqtt: ${this._statusResponse.mqttStatus}
      </div>
      <ul>
        <li class="flex">
          Volume: ${this._statusResponse.sensors.volume.value} ${this._statusResponse.sensors.volume.unit} (Distance:
          ${this._statusResponse.sensors.distance.value} ${this._statusResponse.sensors.distance.unit})
        </li>
        <li class="flex">
          <svg class="h-6" viewBox="0 96 960 960">
            <use href="#thermostat"></use>
          </svg>
          ${this.renderSensor("Temperature", this._statusResponse.sensors.temperature)}
        </li>
        <li class="flex">
          <svg class="h-6" viewBox="0 96 960 960">
            <use href="#thermostat"></use>
          </svg>
          ${this.renderSensor("Water Temperature", this._statusResponse.sensors.waterTemperature)}
        </li>
        <li class="flex">${this.renderSensor("Pressure", this._statusResponse.sensors.pressure)}</li>
        <li class="flex">
          <svg class="h-6" viewBox="0 96 960 960">
            <use href="#water_ph"></use>
          </svg>
          PH: ${this._statusResponse.sensors.ph.value} ${this._statusResponse.sensors.ph.unit}
          (${this._statusResponse.sensors.phVoltage.value} ${this._statusResponse.sensors.phVoltage.unit})
        </li>
        <li class="flex">
          <svg class="h-6" viewBox="0 96 960 960">
            <use href="#water_tds"></use>
          </svg>
          ${this.renderSensor("TDS", this._statusResponse.sensors.tds)}
        </li>
      </ul>
    </div>`;
  }

  renderSensor(name: string, sensor: Sensor) {
    return html` <li>${name}: ${sensor.value} ${sensor.unit}</li>`;
  }
}
