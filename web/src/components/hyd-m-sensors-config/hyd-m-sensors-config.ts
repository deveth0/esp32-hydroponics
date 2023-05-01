import { customElement, query, state } from "lit/decorators.js";
import { html, LitElement } from "lit";
import { apiFetch, apiPostJson } from "../../util";
import { renderFormInputNumber } from "../formFields";

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
          ${renderFormInputNumber(
            "PH Acid Voltage (4.0 pH)",
            "phAcidVoltage",
            this._sensorsConfig?.ph.acidVoltage,
            0.01,
            0,
          )}
          ${renderFormInputNumber(
            "PH Neutral Voltage (7.0 pH)",
            "phNeutralVoltage",
            this._sensorsConfig?.ph.neutralVoltage,
            0.01,
            0,
          )}
        </fieldset>
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Temperature Offsets</legend>
          <label for="temperatureAdjustment">Temperature Adjustment</label>
          ${renderFormInputNumber(
            "Temperature Adjustment",
            "temperatureAdjustment",
            this._sensorsConfig?.temperature.adjustment,
            0.5,
            0,
          )}
          ${renderFormInputNumber(
            "Water Temperature Adjustment",
            "waterTemperatureAdjustment",
            this._sensorsConfig?.waterTemperature.adjustment,
            0.5,
            0,
          )}
        </fieldset>
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Tank</legend>
          ${renderFormInputNumber("Tank Width", "tankWidth", this._sensorsConfig?.tank.width, 1, 1)}
          ${renderFormInputNumber("Tank Height", "tankHeight", this._sensorsConfig?.tank.height, 1, 1)}
          ${renderFormInputNumber("Tank Length", "tankLength", this._sensorsConfig?.tank.length, 1, 1)}
        </fieldset>
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Pump Tank Level Configs</legend>
          ${renderFormInputNumber(
            "Min Water Level (cm)",
            "minWaterLevel",
            this._sensorsConfig?.tank.minWaterLevel,
            1,
            1,
          )}
          ${renderFormInputNumber(
            "Max difference during pump cycle (cm)",
            "maxWaterLevelDifference",
            this._sensorsConfig?.tank.maxWaterLevelDifference,
            1,
            1,
          )}
        </fieldset>
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Measurements</legend>
          ${renderFormInputNumber(
            "Number of measurements",
            "numberMeasurements",
            this._sensorsConfig?.measurement.numberMeasurements,
            1,
            1,
          )}
          ${renderFormInputNumber(
            "Temperature Interval",
            "temperatureInterval",
            this._sensorsConfig?.measurement.temperatureInterval,
            1,
            1,
          )}
          ${renderFormInputNumber(
            "Distance Interval",
            "distanceInterval",
            this._sensorsConfig?.measurement.distanceInterval,
            1,
            1,
          )}
          ${renderFormInputNumber(
            "PH - TDS Interval",
            "phTdsInterval",
            this._sensorsConfig?.measurement.phTdsInterval,
            1,
            1,
          )}
          ${renderFormInputNumber("PH On Time", "phOnTime", this._sensorsConfig?.measurement.phOnTime, 1, 1)}
          ${renderFormInputNumber("TDS On Time", "tdsOnTime", this._sensorsConfig?.measurement.tdsOnTime, 1, 1)}
        </fieldset>
        <button class="btn-primary" type="submit">Save</button>
      </form>
    </div>`;
  }
}
