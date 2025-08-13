#pragma once
#include <string>
#include "settings/SettingsStore.h"

extern SettingsStore settings;

// статус OTA обновления
class EventUpdateStatus : public Event {
public:
    bool isEsp = true; // is updating esp
    byte fileNum = 0; // current number
    byte filesCount = 1; // total count
    byte progress; // 0-100
    int errorCode = 0;

    //bool isComplete = false;

    EventUpdateStatus() : Event("update-status") {}
    //EventUpdateStatus(byte progress) : progress(progress), Event("update-status") {}

    void toString(std::string& msg) override {
        addParam(msg, "e", std::to_string(isEsp));
        addParam(msg, "fi", std::to_string(fileNum));
        addParam(msg, "fn", std::to_string(filesCount));
        addParam(msg, "progress", std::to_string(progress));
        addParam(msg, "err", std::to_string(errorCode));

        //addParam(msg, "complete", std::to_string(isComplete));
    }

    void startNextFile() {
        ++fileNum;
        progress = 0;
    }

};


