#pragma once
#include "commands/command.h"
#include <string>

class CommandPairing : public Command {
public:
  bool active = true; // активировать или деактивировать

  bool setParam(const std::string& paramName, const std::string& paramValue) {
    if (paramName == "active") {
      active = std::stoi(paramValue);
    }
    else {
      Command::setParam(paramName, paramValue);
    }
    return true;
  }

  void invoke() override;
};

