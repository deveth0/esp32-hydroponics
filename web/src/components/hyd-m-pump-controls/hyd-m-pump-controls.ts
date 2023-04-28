import { customElement, state } from "lit/decorators.js";
import { html, LitElement } from "lit";
import { apiPostJson } from "../../util";
import { PumpStatusResponse } from "../../data/StatusResponse";

@customElement("hyd-m-pump-controls")
export class PumpConfig extends LitElement {
  @state()
  private _pumpStatus: PumpStatusResponse;

  createRenderRoot() {
    return this; // turn off shadow dom to access external styles
  }

  connectedCallback() {
    super.connectedCallback();
  }

  render() {
    return html` <div class="mb-12">
      <h2 class="mb-6 text-lg font-bold text-gray-500">Controls</h2>
      ${this._pumpStatus?.enabled
        ? html`<h4>Pump running until ${new Date(this._pumpStatus.runUntil).toString()}</h4>`
        : html` <button class="btn-primary" type="submit" @click="${() => this._runPump(60 * 1000)}">
              Start Pump for 1 minute
            </button>
            <button class="btn-primary" @click="${() => this._runPump(-1)}" type="submit">Empty tank</button>`}
    </div>`;
  }

  private _runPump(duration: number) {
    apiPostJson<unknown, PumpStatusResponse>("/api/pump.json", {
      duration,
    })
      .then(response => {
        this._pumpStatus = response;
      })
      .catch(error => {
        console.error("Error:", error);
      });
  }
}
