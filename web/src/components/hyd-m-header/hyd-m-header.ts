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
      <header class="hyd-m-header px-4 md:px-8 flex flew-row mb-4 md:mb-8 bg-gray-700 items-center">
        <a class="flex flex-row" href="/">
          <svg class="h-20 mr-4" viewBox="0 0 303 303">
            <use href="#logo_small"></use>
          </svg>
        </a>
        <h3 class="text-primary text-2xl md:text-4xl font-bold">ESP32-hydroponics</h3>
        <a
          class="ml-auto text-white inline-flex items-center hover:text-secondary hover:fill-secondary"
          href="/settings.html"
        >
          <svg class="h-6" viewBox="0 96 960 960">
            <use href="#settings"></use>
          </svg>
          <span class="hidden md:block md:ml-2">Settings</span></a
        >
      </header>
    `;
  }
}
