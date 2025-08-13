#include <string>
#include "global.h"
#include "net/connector.h"
#include "net/cert-le.h"
#include "events/event.h"
#include "events/EventConnection.h"
#include "events/EventDescriptor.h"
#include "events/EventWifiCheckResult.h"
#include "commands/command.h"
#include "commands/commands_prog_create.h"
// #include "display/Display.h"
#include "settings/SettingsStore.h"


static struct CheckCredentials {
  std::string ssid;
  std::string password;
  std::string token;
  std::string host;
  std::string protocol;
  uint16_t port;
} checkCredentials;


Connector::Connector(SettingsStore& settings) : settings(settings) {}

// void Connector::setDisplay(Display* _display) {
//   display = _display;
// }

// прочитать настройки и если возможно соединиться
void Connector::start() {
  startWifi();
  startBle();
  publishConnectionState();
}

void Connector::startBle() {
  std::string token = settings.getBleToken();
  if (! token.empty()) {
    prt("ble", token.c_str());
    btConnection.setToken(token);
    hasBleConfig = true;
  } else {
    debugln("no ble tok");
  }
  btConnection.start();
  btConnection.startAdvertising();
  btConnection.allowPairing = false;
  hasBle = true;
}

void Connector::loop() {
  bleLoop();
  wifiLoop();
  sendStatusQueueLoop();
  //webSocketLoop();
}

void Connector::bleLoop() {
  if (btConnection.isOnline() != hasConnectedBle) {
    hasConnectedBle = btConnection.isOnline();
    // если нет конфига и кто-то подключился
    if (!hasBleConfig && hasConnectedBle) {
      prt("SET BLE TOK", btConnection.getToken().c_str());
      settings.setBleToken(btConnection.getToken().c_str());
      hasBleConfig = true;

      if (isPairing) {
        stopPairing();
      }
    }
    publishConnectionState();
  }
}

// отправка общей информации об устройстве после подключения
void Connector::sendStatusQueueLoop() {
  if (initInfoSendingLevel <= 0) return;

  if (millis() - initInfoSendingTime > 100) {
    switch (initInfoSendingLevel) {
      case 3: // инфа об устройстве
        {
          EventDescriptor evt;
          emit(evt);
        }
        break;

      case 2: // запущенная программа, если есть
        {
          CommandProgInfo cmd;
          dispatch(&cmd);
        }
        break;

      case 1: // статусы соединений
        publishConnectionState();
        break;
    }

    setSendStatusQueue(initInfoSendingLevel - 1);
  }
}

void Connector::setSendStatusQueue(uint8_t level) {
  initInfoSendingLevel = level;
  initInfoSendingTime = millis();
}

//Timer timerPrint(1000);

void Connector::wifiLoop() {
  bool isStateChanged = false;

  serverManager.loop();

  // 30сек на проверку wifi ssid/password
  //if (isWifiCheckingCredentials) {
  //  if (timerPrint.ready()) {
  //    prt("wifi check mode", millis() - startWifiCheckingTime);
  //  }
  //}

  // проверка затянулась - передать в телефон fail
  if (isWifiCheckingCredentials && (millis() - startWifiCheckingTime > 30000)) {
    isWifiCheckingCredentials = false;
    serverManager.stop();
    
    EventWifiCheckResult evt;
    evt.errorCode = 3;
    emit(evt);
  }

  if (serverManager.isWifiConnected() != hasWifiConnection) { // is wifi online changed
    hasWifiConnection = serverManager.isWifiConnected();
    if (hasWifiConnection) { // connected
      onWifiConnected();
    }
    else { // disconnected
      onWifiDisconnected();
    }
    //publishConnectionState();
    isStateChanged = true;
  }

  if (serverManager.isOnline() != hasConnectedServer) {
    hasConnectedServer = !hasConnectedServer;
    isStateChanged = true;
  }

  if (isStateChanged) { // отправить события состояния
    setSendStatusQueue(hasConnectedServer ? 1 : 3);
  }
}

void Connector::onWifiConnected() {
  debugln("Connector::onWifiConnected");
  // был запущен режим проверки настроек подключения
  if (isWifiCheckingCredentials) {
    settings.setWifiSsid(checkCredentials.ssid.c_str());
    settings.setWifiPassword(checkCredentials.password.c_str());
    settings.setServerToken(checkCredentials.token.c_str());
    settings.setWebsocketHost(checkCredentials.host.c_str());
    settings.setWebsocketProtocol(checkCredentials.protocol.c_str());
    settings.setWebsocketPort(checkCredentials.port);
    isWifiCheckingCredentials = false;
    hasWifiConfig = true;
  }

  //startWebSocket();
}

void Connector::setWebsocket(const std::string& token, const std::string& host, const std::string& protocol, uint16_t port) {
  settings.setServerToken(token.c_str());
  settings.setWebsocketHost(host.c_str());
  settings.setWebsocketProtocol(protocol.c_str());
  settings.setWebsocketPort(port);
  //stopWebSocket();

  serverManager.setToken(token);
  serverManager.setServer(host, protocol, port);
  delay(20);

  //startWebSocket();
}

