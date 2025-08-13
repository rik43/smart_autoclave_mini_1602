#pragma once
#include <string>
#include "settings/SettingsStore.h"

// отображение на дисплее
class EventDisplay : public Event {
public:
    int code = 0;

    EventDisplay(int code) : Event("display") {
        this->code = code;
    }

    void toString(std::string& msg) override {
        addParam(msg, "code", std::to_string(code));
    }
};
