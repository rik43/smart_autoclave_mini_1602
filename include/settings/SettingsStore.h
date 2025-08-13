#pragma once

// Загрузка / Сохранение настроек во встроенную память
// https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/src/Preferences.h

#include <Preferences.h>
#include "settings_const.h"
#include "const.h"

// типы переменных менять нежелательно, 
// т.к. переменная созданная с другим типом не прочитается, пока не перезаписать с правильным типом

class SettingsStore {
private:
  Preferences preferences;

  uint16_t displayVersion = 0;

  void checkMigration() {
    checkStore();

    uint32_t versionStorage = getSettingsVersion();

    if (versionStorage == 0) {
      initStore();
    }
    else if (versionStorage < SETTINGS_VERSION) {
      migrateSettings(versionStorage, SETTINGS_VERSION);
    }
    else if (versionStorage > SETTINGS_VERSION) {
      // версия в nvs больше чем текущая прошивки (залили более старую прошивку?)
      prt("too big version in storage", versionStorage);
      prt("current", SETTINGS_VERSION);
    }
  }

  void checkStore() {
    // версия железа из первой прошивки
    if (!preferences.isKey(SETTING_KEY_HARDWARE_VERSION)) {
      saveHardwareVersion();
    }

    // инициализация яркости дисплея
    if (!preferences.isKey(SETTING_KEY_SCREEN_BRIGHTNESS)) {
      setScreenBrightness(100);
    }

    // инициализация заводской калибровки
    if (!preferences.isKey(SETTING_KEY_CALIBRATION_FACTORY)) {
      setFactoryCalibration(FACTORY_TEMPERATURE_CALIBRATION);
    }

    // инициализация заводской калибровки макс мощности
    if (!preferences.isKey(SETTING_KEY_HEATER_MAX_POWER)) {
      setHeaterMaxPower(FACTORY_HEATER_MAX_POWER);
    }

    // инициализация юзерской калибровки
    //if (!preferences.isKey(SETTING_KEY_CALIBRATION_USER)) {
    //  setUserCalibration(0.0);
    //}
  }

  // инициализировать переменные после первого запуска
  void initStore() {
    debugln("init store");
    // текущая версия настроек NVS
    preferences.putUInt(SETTING_KEY_VERSION, SETTINGS_VERSION);
  }

  void saveHardwareVersion() {
    preferences.putUInt(SETTING_KEY_HARDWARE_VERSION, HARDWARE_VERSION_DEFAULT);
  }

  // произвести миграцию настроек от указанной версии до новой
  void migrateSettings(uint32_t currentVersion, uint32_t maxVersion) {
    if (currentVersion <= 3 && currentVersion <= maxVersion) {
      //debugln("migrate 2 to 3");
      //setWifiSsid("Dz");
      //setWifiPassword("Dz13042020");
      //saveSettingsVersion(currentVersion = 3);
    }
    if (currentVersion <= 4 && currentVersion <= maxVersion) {
      //debugln("migrate 3 to 4");
      //setDevicePower(1500);
      //setDeviceVolume(18);
    }
    // migrate other ...

    saveSettingsVersion(currentVersion);
  }

  std::string getString(const char* key) {
    // Проверка существования ключа
    if (!preferences.isKey(key)) {
      prt("Key not found", key);
      return "";
    }

    char str[128]; // буфер под строку для чтения
    size_t readLen = preferences.getString(key, str, sizeof(str));

    if (readLen > 0) {
      // Проверяем, является ли последний символ нулевым символом
      if (str[readLen - 1] == '\0') {
        readLen--; // Уменьшаем readLen, исключая нулевой символ
      }
      // Создаем строку std::string на основе считанных данных
      return std::string(str, readLen);
    }

    return "";
  }

public:

  // Инициализировать модуль настроек и если надо проинициализировать или мигрировать
  SettingsStore() {
  }
  ~SettingsStore() {
    preferences.end();
  }
  SettingsStore(const SettingsStore&) = delete; // copy disable
  SettingsStore& operator=(const SettingsStore&) = delete; // copy disable

  void start() {
    preferences.begin("settings", false);
    checkMigration();
  }

  // CLEAR

  // полная очистка
  void clearAll() {
    preferences.clear();
  }

  // очистить настройки всех соединений
  void clearAllConnections() {
    clearCloud();
    clearWifi();
    clearBle();
  }

  void saveRemove(const char * key) {
    if (preferences.isKey(key)) {
      preferences.remove(key);
    }
  }

  void clearBle() {
    saveRemove(SETTING_KEY_BLE_TOKEN);
  }

  void clearWifi() {
    saveRemove(SETTING_KEY_WIFI_SSID);
    saveRemove(SETTING_KEY_WIFI_PASSWORD);
  }

  void clearCloud() {
    saveRemove(SETTING_KEY_WEBSOCKET_HOST);
    saveRemove(SETTING_KEY_WEBSOCKET_PROTOCOL);
    saveRemove(SETTING_KEY_WEBSOCKET_PORT);
    saveRemove(SETTING_KEY_SERVER_TOKEN);
  }

  // GET/SET

  std::string getProductType() {
    return DESCRIPTOR_TYPE;
  }

  std::string getModel() {
    return DESCRIPTOR_MODEL;
  }

  uint16_t getFirmwareVersion() {
    return FIRMWARE_VERSION;
  }

