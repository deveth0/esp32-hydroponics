import { customElement, query, state } from "lit/decorators.js";
import { html, LitElement } from "lit";
import { apiFetch, apiPostJson } from "../../util";
import { renderFormInputCheckbox, renderFormInputNumber, renderFormInputText } from "../formFields";
import { renderButton } from "../elements";

interface MqttConfigResponse {
  enabled: boolean;
  broker: string;
  port: number;
  user: string;
  pwd: string;
  clientId: string;
  deviceTopic: string;
  groupTopic: string;
}

@customElement("hyd-m-mqtt-config")
export class MqttConfig extends LitElement {
  @state()
  _mqttConfig: MqttConfigResponse = undefined;
  @query("#mqtt-config-form")
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
    apiFetch<MqttConfigResponse>("/api/config/mqtt.json")
      .then(response => {
        this._mqttConfig = response;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }

  handleSubmit(event: SubmitEvent) {
    event.preventDefault();
    this._isLoading = true;

    const formData = new FormData(this.configForm);

    apiPostJson<unknown, MqttConfigResponse>("/api/config/mqtt.json", {
      enabled: formData.get("mqttEnabled") === "on",
      broker: formData.get("mqttBroker"),
      port: formData.get("mqttPort"),
      user: formData.get("mqttUser"),
      pwd: formData.get("mqttPassword") !== "" ? formData.get("mqttPassword") : null,
      clientId: formData.get("mqttClientId"),
      deviceTopic: formData.get("mqttDeviceTopic"),
      groupTopic: formData.get("mqttGroupTopic"),
    })
      .then(response => {
        this._mqttConfig = response;
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
      <h2 class="mb-6 text-lg font-bold text-gray-300">Config</h2>
      <form id="mqtt-config-form" @submit="${this.handleSubmit}">
        <fieldset class="border border-solid border-gray-300 p-3">
          <legend class="text-sm">MQTT</legend>
          ${renderFormInputCheckbox("Enable MQTT", "mqttEnabled", this._mqttConfig?.enabled)}
          ${renderFormInputText("MQTT Broker", "mqttBroker", this._mqttConfig?.broker)}
          ${renderFormInputNumber("Port", "mqttPort", this._mqttConfig?.port)}
          <div role="alert" class="mb-4">
            <div class="bg-red-500 text-white font-bold rounded-t px-4 py-2">WARNING</div>
            <div class="border border-t-0 border-red-400 rounded-b bg-red-100 px-4 py-3 text-red-700">
              <p>
                The MQTT credentials are sent over an unsecured connection.<br />
                Never use the MQTT password for another service!
              </p>
            </div>
          </div>
          ${renderFormInputText("Username", "mqttUser", this._mqttConfig?.user)}
          ${renderFormInputText("Password", "mqttPassword", this._mqttConfig?.pwd, true)}
          ${renderFormInputText("Client ID", "mqttClientId", this._mqttConfig?.clientId)}
          ${renderFormInputText("Device Topic", "mqttDeviceTopic", this._mqttConfig?.deviceTopic)}
          ${renderFormInputText("Group Topic", "mqttGroupTopic", this._mqttConfig?.groupTopic)}
        </fieldset>
        ${renderButton("Save", this._isLoading, this._mqttConfig === undefined)}
      </form>
    </div>`;
  }
}
