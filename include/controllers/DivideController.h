#pragma once
#include <string>
#include <cfloat> // Для FLT_MAX

// Выполняет деление переменных
// Формула расчёта ( set = A / B )
class DivideController : public BaseController {
private:
  Parameter* paramResult = nullptr;   // float
  Parameter* paramA = nullptr;        // float
  Parameter* paramB = nullptr;        // float

public:
  DivideController() : BaseController() {
    type = "Divide";
  }

  ~DivideController() {
    deleteParam(paramResult);
    deleteParam(paramA);
    deleteParam(paramB);
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
    else {
      BaseController::setParam(paramName, paramValue, memory);
    }

    return true;
  }

  bool setup(Memory& memory, VTime& time) {
    if (paramResult == nullptr) {
      requiredParam("set");
    }
    if (paramA == nullptr) {
      requiredParam("a");
    }
    if (paramB == nullptr) {
      requiredParam("b");
    }
    return true;
  }

  void update() {
    float a = paramA->getFloat();
    float b = paramB->getFloat();
    float result;

    if (b != 0.0f) {
      result = a / b;
    }
    else {
      result = FLT_MAX;
    }

    paramResult->setValue(result);

    return;
  }
};
