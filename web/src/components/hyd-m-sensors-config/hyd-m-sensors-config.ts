import { customElement, query, state } from "lit/decorators.js";
import { html, LitElement } from "lit";
import { apiFetch, apiPostJson } from "../../util";

interface SensorsConfigResponse {
  ph: SensorsConfigPhResponse;
  temperature: SensorsConfigTemperatureResponse;
  waterTemperature: SensorsConfigTemperatureResponse;
  tank: SensorsConfigTankResponse;
  measurement: SensorsConfigMeasurementResponse;
}

interface SensorsConfigPhResponse {
  neutralVoltage: number;
  acidVoltage: number;
}

interface SensorsConfigTemperatureResponse {
  adjustment: number;
}

interface SensorsConfigTankResponse {
  width: number;
  height: number;
  length: number;
  minWaterLevel: number;
  maxWaterLevelDifference: number;
}

interface SensorsConfigMeasurementResponse {
  numberMeasurements: number;
  temperatureInterval: number;
  distanceInterval: number;
  phTdsInterval: number;
  phOnTime: number;
  tdsOnTime: number;
}

@customElement("hyd-m-sensors-config")
export class SensorsConfig extends LitElement {
  @state()
  _sensorsConfig: SensorsConfigResponse;

  @query("#sensors-config-form")
  configForm: HTMLFormElement;

  createRenderRoot() {
    return this; // turn off shadow dom to access external styles
  }

  connectedCallback() {
    super.connectedCallback();
    this.fetchConfig();
  }

  fetchConfig() {
    apiFetch<SensorsConfigResponse>("/api/config/sensors.json")
      .then(response => {
        this._sensorsConfig = response;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }

  handleSubmit(event: SubmitEvent) {
    event.preventDefault();

    const formData = new FormData(this.configForm);

    const submitData = {
      ph: {
        neutralVoltage: formData.get("phNeutralVoltage"),
        acidVoltage: formData.get("phAcidVoltage"),
      },
      temperature: {
        adjustment: formData.get("temperatureAdjustment"),
      },
      waterTemperature: {
        adjustment: formData.get("waterTemperatureAdjustment"),
      },
      tank: {
        height: formData.get("tankHeight"),
        width: formData.get("tankWidth"),
        length: formData.get("tankLength"),
        minWaterLevel: formData.get("minWaterLevel"),
        maxWaterLevelDifference: formData.get("maxWaterLevelDifference"),
      },
      measurement: {
        numberMeasurements: formData.get("numberMeasurements"),
        temperatureInterval: formData.get("temperatureInterval"),
        distanceInterval: formData.get("distanceInterval"),
        phTdsInterval: formData.get("phTdsInterval"),
        phOnTime: formData.get("phOnTime"),
        tdsOnTime: formData.get("tdsOnTime"),
      },
    };

    apiPostJson<unknown, SensorsConfigResponse>("/api/config/sensors.json", submitData)
      .then(response => {
        this._sensorsConfig = response;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }

  render() {
    return html` <div>
      <h2 class="mb-6 text-lg font-bold text-gray-500">Config</h2>
      <form id="sensors-config-form" @submit="${this.handleSubmit}">
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">PH calibration</legend>
          <label for="phAcidVoltage">PH Acid Voltage (4.0 pH)</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="phAcidVoltage"
            name="phAcidVoltage"
            value="${this._sensorsConfig?.ph.acidVoltage}"
          />
          <label for="phNeutralVoltage">PH Neutral Voltage (7.0 pH)</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="phNeutralVoltage"
            name="phNeutralVoltage"
            value="${this._sensorsConfig?.ph.neutralVoltage}"
          />
        </fieldset>
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Temperature Offsets</legend>
          <label for="temperatureAdjustment">Temperature Adjustment</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="temperatureAdjustment"
            name="temperatureAdjustment"
            step="0.5"
            value="${this._sensorsConfig?.temperature.adjustment}"
          />
          <label for="waterTemperatureAdjustment">Water Temperature Adjustment</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="waterTemperatureAdjustment"
            name="waterTemperatureAdjustment"
            step="0.5"
            value="${this._sensorsConfig?.waterTemperature.adjustment}"
          />
        </fieldset>
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Tank</legend>
          <label for="tankWidth">Tank Width</label>
          <input
            class="shadow appearance-none border rounded py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="tankWidth"
            name="tankWidth"
            value="${this._sensorsConfig?.tank.width}"
          />
          <label for="tankHeight">Tank Height</label>
          <input
            class="shadow appearance-none border rounded py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="tankHeight"
            name="tankHeight"
            value="${this._sensorsConfig?.tank.height}"
          />
          <label for="tankLength">Tank Length</label>
          <input
            class="shadow appearance-none border rounded py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="tankLength"
            name="tankLength"
            value="${this._sensorsConfig?.tank.length}"
          />
        </fieldset>
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Pump Tank Level Configs</legend>
          <label for="minWaterLevel">Min Water Level (cm)</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="minWaterLevel"
            name="minWaterLevel"
            min="1"
            value="${this._sensorsConfig?.tank.minWaterLevel}"
          />
          <label for="maxWaterLevelDifference">Max difference during pump cycle (cm)</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="maxWaterLevelDifference"
            name="maxWaterLevelDifference"
            min="1"
            value="${this._sensorsConfig?.tank.maxWaterLevelDifference}"
          />
        </fieldset>
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Measurements</legend>

          <label for="numberMeasurements">Number Measurements</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="numberMeasurements"
            name="numberMeasurements"
            min="1"
            value="${this._sensorsConfig?.measurement.numberMeasurements}"
          />
          <label for="temperatureInterval">Temperature Interval</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="temperatureInterval"
            name="temperatureInterval"
            min="1"
            value="${this._sensorsConfig?.measurement.temperatureInterval}"
          />
          <label for="distanceInterval">Distance interval</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="distanceInterval"
            name="distanceInterval"
            value="${this._sensorsConfig?.measurement.distanceInterval}"
          />
          <label for="phTdsInterval">PH - TDS Interval</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="phTdsInterval"
            name="phTdsInterval"
            value="${this._sensorsConfig?.measurement.phTdsInterval}"
          />
          <label for="phOnTime">PH On Time</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="phOnTime"
            name="phOnTime"
            value="${this._sensorsConfig?.measurement.phOnTime}"
          />
          <label for="tdsOnTime">TDS On Time</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="tdsOnTime"
            name="tdsOnTime"
            value="${this._sensorsConfig?.measurement.tdsOnTime}"
          />
        </fieldset>
        <button class="btn-primary" type="submit">Save</button>
      </form>
    </div>`;
  }
}
