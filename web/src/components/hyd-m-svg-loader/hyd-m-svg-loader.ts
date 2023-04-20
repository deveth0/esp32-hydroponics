import { customElement, state } from "lit/decorators.js";
import { html, LitElement, nothing } from "lit";
import { unsafeHTML } from "lit/directives/unsafe-html.js";

@customElement("hyd-m-svg-loader")
export class SvgLoader extends LitElement {
  @state()
  private _svgSprite: string;

  createRenderRoot() {
    return this; // turn off shadow dom to access external styles
  }

  connectedCallback() {
    super.connectedCallback();
    this.fetchSvg();
  }

  render() {
    if (this._svgSprite) return html` <div style="display:none">${unsafeHTML(this._svgSprite)}</div>`;
    return nothing;
  }

  private fetchSvg() {
    fetch("/sprite.svg")
      .then(response => {
        if (!response.ok) throw new Error();
        return response.text();
      })
      .then(sprite => {
        this._svgSprite = sprite;
      })
      .catch(error => console.error(error));
  }
}