void Connector::onWifiDisconnected() {
  debugln("Connector::onWifiDisconnected");
  //stopWebSocket();
  //isWifiCheckingCredentials = false;
}

// Подключение к wifi с настройками по умолчанию
void Connector::startWifi() {
  // password может быть пустым, но не ssid
  std::string wifiSsid = settings.getWifiSsid();
  if (wifiSsid.length() != 0) {
    std::string wifiPassword = settings.getWifiPassword();
    addWebSocketConfig();
    startWifi(wifiSsid, wifiPassword);
    hasWifiConfig = true;
  }
  else {
    debugln("no wifi credentials");
  }
}

void Connector::addWebSocketConfig() {
  std::string token = settings.getServerToken();
  
  std::string host = settings.getWebsocketHost();
  std::string protocol = settings.getWebsocketProtocol();
  uint16_t port = settings.getWebsocketPort();

  serverManager.setToken(token);
  serverManager.setServer(host, protocol, port);
}

void Connector::startWifi(const std::string& wifiSsid, const std::string& wifiPassword) {
  //serverManager.stop(); // start запускает stop если подключено
  serverManager.wifiSetup(wifiSsid.c_str(), wifiPassword.c_str());
  serverManager.start();
}

// wifi provisioning
// проверить wifi credentials и сохранить их, если они валидные
void Connector::checkWifi(
  const std::string& ssid,
  const std::string& password,
  const std::string& token,
  const std::string& host,
  const std::string& protocol,
  uint16_t port
) {

  // сохраняем временно, если подключиться - то в общее хранилище
  checkCredentials.ssid = ssid;
  checkCredentials.password = password;
  checkCredentials.token = token;
  checkCredentials.host = host;
  checkCredentials.protocol = protocol;
  checkCredentials.port = port;
  // сохраняем в NVS в onWifiConnected() при успешном подключении

  isWifiCheckingCredentials = true;
  startWifiCheckingTime = millis();

  hasWifiConfig = false;
  //hasWifiConnection = false;
  //hasConnectedServer = false;
  publishConnectionState(); // reset icons in display
  delay(10);

  serverManager.setToken(token);
  serverManager.setServer(host, protocol, port);

  startWifi(ssid, password);
}

/*
void Connector::webSocketLoop() {
  if (hasWifiConnection) {
    //wsConnection.loop();
  }
  if (wsConnection.isOnline() != hasConnectedServer) {
    hasConnectedServer = wsConnection.isOnline();
    publishConnectionState();

    if (hasConnectedServer) {
      // после подключения сразу отправляем инфо о себе
      EventDescriptor evt;
      emit(evt);

      // и текущую программу (эмулируем команду чтения программы)
      CommandProgInfo cmd;
      dispatch(cmd);
    }
  }
}

void Connector::startWebSocket() {
  std::string token = settings.getServerToken();
  
  std::string host = settings.getWebsocketHost();
  std::string protocol = settings.getWebsocketProtocol();
  uint16_t port = settings.getWebsocketPort();

  wsConnection.setToken(token);
  wsConnection.setServer(host, protocol, port);
  wsConnection.start();
}

void Connector::stopWebSocket() {
  wsConnection.stop();
  //hasConnectedServer = false;
}
*/

void Connector::startPairing() {
  btConnection.startAdvertising();
  //hasBle = true;
  isPairing = true;
  publishConnectionState();
}

void Connector::stopPairing() {
  btConnection.stopAdvertising();
  //hasBle = false;
  isPairing = false;
  publishConnectionState();
}

// сброс настроек сопряжения wifi
void Connector::resetWifiPairing() {
  //stopWebSocket();
  serverManager.stop();
  settings.clearWifi();
  settings.clearCloud();
  hasWifiConfig = false;
  publishConnectionState();
}

// сброс настроек сопряжения BLE
void Connector::resetBlePairing() {
  btConnection.stop();
  settings.clearBle();
  hasBleConfig = false;
}

// отключиться от всех соединений и забыть их
void Connector::resetPairing() {
  resetBlePairing();
  resetWifiPairing();
}

void Connector::publishConnectionState() {
  EventConnection event;
  event.ble = hasBle;
  event.wifi = hasWifiConnection;
  event.server = hasConnectedServer;
  event.ble_config = hasBleConfig;
  event.wifi_config = hasWifiConfig;
  event.ble_client = hasConnectedBle;
  event.wifi_checking = isWifiCheckingCredentials;
  event.pairing = isPairing;
  send(event);
}

void Connector::sendToBle(const char* msg) {
  btConnection.sendEvent(msg);
}

void Connector::sendToServer(const char* msg) {
  serverManager.send(msg);
}

void Connector::sentToDisplay(Event& event) {
  // if (display != nullptr) {
  //   display->dispatchEvent(event);
  // }
}

void Connector::send(const char* msg) {
  sendToServer(msg);
  sendToBle(msg);

  prt("EVT", msg);
}

void Connector::send(std::string msg) {
  send(msg.c_str());
}

void Connector::send(Event& event) {
  send(event.getMessage().c_str());
  sentToDisplay(event);
}
