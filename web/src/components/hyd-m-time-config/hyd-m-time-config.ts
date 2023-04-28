import { customElement, query, state } from "lit/decorators.js";
import { html, LitElement } from "lit";
import { apiFetch, apiPostJson } from "../../util";

interface TimeConfigResponse {
  ntpServer: string;
  timezone: number;
  longitude: number;
  latitude: number;
}

@customElement("hyd-m-time-config")
export class TimeConfig extends LitElement {
  @state()
  _timeConfig: TimeConfigResponse;

  @query("#time-config-form")
  configForm: HTMLFormElement;

  createRenderRoot() {
    return this; // turn off shadow dom to access external styles
  }

  connectedCallback() {
    super.connectedCallback();
    this.fetchConfig();
  }

  fetchConfig() {
    apiFetch<TimeConfigResponse>("/api/config/time.json")
      .then(response => {
        this._timeConfig = response;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }

  handleSubmit(event: SubmitEvent) {
    event.preventDefault();

    const formData = new FormData(this.configForm);

    const submitData = {
      ntpServer: formData.get("ntpServer"),
      timezone: formData.get("timezone"),
      latitude: formData.get("latitude"),
      longitude: formData.get("longitude"),
    };

    apiPostJson<unknown, TimeConfigResponse>("/api/config/time.json", submitData)
      .then(response => {
        this._timeConfig = response;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }

  render() {
    return html` <div>
      <h2 class="mb-6 text-lg font-bold text-gray-500">Config</h2>
      <form id="time-config-form" @submit="${this.handleSubmit}">
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Time</legend>
          <label for="ntpServer">NTP Server</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="string"
            id="ntpServer"
            name="ntpServer"
            value="${this._timeConfig?.ntpServer}"
          />
        </fieldset>
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">Location</legend>
          <label for="latitude">Latitude</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="latitude"
            name="latitude"
            step="0.01"
            value="${this._timeConfig?.latitude}"
          />
          <label for="longitude">Longitude</label>
          <input
            class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
            type="number"
            id="longitude"
            name="longitude"
            step="0.01"
            value="${this._timeConfig?.longitude}"
          />
        </fieldset>
        <button class="btn-primary" type="submit">Save</button>
      </form>
    </div>`;
  }
}
