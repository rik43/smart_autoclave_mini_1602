#pragma once
#include <string>
#include <array>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "net/cert-le.h"
#include "events/EventUpdateStatus.h"
#include "display/DwinFirmwareUpdate.h"
#include "command.h"


// Переиспользуем одно событие, каждый раз обновляем и отправляем его
static EventUpdateStatus eventUpdate;
static unsigned long lastUpdateTime = 0; // время последней отправки события

extern SettingsStore settings;


// callback update esp32 firmware
static void updateProgress(int current, int total) {
  byte progress = total > 0 ? current * 100 / total : 100;
  
  unsigned long currentTime = millis();

  bool canSendUpdate = (progress == 100) || // всегда отправляем при 100%
                       (progress != eventUpdate.progress && 
                       (currentTime - lastUpdateTime >= 3000 || lastUpdateTime == 0)); // не чаще раз в 3 сек
  
  if (canSendUpdate) {
    eventUpdate.progress = progress;
    emit(eventUpdate);
    lastUpdateTime = currentTime;
  }
}


// Обновляет прошивку ESP и дисплея (либо одно из них)
class CommandUpdateFirmware : public Command {
private:
  int totalFilesCount = 0;
  
  // support both https and http urls
  std::string firmwareUrl; // ESP32

  // support both https and http urls
  std::array<std::string, 32> urls; // DWIN bin files

  int dwinFilesCount = 0;

public:
  bool setParam(const std::string& paramName, const std::string& paramValue) {
    if (paramName == "firmware") { // esp32
      firmwareUrl = paramValue;
      ++totalFilesCount;
    }
    else if (paramName.rfind("file", 0) == 0) { // Проверка, начинается ли paramName с "file"
      // DWIN: параметры могут быть file0, file1, file2, ... file31.
      try {
        int index = std::stoi(paramName.substr(4)); // Извлекаем индекс из paramName
        if (index >= 0 && index < urls.size()) {
            urls[index] = paramValue;
            ++totalFilesCount;
            ++dwinFilesCount;
            return true;
        }
      } catch (const std::invalid_argument& e) {
        // Обработка ошибки, если paramName не содержит допустимого числа
        prt("wrong number in", paramName.c_str());
      }
    }
    else {
      Command::setParam(paramName, paramValue);
    }
    return true;
  }

  // void updateStackSize() {
  //   prt("free heap", ESP.getFreeHeap());
  //   prt("stack high water mark", uxTaskGetStackHighWaterMark(NULL));
  //   // stack size of the task
  //   uint8_t* stackStart = pxTaskGetStackStart(NULL);
  //   uint8_t* stackEnd;
  //   asm volatile("mov %0, sp" : "=r" (stackEnd));
  //   prt("stack size", (uint32_t)stackEnd - (uint32_t)stackStart);
  // }

  void invoke() override {
    if (!hasWifiConnection()) return;

    //updateStackSize();

    eventUpdate.errorCode = 0;
    eventUpdate.progress = 0;
    eventUpdate.fileNum = 0;
    eventUpdate.filesCount = totalFilesCount;

    bool isUpdated = false;
    bool isEspFlashOk = true;
    bool isDwinFlashOk = true;

    // ESP32
    if (!firmwareUrl.empty()) {
      eventUpdate.startNextFile();
      eventUpdate.isEsp = true;
      isEspFlashOk = handleUpdateEsp();
      isUpdated = isEspFlashOk;
    }

    // во время обновления прошивки ESP стек достигает минимального значения 400b
    //updateStackSize(); return;

    // DWIN
    if (isEspFlashOk && dwinFilesCount) {
      isDwinFlashOk = handleUpdateDwin();
      isUpdated = isUpdated || isDwinFlashOk;
    }

    // отправить успех и перезагрузиться
    if (isUpdated) {
      updateEnded(isEspFlashOk && isDwinFlashOk);
    }
  }

  bool handleUpdateEsp() {
    // httpUpdate создан глобально в cpp файле библ
    httpUpdate.rebootOnUpdate(false);
    httpUpdate.onProgress(updateProgress);

    const int TIMEOUT = 120000; // 120 секунд

    WiFiClientSecure httpsClient;
    WiFiClient httpClient;

    t_httpUpdate_return ret;
    if (firmwareUrl.substr(0, 5) == "https") {
        httpsClient.setCACert(rootCACertificate);
        httpsClient.setTimeout(TIMEOUT);
        ret = httpUpdate.update(httpsClient, firmwareUrl.c_str());
    } else {
        httpClient.setTimeout(TIMEOUT);
        ret = httpUpdate.update(httpClient, firmwareUrl.c_str());
    }

    // WiFiClientSecure client;
    // client.setCACert(rootCACertificate);
    // client.setTimeout(12000);
    // t_httpUpdate_return ret = httpUpdate.update(client, firmwareUrl.c_str());

    switch (ret) {
      case HTTP_UPDATE_FAILED: {
          eventUpdate.errorCode = httpUpdate.getLastError();
          emit(eventUpdate);
          // getLastError: ошибки http client + 9 от update:
          // https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPUpdate/src/HTTPUpdate.h#L35
          // https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPUpdate/src/HTTPUpdate.cpp#L115
          Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        }
        break;

      case HTTP_UPDATE_NO_UPDATES: {
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          eventUpdate.errorCode = -1000;
          emit(eventUpdate);
        }
        break;

      case HTTP_UPDATE_OK: {
          Serial.println("HTTP_UPDATE_OK");
          return true;
        }
        break;
    }

    return false;
  }

