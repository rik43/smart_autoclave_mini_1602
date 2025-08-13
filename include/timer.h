#pragma once
#include <Arduino.h>

class Timer {
  public:
    Timer(uint16_t nprd = 0) {
      setPeriod(nprd);
    }
    
    void setPeriod(uint16_t nprd) {
      prd = nprd;
    }

    uint16_t getPeriod() {
      return prd;
    }

    bool ready() {
      if (millis() - tmr >= prd) {
        tmr = millis();
        return true;
      }
      return false;
    }

    void reset() {
      tmr = millis();
    }

  private:
    uint32_t tmr = 0;
    uint16_t prd = 0;
};