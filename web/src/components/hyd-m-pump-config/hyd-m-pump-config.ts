import { customElement, query, state } from "lit/decorators.js";
import { html, LitElement } from "lit";
import { apiFetch, apiPostJson } from "../../util";

interface ConfigResponse {
  pumpEnabled: boolean;
  pumpConfig: Record<string, PumpConfigEntry>;
}

interface PumpConfigEntry {
  interval: number;
  duration: number;
}

@customElement("hyd-m-pump-config")
export class PumpConfig extends LitElement {
  @state()
  _pumpEnabled: boolean;

  @state()
  _pumpConfig: Record<string, PumpConfigEntry> = {};

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
    apiFetch<ConfigResponse>("/api/config/pump.json")
      .then(response => {
        this._pumpEnabled = response.pumpEnabled;
        this._pumpConfig = response.pumpConfig;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }

  handleSubmit(event: SubmitEvent) {
    event.preventDefault();

    const formData = new FormData(this.configForm);

    // collect data in correct format
    const newPumpConfig = [...formData.entries()].reduce((store: Record<string, Record<string, number>>, word) => {
      const letter = word[0].substring(0, 4);
      const keyStore =
        store[letter] || // Does it exist in the object?
        (store[letter] = {}); // If not, create it as an empty array
      keyStore[word[0].substring(4).toLowerCase()] = parseInt(word[1].toString());

      return store;
    }, {});

    apiPostJson<unknown, ConfigResponse>("/api/config/pump.json", {
      pumpConfig: newPumpConfig,
      pumpEnabled: this._pumpEnabled,
    })
      .then(response => {
        this._pumpConfig = response.pumpConfig;
        this._pumpEnabled = response.pumpEnabled;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }

  render() {
    return html` <div>
      <h2 class="mb-6 text-lg font-bold text-gray-500">Config</h2>
      <form id="pump-config-form" @submit="${this.handleSubmit}">
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Pump</legend>
          <div class="mb-4 flex">
            <label class="block text-grey-700 text-sm font-bold mb-2 mr-10" for="pumpEnabled">Enable Pump</label>
            <input type="checkbox" checked="${this._pumpEnabled}" id="pumpEnabled" name="pumpEnabled" />
          </div>
        </fieldset>
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Temperature based cycles</legend>
          ${this.renderFormInput("< 10 °C", "le10")} ${this.renderFormInput("10 °C - 15 °C", "le15")}
          ${this.renderFormInput("15 °C - 20 °C", "le20")} ${this.renderFormInput("20 °C - 25 °C", "le25")}
          ${this.renderFormInput("> 25 °C", "gt25")}
        </fieldset>
        <button class="btn-primary" type="submit">Save</button>
      </form>
    </div>`;
  }

  renderFormInput(label: string, name: string) {
    return html` <div class="mb-4 flex">
      <label class="block text-gray-700 text-sm font-bold mb-2 mr-10" for=${name + "Interval"}>${label}</label>
      <input
        class="shadow border rounded py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
        id=${name + "Interval"}
        name=${name + "Interval"}
        required
        min="0"
        max="300"
        value="${this._pumpConfig[name]?.interval}"
        type="number"
        placeholder="Minutes"
      />
      <input
        class="shadow border rounded py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
        id=${name + "Duration"}
        name=${name + "Duration"}
        required
        min="0"
        max="60"
        value="${this._pumpConfig[name]?.duration}"
        type="number"
        placeholder="Minutes"
      />
    </div>`;
  }
}
