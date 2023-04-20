import { customElement } from "lit/decorators.js";
import { html, LitElement } from "lit";

@customElement("hyd-m-header")
export class Header extends LitElement {
  createRenderRoot() {
    return this; // turn off shadow dom to access external styles
  }

  render() {
    return html`
      <hyd-m-svg-loader></hyd-m-svg-loader>
      <header class="hyd-m-header flex flew-row mb-8">
        <h3 class="text-primary text-4xl font-bold">
          <a href="/">ESP32-hydroponics </a>
        </h3>
        <a class="ml-auto inline-flex items-center hover:text-secondary hover:fill-secondary" href="/settings.html">
          <svg class="h-6" viewBox="0 96 960 960">
            <use href="#settings"></use>
          </svg>
          <span>Settings</span></a
        >
      </header>
    `;
  }
}
