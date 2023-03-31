import { customElement, property, query, state } from "lit/decorators.js";
import { html, LitElement, nothing } from "lit";
import { apiFetch } from "../../util";

interface WifiApiResponse {
  status: "inprogress" | "failed" | "success";
  networks: WifiNetwork[];
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
  @property({ type: Number }) scanLoops = 0;
  @query("#CS")
  input: HTMLInputElement;
  @state()
  private _wifiNetworks: WifiNetwork[] = [];

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
        id="scan"
        class="shadow bg-purple-500 hover:bg-purple-400 focus:shadow-outline focus:outline-none text-white font-bold py-2 px-4 rounded"
        @click="${this._loadWifiNetworks}"
        type="button"
      >
        Scan
      </button>
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

    apiFetch<WifiApiResponse>(url)
      .then(response => {
        return (
          response.networks
            // unique networks
            .filter((value, index, array) => array.indexOf(value) === index)
            // Sort by signal strength.
            .sort((a, b) => b.rssi - a.rssi)
        );
      })
      .then(networks => {
        // If there are no networks, fetch it again in a second.
        // but only do this a few times.
        if (networks.length === 0 && this.scanLoops < 10) {
          this.scanLoops++;
          setTimeout(this._loadWifiNetworks, 1000);
          return;
        }
        this.scanLoops = 0;

        this._wifiNetworks = networks;
      })
      .catch(() => {
        // do nothing
      });
  }
}
