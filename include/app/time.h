#pragma once
#include <Arduino.h>

class VTime {
  private:
    unsigned long currentMillis; // текущее виртуальное время
    float timeSpeed;             // скорость проистечения времени

  public:
    VTime(float speed = 1.0) : currentMillis(0), timeSpeed(speed) {}

    VTime(const VTime&) = delete; // copy disable
    VTime& operator=(const VTime&) = delete; // copy disable

    unsigned long millis() {
      unsigned long realMillis = ::millis(); // получаем текущее реальное время
      unsigned long elapsedRealMillis = realMillis - currentMillis; // вычисляем прошедшее реальное время

      // обновляем виртуальное время с учетом скорости проистечения времени
      currentMillis += static_cast<unsigned long>(elapsedRealMillis * timeSpeed);

      return currentMillis;
    }

    void setTimeSpeed(float speed) {
      timeSpeed = speed;
    }

    float getTimeSpeed() {
      return timeSpeed;
    }

};


