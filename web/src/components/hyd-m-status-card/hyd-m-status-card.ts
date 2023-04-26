import { customElement, state } from "lit/decorators.js";
import { html, LitElement, nothing, TemplateResult } from "lit";
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

enum PumpStatus {
  UNKNOWN = 0,
  SCHEDULED_RUN = 200,
  SCHEDULED_STOP = 201,
  MANUAL_RUN = 202,
  UNKNOWN_TANK_VOLUME = 500,
  EMERGENCY_PUMP_STOP = 501,
}
interface StatusResponse {
  pump: {
    status: PumpStatus;
    enabled: boolean;
    running: boolean;
    runUntil: number;
  };
  wifiStatus: ConnectionStatus;
  mqttStatus: ConnectionStatus;
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
    const fanIcon = this._statusResponse.pump.running ? "#fan_on" : "#fan_off";
    const wifiIcon = this.pickIcon(this._statusResponse.wifiStatus);
    const mqttIcon = this.pickIcon(this._statusResponse.mqttStatus);

    let waterLevelIcon;
    if (this._statusResponse.sensors.waterLevel.value <= 20) waterLevelIcon = "#humidity_low";
    if (this._statusResponse.sensors.waterLevel.value > 20 && this._statusResponse.sensors.waterLevel.value <= 75)
      waterLevelIcon = "#humidity_mid";
    if (this._statusResponse.sensors.waterLevel.value > 75) waterLevelIcon = "#humidity_high";

    const pumpRunUntilContent =
      this._statusResponse.pump.runUntil > 0
        ? html`(until: ${new Date(this._statusResponse.pump.runUntil).toString()})`
        : nothing;

    return html` <div class="m-4 border rounded p-4">
      <span
        >Status (last update: ${this._statusResponse.sensors.lastUpdate / 1000} s)
        ${PumpStatus[this._statusResponse.pump.status]}</span
      >
      <div class="flex">
        <svg class="h-6 mr-4" viewBox="0 96 960 960">
          <use href="${fanIcon}"></use>
        </svg>
        Pump: ${this._statusResponse.pump.running} ${pumpRunUntilContent}
      </div>
      <div class="flex">
        <svg class="h-6 mr-4" viewBox="0 96 960 960">
          <use href="${wifiIcon}"></use>
        </svg>
        Wifi: ${this._statusResponse.wifiStatus}
      </div>
      <div class="flex">
        <svg class="h-6 mr-4" viewBox="0 96 960 960">
          <use href="${mqttIcon}"></use>
        </svg>
        Mqtt: ${this._statusResponse.mqttStatus}
      </div>
      <ul>
        ${this.renderListEntry(
          waterLevelIcon,
          html` Volume: ${this._statusResponse.sensors.volume.value} ${this._statusResponse.sensors.volume.unit}
          (Distance: ${this._statusResponse.sensors.distance.value} ${this._statusResponse.sensors.distance.unit})`,
        )}
        ${this.renderListEntry(
          "#thermostat",
          this.renderSensor("Temperature", this._statusResponse.sensors.temperature),
        )}
        ${this.renderListEntry(
          "#thermostat",
          this.renderSensor("Water Temperature", this._statusResponse.sensors.waterTemperature),
        )}
        ${this.renderListEntry("#thermostat", this.renderSensor("Pressure", this._statusResponse.sensors.pressure))}
        ${this.renderListEntry(
          "#water_ph",
          html`PH: ${this._statusResponse.sensors.ph.value} ${this._statusResponse.sensors.ph.unit}
          (${this._statusResponse.sensors.phVoltage.value} ${this._statusResponse.sensors.phVoltage.unit})`,
        )}
        ${this.renderListEntry("#water_tds", this.renderSensor("TDS", this._statusResponse.sensors.tds))}
      </ul>
    </div>`;
  }

  renderSensor(name: string, sensor: Sensor) {
    return html` <li>${name}: ${sensor.value} ${sensor.unit}</li>`;
  }

  private renderListEntry(icon: string, content: TemplateResult) {
    return html`<li class="flex">
      <svg class="h-6 mr-4" viewBox="0 96 960 960">
        <use href="${icon}"></use>
      </svg>
      ${content}
    </li>`;
  }

  private pickIcon(status: ConnectionStatus): string {
    switch (status) {
      case ConnectionStatus.CONNECTED:
        return "#wifi_on";
      case ConnectionStatus.DISABLED:
        return "#wifi_off";
      case ConnectionStatus.DISCONNECTED:
      default:
        return "#wifi_bad";
    }
  }
}
