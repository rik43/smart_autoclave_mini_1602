#pragma once

// Класс таймера. 
// Все параметры времени в милисекундах
// - time:int - накапливается время работы от started
// - countdown:bool - считать время назад (уменьшать переменную)
class TimerController : public BaseController {
  private:
    VTimer* timer;

    Parameter* paramTime = nullptr; // int - мс, прошедшее время
    Parameter* paramCountdown = nullptr; // bool - считать время назад

  public:

    TimerController(): BaseController() {
      type = "Timer";
    }

    ~TimerController() {
      deleteParam(paramTime);
      deleteParam(paramCountdown);

      if (timer != nullptr) {
        delete timer;
        timer = nullptr;
      }
    }

    bool setParam(const char* paramName, const char* paramValue, Memory &memory) {
      if (strcmp(paramName, "time") == 0) {
        paramTime = memory.makeParameter(PARAMETER_TYPE_INT, paramValue);
      }
      else if (strcmp(paramName, "countdown") == 0) {
        paramCountdown = memory.makeParameter(PARAMETER_TYPE_BOOL, paramValue);
      }
      else {
        BaseController::setParam(paramName, paramValue, memory);
      }
      return true;
    }

    bool setup(Memory& memory, VTime& time) {
      if (paramTime == nullptr) {
        return requiredParam("time");
      }

      timer = new VTimer(1, time);

      return true;
    }

    void update() {

      int32_t interval = timer->interval();

      bool isCountdown = (paramCountdown != nullptr) && paramCountdown->isTrue();

      if (isCountdown) {
        paramTime->addInt(-interval);
      } else {
        paramTime->addInt(interval);
      }

      return;
    }
};
