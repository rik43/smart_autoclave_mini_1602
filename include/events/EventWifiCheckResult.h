#pragma once
#include <string>
#include "settings/SettingsStore.h"

// результат проверки Wi-Fi соединения
class EventWifiCheckResult : public Event {
public:
    int errorCode = 0;

    EventWifiCheckResult() : Event("wifi-check-result") {}

    void toString(std::string& msg) override {
        addParam(msg, "error", std::to_string(errorCode));
    }

};


