#pragma once
#include "events/events_prog.h"
#include "app/app.h"
#include "commands/command.h"

extern App app;

class CommandProgStop : public Command {
private:

public:
  bool setParam(const std::string& paramName, const std::string& paramValue) {
    //if (paramName == "byUser") {
    //  //prt("byUser", paramValue.c_str()); // test
    //}
    Command::setParam(paramName, paramValue);
    return true;
  }

  void invoke() override {
    app.programStop(true);
  }

};
