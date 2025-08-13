#pragma once

#include "global.h"
#include "net/BluetoothConnection.h"
#include "net/ServerManager.h"
//#include "net/WifiConnectionManager.h"
//#include "net/WSConnection.h"

class Command;
class Event;
class Display;
class SettingsStore;

class Connector {
private:
  //WifiConnectionManager wifiManager;
  //WSConnection wsConnection;
  ServerManager serverManager;
  BluetoothConnection btConnection;

  Display* display = nullptr;

  SettingsStore& settings;

  bool isOnline = false; // есть какое то подключение
  bool hasBle = false; // включен BLE
  bool hasWifiConnection = false; // есть подключение к Wifi
  bool hasConnectedServer = false; // подключен к серверу через вебсокеты
  bool hasConnectedBle = false; // есть устройство подключенное по BT
  bool isPairing = false; // в режиме сопряжения (включен ble advertising)
  bool autoPairing = false; // автоматически принимать все соединения

  bool hasBleConfig = false; // есть BLE токен в настройках
  bool hasWifiConfig = false; // есть cloud токен в настройках
  bool isWifiCheckingCredentials = false; // режим проверки wifi доступов

  uint8_t initInfoSendingLevel = 0; // этап отправки стартовой информации об устройстве
  unsigned long initInfoSendingTime; // время начала отправки стартовой информации

  unsigned long startWifiCheckingTime; // время начала проверки wifi доступа (таймаут 30сек)

public:
  Connector(SettingsStore& settings);
  void setDisplay(Display* display);
  void start();
  void startBle();
  void loop();
  void bleLoop();
  void wifiLoop();
  void sendStatusQueueLoop();
  void setSendStatusQueue(uint8_t level);
  void onWifiConnected();
  void onWifiDisconnected();

  void startWifi();
  void startWifi(const std::string& wifiSsid, const std::string& wifiPassword);
  void checkWifi(const std::string& wifiSsid, const std::string& wifiPassword,
    const std::string& token, const std::string& host, const std::string& protocol, uint16_t port);
  void setWebsocket(const std::string& token, const std::string& host, const std::string& protocol, uint16_t port);

  //void webSocketLoop();
  //void startWebSocket();
  //void stopWebSocket();
  void addWebSocketConfig();

  void startPairing();
  void stopPairing();
  void resetWifiPairing();
  void resetBlePairing();
  void resetPairing();

  void publishConnectionState();
  void sendToBle(const char* msg);
  void sendToServer(const char* msg);
  void sentToDisplay(Event& event);
  void send(const char* msg);
  void send(std::string msg);
  void send(Event& event);
};
