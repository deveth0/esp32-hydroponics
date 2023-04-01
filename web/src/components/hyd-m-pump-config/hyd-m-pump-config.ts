import { customElement, query, state } from "lit/decorators.js";
import { html, LitElement } from "lit";
import { apiFetch, apiPostJson } from "../../util";

interface ConfigResponse {
  pumpConfig: Record<string, number>;
}

@customElement("hyd-m-pump-config")
export class PumpConfig extends LitElement {
  @state()
  _pumpConfig: Record<string, number> = {};

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
    apiFetch<ConfigResponse>("/api/config.json")
      .then(response => {
        this._pumpConfig = response.pumpConfig;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }

  handleSubmit(event: SubmitEvent) {
    event.preventDefault();

    const formData = new FormData(this.configForm);

    apiPostJson<unknown, ConfigResponse>("/api/config.json", { pumpConfig: Object.fromEntries(formData) })
      .then(response => {
        this._pumpConfig = response.pumpConfig;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }

  render() {
    return html` <div>
      <h2 class="mb-6 text-lg font-bold text-gray-500">Config</h2>
      <form class="bg-white shadow-md roundex px-8 pt-6 pb-8 mb-4" id="pump-config-form" @submit="${this.handleSubmit}">
        ${this.renderFormInput("< 10 °C", "le10")} ${this.renderFormInput("10 °C - 15 °C", "le15")}
        ${this.renderFormInput("15 °C - 20 °C", "le20")} ${this.renderFormInput("20 °C - 25 °C", "le25")}
        ${this.renderFormInput("> 25 °C", "gt25")}
        <button
          class="shadow bg-purple-500 hover:bg-purple-400 focus:shadow-outline focus:outline-none text-white font-bold py-2 px-4 rounded"
          type="submit"
        >
          Save
        </button>
      </form>
    </div>`;
  }

  renderFormInput(label: string, name: string) {
    return html` <div class="mb-4 flex">
      <label class="block text-gray-700 text-sm font-bold mb-2 mr-10" for=${name}>${label}</label>
      <input
        class="shadow border rounded py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
        id=${name}
        name=${name}
        required
        min="0"
        max="300"
        value="${this._pumpConfig[name]}"
        type="number"
        placeholder="Minutes"
      />
    </div>`;
  }
}
