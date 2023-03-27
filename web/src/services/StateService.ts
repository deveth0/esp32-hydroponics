import ObserverService from './ObserverService';
import ApiService from "./ApiService";

class StateService {

  #calibrated: boolean = true;


  constructor() {
    ApiService.status();
  }

  get calibrated(): boolean {
    return this.#calibrated;
  }

  set calibrated(value) {
    if (this.calibrated === value) return;
    this.#calibrated = value;
    ObserverService.notifyObservers("calibrated");
  }

  setFromStatus(json: any) {
    if (json.calibrated !== undefined) {
      this.calibrated = json.calibrated;
    }
  }

  observeStateChanges(stateName: string, callback: () => void) {
    ObserverService.observe(stateName, callback);
  }
}

export default new StateService();