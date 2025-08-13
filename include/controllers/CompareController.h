#pragma once
#include <string>
#include <cfloat> // Для FLT_MAX

// Выполняет сравнение переменных
// Формула расчёта ( set = A > B )
// sign (<, <=, <>, >, >=, =, ==)
class CompareController : public BaseController {
private:

  Parameter* paramResult = nullptr;   // bool
  Parameter* paramA = nullptr;        // float
  Parameter* paramB = nullptr;        // float
  char sign[3] = ">=";

public:
  CompareController() : BaseController() {
    type = "Compare";
  }

  ~CompareController() {
    deleteParam(paramResult);
    deleteParam(paramA);
    deleteParam(paramB);
  }

  bool setParam(const char* paramName, const char* paramValue, Memory& memory) {

    if (strcmp(paramName, "set") == 0) {
      paramResult = memory.makeParameter(PARAMETER_TYPE_BOOL, paramValue);
    }
    else if (strcmp(paramName, "a") == 0) {
      paramA = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "b") == 0) {
      paramB = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
    }
    else if (strcmp(paramName, "sign") == 0) {
      // max 2 char accepted
      sign[0] = paramValue[0];
      sign[1] = paramValue[1];
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
    bool result;
  
    switch (sign[0]) {
    case '<':
      if (sign[1] == '=') {
        result = a <= b;
      }
      else if (sign[1] == '>') {
        result = a != b;
      }
      else {
        result = a < b;
      }
      break;
    case '>':
      if (sign[1] == '=') {
        result = a >= b;
      }
      else {
        result = a > b;
      }
      break;
    case '=':
      if (sign[1] == '=') {
        result = std::abs(a - b) < 0.001;
      }
      else {
        result = a == b;
      }
      break;
    default:
      result = false;
      break;
    }

    paramResult->setValue(result);

    return;
  }
};
