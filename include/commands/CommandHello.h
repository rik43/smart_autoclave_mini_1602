#pragma once
#include "commands/command.h"
#include "events/EventAuthBleOk.h"
#include <string>

class CommandHello : public Command {
public:
  bool setParam(const std::string& paramName, const std::string& paramValue) {
    return true;
  }

  void invoke() override {
    EventAuthBleOk event;
    emit(event);
  }
};

