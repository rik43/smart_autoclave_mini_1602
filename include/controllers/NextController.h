#pragma once
#include <string>
#include <cfloat> // Для FLT_MAX

// Выполняет переход на след блок ( NEXT = 1 )
class NextController : public BaseController {
private:
  Parameter* paramNext = nullptr; // параметр сами не создаем - сами не удаляем

public:
  NextController() : BaseController() {
    type = "Next";
  }

  ~NextController() {
    
  }

  bool setParam(const char* paramName, const char* paramValue, Memory& memory) {
    {
      BaseController::setParam(paramName, paramValue, memory);
    }

    return true;
  }

  bool setup(Memory& memory, VTime& time) {
    paramNext = memory.findParam("NEXT");
    if (paramNext == nullptr) {
      return requiredParam("NEXT");
    }

    return true;
  }

  void update() {
    paramNext->setTrue();
    return;
  }
};
