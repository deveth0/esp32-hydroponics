import { customElement, query, state } from "lit/decorators.js";
import { html, LitElement, nothing } from "lit";
import { apiFetch } from "../../util";

enum WifiApiStatus {
  INPROGRESS = "inprogress",
  FAILED = "failed",
  SUCCESS = "success",
}

interface WifiApiResponse {
  status: WifiApiStatus;
  networks?: WifiNetwork[];
}

interface WifiNetwork {
  ssid: string;
  rssi: number;
  bssid: string;
  channel: number;
  enc: number;
}

export interface SSIDSelectedEvent {
  ssid: string;
}

/**
 * Renders a form element plus a "scan" option for ssids
 */
@customElement("hyd-m-wifi-ssid-picker")
export class WifiSsidPicker extends LitElement {
  @query("#CS")
  input: HTMLInputElement;
  @state()
  private _wifiNetworks: WifiNetwork[] = [];

  @state()
  private _status: WifiApiStatus;

  @state()
  private _isLoading = false;

  createRenderRoot() {
    return this; // turn off shadow dom to access external styles
  }

  // Render the UI as a function of component state
  render() {
    return html` <div class=" mb-4">
      <button
        ?disabled=${this._status === WifiApiStatus.INPROGRESS || this._isLoading}
        id="scan"
        class="btn-primary flex"
        @click="${this._loadWifiNetworks}"
        type="button"
      >
        Scan available networks
        ${this._isLoading
          ? html`<svg class="spinner" viewBox="0 0 24 24">
              <use href="#spinner"></use>
            </svg>`
          : nothing}
      </button>
      ${this._status !== WifiApiStatus.FAILED
        ? nothing
        : html`
            <div role="alert">
              <div class="bg-red-500 text-white font-bold rounded-t px-4 py-2">Wifi Scan failed, please try again</div>
            </div>
          `}
      ${this._wifiNetworks.length === 0
        ? nothing
        : html` <div class="border border-blue-400 rounded-b bg-blue-100 px-4 py-3 text-blue-700">
            ${this._wifiNetworks.map(
              network => html` <div class="cursor-pointer" @click="${() => this.onSelectSSID(network.ssid)}">
                ${network.ssid} (${network.rssi} dBm)
              </div>`,
            )}
          </div>`}
    </div>`;
  }

  private onSelectSSID(ssid: string) {
    this.dispatchEvent(
      new CustomEvent<SSIDSelectedEvent>("ssid-selected-event", {
        detail: { ssid },
      }),
    );
  }

  private _loadWifiNetworks() {
    const url = "/api/wifiscan.json";
    this._wifiNetworks = [];
    this._isLoading = true;
    apiFetch<WifiApiResponse>(url, {
      retries: 10,
      retryDelay: 2000,
      retryOn: [202, 500, 503],
    })
      .then(response => {
        this._status = response.status;
        return response.networks
          ? response.networks
              // unique networks
              .filter((value, index, array) => array.indexOf(value) === index)
              // Sort by signal strength.
              .sort((a, b) => b.rssi - a.rssi)
          : [];
      })
      .then(networks => {
        this._isLoading = false;
        this._wifiNetworks = networks;
      })
      .catch(() => {
        // do nothing
      });
  }
}
