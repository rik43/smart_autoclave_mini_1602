#include "commands/commands.h"
#include "net/connector.h"
#include "net/checkWifi.h"
#include "events/EventWifiCheckResult.h"
#include "events/EventDisplay.h"


extern Connector connector;


void CommandPairing::invoke() {
  if (active) {
    connector.startPairing();
  } else {
    connector.stopPairing();
  }
}

void CommandWifiProvisioning::invoke() {
  if (!ssid.empty()) {
    // int checkResult = checkWifi(ssid.c_str(), password.c_str());

    // if (checkResult == 0) {
    //   debugln("Wifi provisioning OK");
    
    connector.checkWifi(ssid, password, token, host, protocol, port);
    
    // } else {
    //   EventWifiCheckResult evt;
    //   evt.errorCode = checkResult;
    //   emit(evt);
    // }
  } 
  else if (!host.empty()) {
    connector.setWebsocket(token, host, protocol, port);
  }
}

void CommandWifiForget::invoke() {
  connector.resetWifiPairing();
}

void CommandConnectionWipe::invoke() {
  connector.resetPairing();

  // show "complete" screen on display
  if (!isInternal) {
    EventDisplay evt(DISPLAY_COMPLETE);
    emit(evt);
  }
}


