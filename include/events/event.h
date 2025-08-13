#pragma once
#include <string>

class Event {
  private:
    bool hasParams = false;

  public:
    std::string name;
    
    Event(std::string name): name(name) {}

    Event(const Event&) = delete; // copy disable
    Event& operator=(const Event&) = delete; // copy disable
    virtual ~Event() = default;

    virtual void toString(std::string& msg) {}

    std::string getMessage() {
      std::string msg = createMessage();
      toString(msg);
      return msg;
    }

    std::string createMessage() {
      std::string msg = "@" + name + "@";
      return msg;
    }

    void addParam(std::string& message, std::string param, std::string value) {
      if (hasParams) message += "&";
      // clear "=" from param
      // clear "&" from value
      message += param + "=" + value;
      hasParams = true;
    }

};