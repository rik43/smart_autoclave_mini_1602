#pragma once

#include "MyPID.h"
//#include "GyverPID.h"

// Класс контроллера PID регуляции температуры
// Параметры:
// - input:float - входное значение с датчика
// - target:float - целевое значение, которое поддерживаем
// - output:float - сила выхода на управляющее устройство 0-1
class PIDController : public BaseController {
private:
  Parameter* paramInput = nullptr; // float - входное значение с датчика
  Parameter* paramTarget = nullptr; // float - целевое значение (к которому должны стремиться)
  Parameter* paramOutput = nullptr; // float - выходное значение на управляющее устройство

  Parameter* paramKp = nullptr; // float - PID koeff P
  Parameter* paramKi = nullptr; // float - PID koeff I
  Parameter* paramKd = nullptr; // float - PID koeff D

  Parameter* paramPvalue = nullptr; // float - PID P value
  Parameter* paramIvalue = nullptr; // float - PID I value
  Parameter* paramDvalue = nullptr; // float - PID D value

  Parameter* paramIlimitMin = nullptr; // float - PID I value limit min
  Parameter* paramIlimitMax = nullptr; // float - PID I value limit max

  MyPID regulator;
  //GyverPID regulator;

  VTimer* timer;

public:
  PIDController() : BaseController() {
    type = "PID";
  }

  ~PIDController() {
    deleteParam(paramInput);
    deleteParam(paramTarget);
    deleteParam(paramOutput);

    deleteParam(paramKp);
    deleteParam(paramKi);
    deleteParam(paramKd);

    deleteParam(paramPvalue);
    deleteParam(paramIvalue);
    deleteParam(paramDvalue);

    deleteParam(paramIlimitMin);
    deleteParam(paramIlimitMax);

    if (timer != nullptr) {
      delete timer;
      timer = nullptr;
    }
  }

  bool setParam(const char* paramName, const char* paramValue, Memory& memory) {
    if (strcmp(paramName, "input") == 0) {
      paramInput = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "target") == 0) {
      paramTarget = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "output") == 0) {
      paramOutput = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "kp") == 0) {
      paramKp = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "ki") == 0) {
      paramKi = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "kd") == 0) {
      paramKd = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "value_p") == 0) {
      paramPvalue = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "value_i") == 0) {
      paramIvalue = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "value_d") == 0) {
      paramDvalue = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "i_limit_min") == 0) {
      paramIlimitMin = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "i_limit_max") == 0) {
      paramIlimitMax = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else {
      BaseController::setParam(paramName, paramValue, memory);
    }
    return true;
  }

  bool setup(Memory& memory, VTime& time) {
    // https://alexgyver.ru/gyverpid/
    // default PID parameters
    regulator.Kp = 0.1; // 25.66;
    regulator.Ki = 0.001; // 34.15;
    regulator.Kd = 0.1; // 4.82;
    //regulator.setDt(100);
    //regulator.setDirection(NORMAL); // включает нагрузку для нагрева
    //regulator.setLimits(0, 1000);

    if (paramInput == nullptr) {
      return requiredParam("input");
    }
    if (paramTarget == nullptr) {
      return requiredParam("target");
    }
    if (paramOutput == nullptr) {
      return requiredParam("output");
    }

    timer = new VTimer(1000, time);

    return true;
  }

  void before() {
    if (paramIvalue != nullptr) {
      prt("param I", paramIvalue->getFloat());
      regulator.setValueI(paramIvalue->getFloat());
    }
  }

  void update() {
    // get PID parameters from params
    if (paramKp != nullptr) {
      regulator.Kp = paramKp->getFloat();
    }
    if (paramKi != nullptr) {
      regulator.Ki = paramKi->getFloat();
    }
    if (paramKd != nullptr) {
      regulator.Kd = paramKd->getFloat();
    }

    if (paramIlimitMin != nullptr) {
      regulator.limitImin = paramIlimitMin->getFloat();
    }
    if (paramIlimitMax != nullptr) {
      regulator.limitImax = paramIlimitMax->getFloat();
    }

    //int32_t interval = timer->interval();
    //if (interval > 1000) interval = 1000;

    if (timer->ready()) {
      regulator.setpoint = paramTarget->getFloat();
      regulator.input = paramInput->getFloat();
      //regulator.setDt(1000);
      //int outputPwm = regulator.getResultTimer();
      //float output = (float)outputPwm / 1000;
      float output = regulator.update();

      paramOutput->setValue(output);

      if (paramPvalue != nullptr) {
        paramPvalue->setValue(regulator.valueP);
      }
      if (paramIvalue != nullptr) {
        paramIvalue->setValue(regulator.valueI);
      }
      if (paramDvalue != nullptr) {
        paramDvalue->setValue(regulator.valueD);
      }
    }
    return;
  }

};
