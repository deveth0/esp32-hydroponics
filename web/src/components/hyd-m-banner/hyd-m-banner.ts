import StateService from "../../services/StateService";


class CatMBanner extends HTMLElement {

  size: number;
  readonly hiddenClassName = "hyd-m-banner--hidden";
  readonly warnClassName= "hyd-m-banner--warn";
  readonly infoClassName = "hyd-m-banner--info";
  warnText: string;
  infoText: string;
  textSpan: HTMLSpanElement;

  constructor() {
    super();
    this.size = parseInt(this.dataset.size) || 100;
    this.warnText = "";
    this.infoText = "";

  }

  handleStateChange() {
    console.log(StateService.calibrated);
    this.warnText = StateService.calibrated ? "" : "Not calibrated yet";
    if (this.infoText !== "") {
      this.classList.add(this.infoClassName)
      this.textSpan.innerText = this.infoText;
    } else {
      this.classList.remove(this.infoClassName)
    }
    if (this.warnText !== "") {
      this.classList.add(this.warnClassName)
      this.textSpan.innerText = this.warnText;
    } else {
      this.classList.remove(this.warnClassName)
    }
    if(this.infoText !== "" || this.warnText !== ""){
      this.classList.remove(this.hiddenClassName)
    } else {
      this.classList.add(this.hiddenClassName)
    }
  }

  connectedCallback() {
    this.renderComponent();
    this.addEvents();
  }

  renderComponent() {
    this.textSpan = this.renderText();
    this.appendChild(this.textSpan);
    this.classList.add("hyd-m-banner", this.hiddenClassName);
  }

  renderText() {
    const text = document.createElement("span");
    text.classList.add("hyd-m-banner__text");
    return text;
  }


  addEvents() {
    StateService.observeStateChanges("calibrated", () => this.handleStateChange());
    StateService.observeStateChanges("autoModeEnabled", () => this.handleStateChange());
  }


}

window.customElements.define('hyd-m-banner', CatMBanner);