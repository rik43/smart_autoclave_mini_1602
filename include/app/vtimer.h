#pragma once

class VTimer {
  public:
    VTimer(uint16_t nprd, VTime& time): prd(nprd), time(time) {}

    VTimer(const VTimer&) = delete; // copy disable
    VTimer& operator=(const VTimer&) = delete; // copy disable

    void setPeriod(uint16_t nprd) {
      prd = nprd;
    }

    uint16_t getPeriod() {
      return prd;
    }

    bool ready() {
      unsigned long ms = time.millis();
      if (ms - tmr >= prd) {
        tmr = ms;
        return true;
      }
      return false;
    }

    // возвращает время прошедшее с предыдущего вызова interval()
    uint32_t interval() {
      unsigned long ms = time.millis();
      uint32_t result;
      if (ms >= tmr2) {
        if (tmr2 == 0) tmr2 = ms; // при первом запуске вернуть 0
        result = ms - tmr2;
      } else {
        result = 0; // сброс времени при переполнении int
      }
      tmr2 = ms;
      return result;
    }

    // время в пределах цикла prd
    uint32_t periodTime() {
      unsigned long ms = time.millis();
      unsigned long delay = ms - tmr;
      return prd ? delay % prd : delay;
    }

    // фаза текущего периода 0.0-1.0
    float getPhase() {
      return periodTime() / prd;
    }

    void reset() {
      tmr = time.millis();
      tmr2 = 0;
      prd = 0;
    }

  private:
    VTime& time;
    uint32_t tmr = 0;
    uint32_t tmr2 = 0;
    uint16_t prd = 0;
};