import { customElement, query, queryAsync, state } from "lit/decorators.js";
import { html, LitElement, nothing } from "lit";
import { apiFetch, apiPostJson } from "../../util";
import { renderFormInputCheckbox, renderFormInputNumber, renderFormInputText } from "../formFields";
import { SSIDSelectedEvent } from "../hyd-m-wifi-ssid-picker/hyd-m-wifi-ssid-picker";
import { renderButton } from "../elements";

interface WifiConfigResponse {
  wifi: {
    ssid: string;
    pwd?: string;
    staticIp: number[];
    gateway: number[];
    subnet: number[];
  };
  mdns: {
    address?: string;
  };
  ap: {
    ssid: string;
    pwd?: string;
    hideAp: boolean;
    channel: number;
    opensOn: number;
  };
}

@customElement("hyd-m-wifi-config")
export class MqttConfig extends LitElement {
  @state()
  _wifiConfig: WifiConfigResponse = undefined;
  @query("#wifi-config-form")
  configForm: HTMLFormElement;
  @query("#wifiSSID")
  ssidInput: HTMLInputElement;
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
    apiFetch<WifiConfigResponse>("/api/config/wifi.json")
      .then(response => {
        this._wifiConfig = response;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }

  handleSubmit(event: SubmitEvent) {
    event.preventDefault();
    this._isLoading = true;

    const formData = new FormData(this.configForm);

    apiPostJson<unknown, WifiConfigResponse>("/api/config/wifi.json", {
      wifi: {
        ssid: formData.get("wifiSSID"),
        pwd: formData.get("wifiPwd"),
        staticIp: [formData.get("I0"), formData.get("I1"), formData.get("I2"), formData.get("I3")],
        gateway: [formData.get("G0"), formData.get("G1"), formData.get("G2"), formData.get("G3")],
        subnet: [formData.get("S0"), formData.get("S1"), formData.get("S2"), formData.get("S3")],
      },
      mdns: {
        address: formData.get("mdnsAddress"),
      },
      ap: {
        ssid: formData.get("apSSID"),
        pwd: formData.get("apPwd"),
        hideAp: formData.get("apHideAp") === "on",
        channel: formData.get("apChannel"),
        opensOn: formData.get("apOpensOn"),
      },
    })
      .then(response => {
        this._wifiConfig = response;
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
      <form id="wifi-config-form" @submit="${this.handleSubmit}">
        <fieldset class="form-fieldset">
          <legend class="form-fieldset-legend">Wifi</legend>
          ${renderFormInputText("SSID", "wifiSSID", this._wifiConfig?.wifi.ssid)}
          <hyd-m-wifi-ssid-picker
            @ssid-selected-event="${(e: CustomEvent<SSIDSelectedEvent>) => {
              this.ssidInput.value = e.detail.ssid;
            }}"
          ></hyd-m-wifi-ssid-picker>
          ${renderFormInputText("Password", "wifiPwd", this._wifiConfig?.wifi.pwd, true)}
          <fieldset class="form-field">
            <legend class="form-input-label mb-4">Static IP (leave at 0.0.0.0 for DHCP)</legend>
            <div class="flex col-span-3 grid grid-cols-4">
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="I0"
                required
                type="number"
                value="${this._wifiConfig?.wifi.staticIp[0]}"
              />
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="I1"
                required
                type="number"
                value="${this._wifiConfig?.wifi.staticIp[1]}"
              />
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="I2"
                required
                type="number"
                value="${this._wifiConfig?.wifi.staticIp[2]}"
              />
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="I3"
                required
                type="number"
                value="${this._wifiConfig?.wifi.staticIp[3]}"
              />
            </div>
          </fieldset>
          <fieldset class="form-field">
            <legend class="form-input-label mb-4">Static gateway</legend>
            <div class="flex col-span-3 grid grid-cols-4">
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="G0"
                required
                type="number"
                value="${this._wifiConfig?.wifi.gateway[0]}"
              />
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="G1"
                required
                type="number"
                value="${this._wifiConfig?.wifi.gateway[1]}"
              />
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="G2"
                required
                type="number"
                value="${this._wifiConfig?.wifi.gateway[2]}"
              />
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="G3"
                required
                type="number"
                value="${this._wifiConfig?.wifi.gateway[3]}"
              />
            </div>
          </fieldset>
          <fieldset class="form-field">
            <legend class="form-input-label mb-4">Static subnet mask</legend>
            <div class="flex col-span-3 grid grid-cols-4">
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="S0"
                required
                type="number"
                value="${this._wifiConfig?.wifi.subnet[0]}"
              />
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="S1"
                required
                type="number"
                value="${this._wifiConfig?.wifi.subnet[1]}"
              />
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="S2"
                required
                type="number"
                value="${this._wifiConfig?.wifi.subnet[2]}"
              />
              <input
                class="form-input col-span-1"
                max="255"
                min="0"
                name="S3"
                required
                type="number"
                value="${this._wifiConfig?.wifi.subnet[3]}"
              />
            </div>
          </fieldset>
        </fieldset>
        <fieldset class="form-fieldset">
          <legend class="form-fieldset-legend">MDNS</legend>
          ${renderFormInputText("MDNS Address", "mdnsAddress", this._wifiConfig?.mdns.address)}
        </fieldset>
        <fieldset class="form-fieldset">
          <legend class="form-fieldset-legend">Accesspoint</legend>
          ${renderFormInputText("Accesspoint SSID", "apSSID", this._wifiConfig?.ap.ssid)}
          ${renderFormInputText("Password", "apPwd", this._wifiConfig?.ap.pwd, true)}
          ${renderFormInputCheckbox("Hide AP", "apHideAp", this._wifiConfig?.ap.hideAp)}
          ${renderFormInputNumber("Wifi Channel", "apChannel", this._wifiConfig?.ap.channel, 1, 1, 13)}
          <div class="form-field">
            <label class="form-input-label" for="apOpensOn"> AP opens </label>
            <select class="form-input" id="apOpensOn" name="apOpensOn">
              <option value="0">No connection after boot</option>
              <option value="1">Disconnected</option>
              <option value="2">Always</option>
              <option value="3">Never (not recommended)</option>
            </select>
          </div>
        </fieldset>
        ${renderButton("Save", this._isLoading, this._wifiConfig === undefined)}
      </form>
    </div>`;
  }
}
