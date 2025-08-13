#pragma once
#include "commands/command_error.h"

//#include "net/connector.h" // Connector& connector
//class Connector;

class Command {
  public:
    
    Command() = default; // Явно указываем, что конструктор по умолчанию разрешен
    Command(const Command&) = delete; // copy disable
    Command& operator=(const Command&) = delete; // copy disable
    virtual ~Command() = default;

    virtual void invoke() {}
    
    virtual bool setParam(const std::string& paramName, const std::string& paramValue) {
      std::string errorMessage = "unknown parameter prop '" + std::string(paramName) + "' = '" + std::string(paramValue) + "'";
      throw CommandError(errorMessage);

      return true;
    }
};