  bool handleUpdateDwin() { 
    for (size_t i = 0; i < urls.size(); ++i) {
      if (!urls[i].empty()) {

        settings.setDisplayFirmwareUpdateStarted();

        bool fileUpdateSuccess = false;
        int retryCount = 0;
        const int MAX_RETRIES = 1000;  // максимальное количество попыток
        const int RETRY_DELAY = 8000;  // задержка между попытками (8 секунд)

        eventUpdate.startNextFile();
        eventUpdate.isEsp = false;

        while (!fileUpdateSuccess && retryCount < MAX_RETRIES) {
          if (!hasWifiConnection()) {
            debugln("Waiting for WiFi connection...");
            vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY));
            continue;
          }

          prt("file", i);
          prt("url", urls[i].c_str());
          prt("attempt", retryCount + 1);

          DwinFirmwareUpdate updater(Serial2, i, urls[i]);
          updater.setUpdateCallback(updateProgress);
          
          int httpErrorCode = updater.updateBlock();

          if (httpErrorCode == 0) {
            fileUpdateSuccess = true;
            eventUpdate.errorCode = 0;
          } else {
            retryCount++;
            // отправляем ошибку, но если проблемы с подключением - не отправится
            eventUpdate.errorCode = httpErrorCode;
            emit(eventUpdate);
            
            if (retryCount < MAX_RETRIES) {
              prt("Error updating. Retrying in", RETRY_DELAY/1000);
              vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY));
            }
          }
        }

        if (!fileUpdateSuccess) {
          debugln("Exceeded maximum number of attempts to update");
          return false;
        }
      }
    }

    settings.setDisplayFirmwareUpdateCompleted();

    return true;
  }

  bool hasWifiConnection() {
    wl_status_t status = WiFi.status();
    
    // Проверяем не только статус подключения, но и наличие валидного IP
    if (status == WL_CONNECTED && WiFi.localIP() != INADDR_NONE) {
        return true;
    }

    // Если в процессе подключения - ждем результата
    if (status == WL_IDLE_STATUS) {
        const int CONNECT_TIMEOUT = 60000; // 60 секунд
        unsigned long startTime = millis();
        
        while (millis() - startTime < CONNECT_TIMEOUT) {
            status = WiFi.status();
            if (status == WL_CONNECTED && WiFi.localIP() != INADDR_NONE) {
                return true;
            }
            if (status != WL_IDLE_STATUS) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    // Если дошли сюда - надо переподключаться
    debugln("Reconnecting to WiFi...");
    
    // Отключаемся если не в состоянии DISCONNECTED
    if (status != WL_DISCONNECTED) {
        WiFi.disconnect(true, false);  // disconnect but keep config
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Проверяем есть ли сохраненные креды
    if (settings.getWifiSsid().length() == 0) {
        debugln("No saved WiFi settings");
        return false;
    }

    // Пробуем подключиться используя сохраненные настройки
    WiFi.mode(WIFI_STA);
    WiFi.begin(settings.getWifiSsid().c_str(), settings.getWifiPassword().c_str());  // использует сохраненные SSID/password

    // Ждем подключения и получения IP
    const int RECONNECT_TIMEOUT = 60000; // 60 секунд
    unsigned long startTime = millis();
    
    while (millis() - startTime < RECONNECT_TIMEOUT) {
        status = WiFi.status();
        if (status == WL_CONNECTED && WiFi.localIP() != INADDR_NONE) {
            debugln("WiFi connected with IP");
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    debugln("Failed to connect to WiFi or get IP");
    return false;
  }

  void updateEnded(bool isSuccess) {
    if (isSuccess) { // продублируем 100% на всякий
      eventUpdate.progress = 100;
      emit(eventUpdate);
    }

    // надо ли отправлять? если отправлять надо еще считать версию дисплея. 
    // либо отправлять после перезагрузки
    //EventDescriptor evtDesc;
    //emit(evtDesc);

    // Небольшая задержка перед перезагрузкой для отправки сообщений
    vTaskDelay(pdMS_TO_TICKS(800));
    //delay(800);

    ESP.restart(); // Перезагрузка
  }

};

