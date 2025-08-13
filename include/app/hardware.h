#pragma once

#include "const.h"
#include "global.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <GyverMAX6675.h>
#include <PWMrelay.h>
#include "app/parameter.h"
#include "app/memory.h"
#include "app/time.h"
#include "app/vtimer.h"
#include "timer.h"
#include "events/EventHardState.h"
#include <WiFi.h>
// термопара
static GyverMAX6675<CLK_PIN, DATA_PIN, CS_PIN> sens;

extern PWMrelay heaterRelay;
extern SemaphoreHandle_t pwmHeaterMutex;

void tickHeaterTask(void *parameter);


class Hardware {

private:
  VTime& time;
  Timer timerRead;

  Parameter* paramPwm = nullptr; // ШИМ на тэн
  Parameter* paramTemp = nullptr; // темп-ра с датчика

public:
  float temperatureRaw = 0.0f; // сырое значение с датчика
  float temperature = 0.0f;    // после фильтров - сглаженное значение
  float temperatureSmoothKf = 0.9f; // 90% старого / 10% нового
  float temperatureCalibration = 0.0f; // калибровка темп-ры (загружается из NVS в App)
  float heaterPowerKf = 1.0f; // ограничение макс мощности 0-1 от номинальной (загружается из NVS в App)
  
  bool isValidTemperature = false; // если много ошибок - выставляем сюда ошибку
  bool isValidTemperatureLastRead = false; // один предыдущий раз
  uint8_t notValidSequentalReadingsCount = 0; // кол-во последовательных ошибок измерения
  
  int pwmValue = 0; // 0-255
  float pwmRatio = 0.0f; // 0-1 // то что на каждом шаге приходит из программы
  float pwmMax = 255.0f; // 0-255 // максимальное значение ШИМ (с учетом заводского ограничения)

  Hardware(VTime& time) : time(time) {
    timerRead.setPeriod(300);

    pwmHeaterMutex = xSemaphoreCreateMutex();  // Создаем мьютекс
    xTaskCreatePinnedToCore(tickHeaterTask, "tickHeaterTask", 1024, NULL, 1, NULL, 0);
  }

  Hardware(const Hardware&) = delete; // copy disable
  Hardware& operator=(const Hardware&) = delete; // copy disable

  // получить и сохранить нужные параметры
  void bindMemoryParameters(Memory& memory) {
    paramPwm = memory.makeParameter(PARAMETER_TYPE_FLOAT, "PWM1", "");
    paramTemp = memory.makeParameter(PARAMETER_TYPE_FLOAT, "TMP1", "");
  }

  // очистка памяти (удалить анонимные параметры)
  void unbindMemoryParameters() {
    if (paramPwm != nullptr && paramPwm->isAnonymous) {
      delete paramPwm;
    }
    if (paramTemp != nullptr && paramTemp->isAnonymous) {
      delete paramTemp;
    }
    paramPwm = nullptr;
    paramTemp = nullptr;
  }

  // полностью остановить работу исполнительных устройств
  void stop() {
    pwmValue = 0;
    pwmRatio = 0;
    updateHeaterPWM(0);
    digitalWrite(PIN_RELAY, 0); // OFF
  }

  // чтение всех сенсоров
  void readSensorData() {
    updateTemperature();
    validateTemperature();
  }

  void updateTemperature() {
    if (sens.readTemp()) {
      temperatureRaw = sens.getTemp();
      //prt("temp", temperature);
      isValidTemperatureLastRead = (temperatureRaw <= MAX_ALLOWED_TEMPERATURE) && (temperatureRaw >= MIN_ALLOWED_TEMPERATURE);
    
      temperatureRaw += temperatureCalibration;
      if (std::abs(temperatureRaw - temperature) < 1.0f) {
        temperature = temperature * temperatureSmoothKf + temperatureRaw * (1 - temperatureSmoothKf);
      } else {
        temperature = temperatureRaw;
      }

      if (paramTemp != nullptr) {
        paramTemp->setValue(temperature);
      }

      //if (isValidTemperatureLastRead) {
      //}
      //else {
      //  // температура вышла за допустимый предел
      //  prt("temp not valid", temperatureRaw);
      //}
    }
    else {
      isValidTemperatureLastRead = false;
      debugln("temp not read!"); // отключен модуль или термопара
    }
  }

  void validateTemperature() {
    if (isValidTemperatureLastRead) {
      // если последнее ок, то сбрасываем ошибку и счётчик ошибок
      notValidSequentalReadingsCount = 0;
      isValidTemperature = true;
    } else {
      // если не ок, то считаем ошибки 
      // и по превышению кол-ва выставляем глобальный статус ошибки
      notValidSequentalReadingsCount++;
      if (notValidSequentalReadingsCount > MAX_SEQUENTAL_ERRORS_TO_FAIL) {
        isValidTemperature = false;
      }
    }
  }

  // применить текущее состояние к железу
  void applyState() {
    applyRelay();
    //emulationUpdate();
  }

  void applyRelay() {
    if (paramPwm != nullptr) {
      pwmRatio = paramPwm->getFloat();
      pwmValue = pwmRatio * pwmMax;
      //analogWrite(PIN_RELAY, pwmValue);
      //pwmValue = pwmRatio * 0.75 * 255.0;
      updateHeaterPWM(pwmValue);
    }
    else {
      updateHeaterPWM(0);
      //digitalWrite(PIN_RELAY, 0); // OFF
    }
  }

  void updateHeaterPWM(byte duty) {
      if (xSemaphoreTake(pwmHeaterMutex, portMAX_DELAY)) { // Получаем доступ к ресурсу
          heaterRelay.setPWM(duty);
          xSemaphoreGive(pwmHeaterMutex); // Освобождаем ресурс
      }
  }

  void setHeaterMaxPower(float value) {
    pwmMax = 255.0f * value;
    if (pwmMax >= 253) pwmMax = 255.0f;
    if (pwmMax <= 20) pwmMax = 20.0f;
  }

  void setTemperatureCalibration(float value) {
    if (std::abs(value) < 5.0f) {
      temperatureCalibration = value;
    }
  }

  // получить RSSI сети, 0 - если нет сети
  int8_t getWifiRssi() {
    return WiFi.RSSI();
  }

  // отправить событие HardState
  void publishState() {
    EventHardState state;
    state.temperature = temperature;
    state.temperatureRaw = temperatureRaw;
    state.pwm = pwmRatio;
    state.isValidTemperature = isValidTemperature;
    state.wifiRssi = getWifiRssi();
    emit(state);
  }
};
