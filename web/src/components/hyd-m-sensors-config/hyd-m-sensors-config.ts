import { customElement, query, state } from "lit/decorators.js";
import { html, LitElement } from "lit";
import { apiFetch, apiPostJson } from "../../util";

interface SensorsConfigResponse {
  ph: SensorsConfigPhResponse;
  temperature: SensorsConfigTemperatureResponse;
  waterTemperature: SensorsConfigTemperatureResponse;
}

interface SensorsConfigPhResponse {
  neutralVoltage: number;
  acidVoltage: number;
}

interface SensorsConfigTemperatureResponse {
  adjustment: number;
}

@customElement("hyd-m-sensors-config")
export class SensorsConfig extends LitElement {
  @state()
  _sensorsConfig: SensorsConfigResponse;

  @query("#pump-config-form")
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

    // collect data in correct format
    const submitData = [...formData.entries()].reduce((store: Record<string, Record<string, number>>, word) => {
      const letter = word[0].substring(0, 4);
      const keyStore =
        store[letter] || // Does it exist in the object?
        (store[letter] = {}); // If not, create it as an empty array
      keyStore[word[0].substring(4).toLowerCase()] = parseInt(word[1].toString());

      return store;
    }, {});

    apiPostJson<unknown, SensorsConfigResponse>("/api/config/sensors.json", { submitData })
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
      <form class="bg-white shadow-md roundex px-8 pt-6 pb-8 mb-4" id="sensors-config-form" @submit="${this.handleSubmit}">
        <div><span>${this._sensorsConfig?.ph.acidVoltage}</span></div>
        <div><span>${this._sensorsConfig?.ph.neutralVoltage}</span></div>
        <div><span>${this._sensorsConfig?.temperature.adjustment}</span></div>
        <div><span>${this._sensorsConfig?.waterTemperature.adjustment}</span></div>
        <button
          class="shadow bg-purple-500 hover:bg-purple-400 focus:shadow-outline focus:outline-none text-white font-bold py-2 px-4 rounded"
          type="submit"
        >
          Save
        </button>
      </form>
    </div>`;
  }

}
