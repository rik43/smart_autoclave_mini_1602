#pragma once
#include "app/time.h"
#include "app/memory.h"
#include "compiler/compilation_error.h"
#include "controllers/BaseController.h"

class ProgramBlock {
  private: 
    VTime& time;
    Memory& memory;

  public:
    uint16_t initedCount = 0;
    uint16_t controllerCount = 0;
    BaseController** controllers;
    bool isWasExecuted = false;

    ProgramBlock(Memory& memory, VTime& time): time(time), memory(memory), controllers(nullptr) {}

    ProgramBlock(const ProgramBlock&) = delete; // copy disable
    ProgramBlock& operator=(const ProgramBlock&) = delete; // copy disable
    ~ProgramBlock() {
      cleanup();
    }

    void init(uint16_t count) {
      cleanup(); // clear previous before
      controllers = new BaseController*[count]; // Выделение памяти под массив указателей
      controllerCount = count;
    }

    void addController(BaseController* ctrl) {
      if (initedCount >= controllerCount) {
        throw CompilationError("exceed controllers count in block");
      }
      bool isValid = ctrl->setup(memory, time);
      controllers[initedCount] = ctrl;
      initedCount++;
    }

    void cleanup() {
      if (controllers != nullptr) {
        for (uint16_t i = 0; i < initedCount; ++i) {
          delete controllers[i];
        }
        delete[] controllers;
        controllers = nullptr;
      }
      controllerCount = 0;
      initedCount = 0;
    }

    // сбросить блок и контроллеры перед началом выполнения блока
    void reset() {
      if (controllers != nullptr) {
        for (uint16_t i = 0; i < initedCount; ++i) {
          BaseController* ctrl = controllers[i];
          ctrl->reset();
        }
      }
      isWasExecuted = false;
    }
};