  uint32_t getHardwareVersion() {
    return preferences.getUInt(SETTING_KEY_HARDWARE_VERSION);
  }

  uint16_t getDisplayVersion() {
    return displayVersion;
  }

  void setDisplayVersion(uint16_t value) {
    displayVersion = value;
  }

  uint32_t getSettingsVersion() {
    return preferences.getUInt(SETTING_KEY_VERSION, 0);
  }

  void saveSettingsVersion(uint32_t ver) {
    preferences.putUInt(SETTING_KEY_VERSION, ver);
  }

  // Статус завершения обновления дисплея. 0 - завершено, 1 - не завершено
  uint8_t getDisplayFirmwareUpdateState() {
    return preferences.getChar(SETTING_KEY_DISPLAY_FIRMWARE_UPDATE_STATE, 0);
  }
  void setDisplayFirmwareUpdateStarted() {
    if (getDisplayFirmwareUpdateState() != 1) {
      preferences.putChar(SETTING_KEY_DISPLAY_FIRMWARE_UPDATE_STATE, 1);
    }
  }
  void setDisplayFirmwareUpdateCompleted() {
    if (getDisplayFirmwareUpdateState() != 0) { // предотвращаем износ флеш памяти
      preferences.putChar(SETTING_KEY_DISPLAY_FIRMWARE_UPDATE_STATE, 0);
    }
  }


  // BLE
  void setBleToken(const char* value) {
    preferences.putString(SETTING_KEY_BLE_TOKEN, value);
  }

  std::string getBleToken() {
    return getString(SETTING_KEY_BLE_TOKEN);
  }


  // WIFI
  void setWifiSsid(const char* value) {
    preferences.putString(SETTING_KEY_WIFI_SSID, value);
  }

  std::string getWifiSsid() {
    return getString(SETTING_KEY_WIFI_SSID);
  }

  void setWifiPassword(const char* value) {
    preferences.putString(SETTING_KEY_WIFI_PASSWORD, value);
  }

  std::string getWifiPassword() {
    return getString(SETTING_KEY_WIFI_PASSWORD);
  }


  // Cloud token
  void setServerToken(const char* value) {
    preferences.putString(SETTING_KEY_SERVER_TOKEN, value);
  }
  std::string getServerToken() {
    return getString(SETTING_KEY_SERVER_TOKEN);
  }

  // Cloud url
  void setWebsocketHost(const char* value) {
    preferences.putString(SETTING_KEY_WEBSOCKET_HOST, value);
  }
  std::string getWebsocketHost() {
    return getString(SETTING_KEY_WEBSOCKET_HOST);
  }

  void setWebsocketProtocol(const char* value) {
    preferences.putString(SETTING_KEY_WEBSOCKET_PROTOCOL, value);
  }
  std::string getWebsocketProtocol() {
    return getString(SETTING_KEY_WEBSOCKET_PROTOCOL);
  }

  void setWebsocketPort(int16_t value) {
    preferences.putUShort(SETTING_KEY_WEBSOCKET_PORT, value);
  }
  int16_t getWebsocketPort() {
    return preferences.getUShort(SETTING_KEY_WEBSOCKET_PORT);
  }


  // Hardware
  // Calibration храним в int32 значение x10 (1 знак после запятой)
  void setUserCalibration(float value) {
    preferences.putInt(SETTING_KEY_CALIBRATION_USER, static_cast<int32_t>(std::round(value * 10.0f)));
  }

  float getUserCalibration() {
    return preferences.getInt(SETTING_KEY_CALIBRATION_USER, 0) / 10.0f;
  }

  void setFactoryCalibration(float value) {
    preferences.putInt(SETTING_KEY_CALIBRATION_FACTORY, static_cast<int32_t>(std::round(value * 10.0f)));
  }

  float getFactoryCalibration() {
    return preferences.getInt(SETTING_KEY_CALIBRATION_FACTORY, 0) / 10.0f;
  }

  float getCalibration() {
    return getFactoryCalibration() + getUserCalibration();
  }


  void setHeaterMaxPower(float value) {
    preferences.putInt(SETTING_KEY_HEATER_MAX_POWER, static_cast<int32_t>(std::round(value * 100.0f)));
  }

  float getHeaterMaxPower() {
    return preferences.getInt(SETTING_KEY_HEATER_MAX_POWER, 100) / 100.0f;
  }



  // яркость дисплея 0-100
  void setScreenBrightness(int16_t value) {
    preferences.putUShort(SETTING_KEY_SCREEN_BRIGHTNESS, value);
  }

  int16_t getScreenBrightness() {
    return preferences.getUShort(SETTING_KEY_SCREEN_BRIGHTNESS);
  }

/*
  // Мощность ТЭН
  void setDevicePower(int16_t value) {
    preferences.putUShort(SETTING_KEY_DEVICE_POWER, value);
  }

  int16_t getDevicePower() {
    return preferences.getUShort(SETTING_KEY_DEVICE_POWER);
  }

  // Объем устройства
  void setDeviceVolume(int16_t value) {
    preferences.putUShort(SETTING_KEY_DEVICE_VOLUME, value);
  }

  int16_t getDeviceVolume() {
    return preferences.getUShort(SETTING_KEY_DEVICE_VOLUME);
  }
*/

};