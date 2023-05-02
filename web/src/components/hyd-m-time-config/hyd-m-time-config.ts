import { customElement, query, state } from "lit/decorators.js";
import { html, LitElement } from "lit";
import { apiFetch, apiPostJson } from "../../util";
import { renderButton } from "../elements";
import { renderFormInputNumber, renderFormInputText } from "../formFields";

interface TimeConfigResponse {
  ntpServer: string;
  longitude: number;
  latitude: number;
}

@customElement("hyd-m-time-config")
export class TimeConfig extends LitElement {
  @state()
  _timeConfig: TimeConfigResponse;
  @query("#time-config-form")
  configForm: HTMLFormElement;
  @state()
  private _isLoading = false;

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
    this._isLoading = true;
    const formData = new FormData(this.configForm);

    const submitData = {
      ntpServer: formData.get("ntpServer"),
      latitude: formData.get("latitude"),
      longitude: formData.get("longitude"),
    };

    apiPostJson<unknown, TimeConfigResponse>("/api/config/time.json", submitData)
      .then(response => {
        this._timeConfig = response;
      })
      .catch(error => {
        console.error("Error:", error);
      })
      .finally(() => {
        this._isLoading = false;
      });
  }

  render() {
    return html` <div>
      <h2 class="page-headline">Config</h2>
      <form id="time-config-form" @submit="${this.handleSubmit}">
        <fieldset class="form-fieldset">
          <legend class="form-fieldset-legend">Time</legend>
          ${renderFormInputText("NTP Server", "ntpServer", this._timeConfig?.ntpServer)}
        </fieldset>
        <fieldset class="form-fieldset">
          <legend class="form-fieldset-legend">Location</legend>
          ${renderFormInputNumber("Latitude", "latitude", this._timeConfig?.latitude, 0.01, -90.0, 90)}
          ${renderFormInputNumber("Longitude", "longitude", this._timeConfig?.longitude, 0.01, -90, 90)}
        </fieldset>
        ${renderButton("Save", this._isLoading, this._timeConfig === undefined)}
      </form>
    </div>`;
  }
}
