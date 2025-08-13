#pragma once
#include <string>

class EventAuthBleOk : public Event {
public:
    EventAuthBleOk() : Event("auth-ble-ok") {}

    void toString(std::string& msg) override {

    }
};


