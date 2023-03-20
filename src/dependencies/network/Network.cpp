#include "Network.h"

IPAddress NetworkClass::localIP()
{
  IPAddress localIP = WiFi.localIP();
  if (localIP[0] != 0) {
    return localIP;
  }

  return INADDR_NONE;
}

IPAddress NetworkClass::subnetMask()
{
  if (WiFi.localIP()[0] != 0) {
    return WiFi.subnetMask();
  }
  return IPAddress(255, 255, 255, 0);
}

IPAddress NetworkClass::gatewayIP()
{
  if (WiFi.localIP()[0] != 0) {
      return WiFi.gatewayIP();
  }
  return INADDR_NONE;
}


bool NetworkClass::isConnected()
{
  return (WiFi.localIP()[0] != 0 && WiFi.status() == WL_CONNECTED);
}

NetworkClass Network;