#pragma once
#include <string>
#include "command.h"

class CommandWifiProvisioning : public Command {
private:
  std::string ssid;
  std::string password;
  std::string token;
  std::string host; // можно отправлять только host (можно IP)
  std::string protocol = "wss";
  int port = 443;

public:
  bool setParam(const std::string& paramName, const std::string& paramValue) {
    if (paramName == "ssid") {
      ssid = paramValue;
    }
    else if (paramName == "password") {
      password = paramValue;
    }
    else if (paramName == "ws_host") {
      host = paramValue;
    }
    else if (paramName == "ws_protocol") {
      protocol = paramValue;
    }
    else if (paramName == "ws_port") {
      port = std::stoi(paramValue);
    }
    else if (paramName == "token") {
      token = paramValue;
    }
    else {
      Command::setParam(paramName, paramValue);
    }
    return true;
  }

  void invoke() override;
};

