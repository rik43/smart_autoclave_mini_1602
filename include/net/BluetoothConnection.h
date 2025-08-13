#pragma once
#include "bluetooth_const.h"
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <WiFi.h>
#include "timer.h"
#include "global.h"
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

class BluetoothConnection {
private:
  BLEServer* pServer = nullptr;
  BLECharacteristic* pCommandCharacteristic = nullptr;
  BLECharacteristic* pEventCharacteristic = nullptr;

  //bool btDeviceConnected = false;
  std::string pairingToken; // токен для подключения устройства
  NimBLEAddress* pairedDeviceAddress = nullptr; // адрес подключенного БТ устройства

public:
  bool allowPairing = false;

  BluetoothConnection() = default; // Явно указываем, что конструктор по умолчанию разрешен
  BluetoothConnection(const BluetoothConnection&) = delete; // copy disable
  BluetoothConnection& operator=(const BluetoothConnection&) = delete; // copy disable

  // запускает BT без advertising (можно подключиться по мак-адресу)
  void start() {

    BLEDevice::init("ZAG-AVT-01");
    BLEDevice::setMTU(512); // макс размер сообщения
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyBLEServerCallbacks(this));
    //pServer->advertiseOnDisconnect(true);

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();

    // Add services
    pAdvertising->addServiceUUID(setupBluetoothServiceBus());

    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);

    //pAdvertising->start();
  }

  // вместо остановки BT забывает сопряженное устройство
  void stop() {
    // прощаемся если кто-то был подключен
    sendEventAnyway("@goodbye@");
    // сбрасываем токен
    pairingToken.clear();
    // удаляем спаренное устройство
    deletePairedDevice();
    //BLEDevice::deinit();
    disconnectAllClients();
  }

  bool isOnline() {
    return pairedDeviceAddress != nullptr;
  }

  void stopAdvertising() {
    allowPairing = false;
    //BLEDevice::getAdvertising()->stop();
    debugln("BLE advertising stopped");
  }

  void startAdvertising() {
    allowPairing = true;
    BLEDevice::getAdvertising()->start();
    debugln("BLE advertising started");
  }

  void sendEvent(const char* message) {
    // может быть подключенное устройство, но не авторизованное - ему не отправляем
    if (pairedDeviceAddress != nullptr) {
      sendEventAnyway(message);
    }
  }

  void sendEvent(const std::string& message) {
    sendEvent(message.c_str());
  }

  void setToken(std::string token) {
    pairingToken = token;
  }

  std::string getToken() {
    return pairingToken;
  }

private:

  void sendEventAnyway(const char* message) {
    pEventCharacteristic->setValue((uint8_t*)message, strlen(message));
    pEventCharacteristic->notify();
  }


  class BLECallbackCommand : public NimBLECharacteristicCallbacks {
    BluetoothConnection* parent;
  public:
    BLECallbackCommand(BluetoothConnection* parent) : parent(parent) {}

    void onWrite(BLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) override {
      std::string value = pCharacteristic->getValue();
      NimBLEAddress deviceAddress(desc->peer_id_addr);
      parent->onMessage(deviceAddress, value);
    }
  };

  void onMessage(const NimBLEAddress& deviceAddress, const std::string& message) {
    if (pairedDeviceAddress != nullptr && deviceAddress == *pairedDeviceAddress) {
      dispatch(message.c_str());
    }
    else { // not authorized
      onNonAuthorizedMessage(deviceAddress, message);
    }
  }

  void onNonAuthorizedMessage(const NimBLEAddress& deviceAddress, const std::string& message) {
    debugln("BLE non authorized message: " + String(message.c_str()));
    // Проверяем, начинается ли полученная строка с "@hello@"
    if (message.rfind("@hello@", 0) == 0) {
      std::string token = message.substr(7); // токен следует сразу после "@hello@"

      if (!pairingToken.empty()) { // есть сопряжение
        if (pairingToken == token) { // auth-ok
          deletePairedDevice();
          pairedDeviceAddress = new NimBLEAddress(deviceAddress); // new connect from friend
          sendEventAnyway("@auth-ble-ok@");
          debugln("BT: auth ok");
        }
        else { // not friend
          sendEventAnyway("@auth-ble-fail@error=busy"); // занято
          debugln("BT: error=busy");
        }
      }
      else { // не сопряжен ни с кем - сопрягаемся с ним
        if (allowPairing) { // в режиме сопряжения
          deletePairedDevice();
          pairedDeviceAddress = new NimBLEAddress(deviceAddress);
          pairingToken = token;
          sendEventAnyway("@auth-ble-ok@");
          stopAdvertising();
          debugln("Token saved: " + String(token.c_str()));
          debugln("Device address: " + String(deviceAddress.toString().c_str()));
        } else { // не в режиме сопряжения
          sendEventAnyway("@auth-ble-fail@error=not_pairable");
          debugln("BT: error=not_pairable");
        }
      }
    }
  }

  void deletePairedDevice() {
    if (pairedDeviceAddress != nullptr) {
      delete pairedDeviceAddress;
      pairedDeviceAddress = nullptr;
    }
  }

  void disconnectAllClients() {
    if (pServer == nullptr) return;

    uint32_t count = pServer->getConnectedCount();

    for (uint32_t connId = 0; connId < count; connId++) {
      pServer->disconnect(connId);
    }

    debugln("All clients disconnected.");
  }

  BLEUUID setupBluetoothServiceBus() {

    BLEService* pService = pServer->createService(BT_SERVICE_UUID_MAIN);

    // char for recieve commands
    pCommandCharacteristic = pService->createCharacteristic(BT_CHARACTERISTIC_UUID_COMMAND, NIMBLE_PROPERTY::WRITE);
    pCommandCharacteristic->setCallbacks(new BLECallbackCommand(this));

    // char for send events
    pEventCharacteristic = pService->createCharacteristic(BT_CHARACTERISTIC_UUID_EVENT, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    pService->start();

    return pService->getUUID();
  }

  class MyBLEServerCallbacks : public BLEServerCallbacks {
    BluetoothConnection* parent;
  public:
    MyBLEServerCallbacks(BluetoothConnection* parent) : parent(parent) {}

    // CONNECT CLIENT
    void onConnect(BLEServer* pServer, ble_gap_conn_desc* desc) {
      Serial.println("BLE client connected");
    }

    // DISCONNECT CLIENT
    void onDisconnect(BLEServer* pServer, ble_gap_conn_desc* desc) {
      Serial.println("BLE client disconnected");
      // если отключился авторизованный - отмечаем это
      NimBLEAddress deviceAddress(desc->peer_id_addr);
      if (parent->pairedDeviceAddress != nullptr && deviceAddress == *(parent->pairedDeviceAddress)) {
        parent->deletePairedDevice();
      }
    }
  };
};

