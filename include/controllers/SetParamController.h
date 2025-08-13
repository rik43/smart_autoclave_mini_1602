#pragma once
#include <string>

// Выполняет копирования значения переменной
// можно дополнительно (опционально) указать множитель и добавочный параметры.
// Формула расчёта ( set = A * B + C )
// все параметры опциональные: A=1 B=1 C=0
// без ABC: out = 1
// только С: out = C + 1
class SetParamController : public BaseController {
private:

  Parameter* paramResult = nullptr;   // float
  Parameter* paramA = nullptr;        // float
  Parameter* paramB = nullptr;        // float
  Parameter* paramC = nullptr;        // float

public:
  SetParamController() : BaseController() {
    type = "S"; // "SetParam";
  }

  ~SetParamController() {
    deleteParam(paramResult);
    deleteParam(paramA);
    deleteParam(paramB);
    deleteParam(paramC);
  }

  bool setParam(const char* paramName, const char* paramValue, Memory& memory) {

    if (strcmp(paramName, "set") == 0) {
      paramResult = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "a") == 0) {
      paramA = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "b") == 0) {
      paramB = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "c") == 0) {
      paramC = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else {
      BaseController::setParam(paramName, paramValue, memory);
    }

    return true;
  }

  bool setup(Memory& memory, VTime& time) {
    if (paramResult == nullptr) {
      return requiredParam("set");
    }
    return true;
  }

  void update() {
    float value = 1; // A * B + C
    
    if (paramA != nullptr) {
      value *= paramA->getFloat();
    }

    if (paramB != nullptr) {
      value *= paramB->getFloat();
    }

    if (paramC != nullptr) {
      value += paramC->getFloat();
    }

    paramResult->setValue(value);

    return;
  }
};
