#pragma once
#include <string>
#include "app/memory.h"

// публикация переменных программы
class EventSoftState : public Event {
public:
    Memory& memory;

    EventSoftState(Memory& memory) : Event("soft-state"), memory(memory) {}

    void toString(std::string& msg) override {
        byte count = memory.getInitedCount();
        for (byte i = 0; i < count; i++) {
            Parameter* param = memory.getParamByIndex(i);
            if ((param != nullptr) && param->isPublished) {
                addParam(msg, param->idString(), param->getString());
            }
        }
    }
};


