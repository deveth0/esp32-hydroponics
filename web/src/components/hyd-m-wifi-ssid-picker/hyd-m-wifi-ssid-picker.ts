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
    return html`
      <div class="mb-4">
        <label class="block text-gray-700 text-sm font-bold mb-2" for="CS">
          Network name (SSID, empty to not connect)
        </label>
        <input
          class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline"
          id="CS"
          name="CS"
          placeholder="SSID"
          type="text"
        />
      </div>
      <button
        ?disabled=${this._status === WifiApiStatus.INPROGRESS || this._isLoading}
        id="scan"
        class="btn-primary"
        @click="${this._loadWifiNetworks}"
        type="button"
      >
        Scan
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
              network => html` <div class="cursor-pointer" @click="${() => (this.input.value = network.ssid)}">
                ${network.ssid} (${network.rssi} dBm)
              </div>`,
            )}
          </div>`}
    `;
  }

  private _loadWifiNetworks() {
    const url = "/api/wifi.json";
    this._wifiNetworks = [];
    this._isLoading = true;
    apiFetch<WifiApiResponse>(url, {
      retries: 10,
      retryDelay: 1000,
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
