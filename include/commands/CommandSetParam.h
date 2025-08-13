#pragma once
#include "app/app.h"

extern App app;

class CommandSetParam : public Command {
private:

public:

  std::string pName;
  std::string pValue;

  bool setParam(const std::string& paramName, const std::string& paramValue) {
    pName = paramName;
    pValue = paramValue;

    // нестандартное поведение - параметр применяется сразу, а не в invoke
    // для производительности, чтобы не сохранять массив параметров
    apply();

    return true;
  }

  void apply() {
    debug("Set param ");
    debug(pName.c_str());
    debug(" = ");
    debugln(pValue.c_str());

    Parameter* param = app.program.memory.findParam(pName);
    if (param != nullptr) {
      param->setValue(pValue);
    }
  }

  void invoke() override {
    //nothing
  }

};
