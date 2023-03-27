import StateService from "./StateService";

class ApiService {

  constructor() {
    // update status regularly
    setInterval(this.status.bind(this), 1000);
  }

  status() {
    fetch('./api/status')
      .then(response => {
        if (!response.ok) {
          throw response
        }
        return response.json();
      })
      .then(data => StateService.setFromStatus(data))
      .catch((error) => {
        console.error('Error:', error);
      });
  }

 
  updateAutoMode(value: boolean) {
    this.updateConfig("autoMode", value);
  }

  updateConfig(key: string, value: any) {
    fetch(`./api/config?${key}=${value}`, {method: 'PUT'})
      .then(response => {
        if (!response.ok) {
          throw response
        }
        return response.json();
      })
      .then(data => StateService.setFromStatus(data))
      .catch((error) => {
        console.error('Error:', error);
      });
  }


}

export default new ApiService();
