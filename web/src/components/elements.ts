import { html, nothing } from "lit";

export function renderButton(text: string, isLoading = false, disabled = false, type = "submit") {
  return html`<button ?disabled=${isLoading || disabled} class="btn-primary" type="${type}">
    ${text}
    ${isLoading
      ? html`<svg class="spinner" viewBox="0 0 24 24">
          <use href="#spinner"></use>
        </svg>`
      : nothing}
  </button>`;
}
