#pragma once
#include "commands/command.h"
#include <string>

// сброс всех соединений
class CommandConnectionWipe : public Command {
public:
  // true - внутренний вызов, не выводит на дисплей сообщение "готово"
  bool isInternal = false;

  bool setParam(const std::string& paramName, const std::string& paramValue) {
    Command::setParam(paramName, paramValue);
    return true;
  }

  void invoke() override;
};
