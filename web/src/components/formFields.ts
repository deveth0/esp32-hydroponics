import { html, TemplateResult } from "lit";

export function renderFormInputText(label: string, name: string, value?: string, password = false): TemplateResult {
  return html` <div class="form-field">
    <label class="form-input-label" for="${name}">${label}</label>
    <input class="form-input" type="${password ? "password" : "text"}" id="${name}" name="${name}" value="${value}" />
  </div>`;
}

export function renderFormInputNumber(
  label: string,
  name: string,
  value?: number,
  step?: number,
  min?: number,
  max?: number,
): TemplateResult {
  return html` <div class="form-field">
    <label class="form-input-label" for="${name}">${label}</label>
    <input
      class="form-input"
      type="number"
      id="${name}"
      name="${name}"
      value="${value}"
      step="${step}"
      min="${min}"
      max="${max}"
    />
  </div>`;
}

export function renderFormInputCheckbox(label: string, name: string, value?: boolean): TemplateResult {
  return html` <div class="form-field">
    <label class="block text-grey-700 text-sm font-bold mb-2 mr-10" for="${name}">${label}</label>
    <input type="checkbox" ?checked="${value}" id="${name}" name="${name}" />
  </div>`;
}
