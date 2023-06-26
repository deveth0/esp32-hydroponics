import { customElement, state } from "lit/decorators.js";
import { html, LitElement, nothing } from "lit";
import { apiPostJson } from "../../util";
import { PumpStatusResponse } from "../../data/StatusResponse";

@customElement("hyd-m-pump-controls")
export class PumpConfig extends LitElement {
  @state()
  private _pumpStatus: PumpStatusResponse;
  @state()
  private _isLoading = false;

  createRenderRoot() {
    return this; // turn off shadow dom to access external styles
  }

  connectedCallback() {
    super.connectedCallback();
  }

  render() {
    return html` <div class="mb-12">
      <h2 class="page-headline">Controls</h2>
      ${this._pumpStatus?.enabled
        ? html`<h4>Pump running for ${this._pumpStatus.runningFor / 1000} seconds)}</h4>`
        : html`<div class="flex space-x-4">
            <button
              class="btn-primary"
              ?disabled=${this._isLoading}
              type="submit"
              @click="${() => this._runPump(60 * 1000)}"
            >
              Start Pump for 1 minute
            </button>
            <button class="btn-primary" ?disabled=${this._isLoading} @click="${() => this._runPump(-1)}" type="submit">
              Empty tank
            </button>
          </div>`}
    </div>`;
  }

  private _runPump(duration: number) {
    this._isLoading = true;
    apiPostJson<unknown, PumpStatusResponse>("/api/pump.json", {
      duration,
    })
      .then(response => {
        this._pumpStatus = response;
      })
      .catch(error => {
        console.error("Error:", error);
      })
      .finally(() => {
        this._isLoading = false;
      });
  }
}
