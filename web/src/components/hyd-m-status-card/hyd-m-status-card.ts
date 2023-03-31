import { customElement, state } from "lit/decorators.js";
import { html, LitElement } from "lit";
import { apiFetch } from "../../util";

interface Sensor {
  unit: string;
  value: number;
}

interface StatusResponse {
  sensors: {
    lastUpdate: number;
    distance: Sensor;
    pressure: Sensor;
    temperature: Sensor;
    waterTemperature: Sensor;
    ph: Sensor;
    tds: Sensor;
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

    return html` <div>
      <h3>Status (last update: ${this._statusResponse.sensors.lastUpdate / 1000} s)</h3>
      <ul>
        <li>${this.renderSensor("Distance", this._statusResponse.sensors.distance)}</li>
        <li>${this.renderSensor("Pressure", this._statusResponse.sensors.pressure)}</li>
        <li>${this.renderSensor("Temperature", this._statusResponse.sensors.temperature)}</li>
        <li>${this.renderSensor("Water Temperature", this._statusResponse.sensors.waterTemperature)}</li>
        <li>${this.renderSensor("PH", this._statusResponse.sensors.ph)}</li>
        <li>${this.renderSensor("TDS", this._statusResponse.sensors.tds)}</li>
      </ul>
    </div>`;
  }

  renderSensor(name: string, sensor: Sensor) {
    return html` <li>${name}: ${sensor.value} ${sensor.unit}</li>`;
  }
}
