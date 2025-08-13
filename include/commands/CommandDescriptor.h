#pragma once
#include <string>
#include "command.h"
#include "events/EventDescriptor.h"

class CommandDescriptor : public Command {
private:
  
public:
  bool setParam(const std::string& paramName, const std::string& paramValue) {
    //if (paramName == "token") {
    //  token = paramValue;
    //}
    //else {
    //  Command::setParam(paramName, paramValue);
    //}
    return true;
  }

  void invoke() override {
    EventDescriptor evt;
    emit(evt);
  }
};

