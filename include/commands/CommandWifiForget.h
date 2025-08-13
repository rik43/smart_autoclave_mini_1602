#pragma once

class CommandWifiForget : public Command {
public:
  bool setParam(const std::string& paramName, const std::string& paramValue) {
    Command::setParam(paramName, paramValue);
    return true;
  }

  void invoke() override;
};
