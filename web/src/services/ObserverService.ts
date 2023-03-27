class ObserverService {

  observerMap: Map<string, Set<() => void>>;
  
  constructor() {
    this.observerMap = new Map();
  }

  observe(observedKey: string, callback: () => void) {
    const curCallbacks = this.getCallbacks(observedKey);
    curCallbacks.add(callback);
    this.observerMap.set(observedKey, curCallbacks);
  }

  unobserve(observedKey: string, callback: () => void) {
    const curCallbacks = this.getCallbacks(observedKey);
    curCallbacks.delete(callback);
    this.observerMap.set(observedKey, curCallbacks);
  }

  notifyObservers(observedKey: string) {
    const callbacks = this.getCallbacks(observedKey);
    callbacks.forEach(callback => callback());
  }

  getCallbacks(observedKey: string) {
    return this.observerMap.get(observedKey) || new Set<()=> void>();
  }
}

export default new ObserverService();