#pragma once
#include <string>
#include "utils.h"
#include "events/event.h"

// публикация переменных состояния железа
class EventHardState : public Event {
public:
    float temperature = 0;
    float temperatureRaw = 0;
    float pwm = 0;
    bool isValidTemperature = 0;
    int8_t wifiRssi = 0;
    //uint32_t restTimeSeconds = 600;
    //uint32_t programStep = 0;
    //uint32_t programStepCount = 0;

    EventHardState() : Event("hard-state") {}

    void toString(std::string& msg) override {
        addParam(msg, "tmp", float2string(temperature, 2));
        addParam(msg, "pwm", float2string(pwm, 3));

        addParam(msg, "tmp_ok", isValidTemperature ? "1" : "0");
        //addParam(msg, "tmp_raw", std::to_string(temperatureRaw));

        addParam(msg, "heap", std::to_string(ESP.getFreeHeap()));
        addParam(msg, "rssi", std::to_string(wifiRssi));
    }
};


