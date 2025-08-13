#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include "global.h"
#include "app/time.h"
#include "app/vtimer.h"
#include "app/memory.h"
#include "app/block.h"
#include "compiler/compilation_error.h"
#include "controllers/controllers.h"
#include "controllers/controller_factory.h"


class Program {
private:
  bool _debug = false; // больше отладочной информации в порт

public:
  VTime& time;
  Memory& memory;

  VTimer* mainTimer;
  VTimer* blockTimer;

  // id управляющего устройства откуда загружена программа
  // во время загрузки каждая строка программы должна быть из одного источника
  //int sourceId = 0; 
  bool isLoaded = false; // программа загружена полностью
  bool isRunning = false; // программа запущена (выполняем update)
  bool isStopped = false; // программа остановлена (аварийно)
  bool isSuccess = false; // программа завершена (успешно)

  uint16_t currentStep = 0;
  uint16_t initedCount = 0;
  uint16_t blockCount = 0;
  ProgramBlock** blocks;

  uint32_t pid; // init process id (актуально только при загрузке программы)
  uint32_t recipeId; // id рецепта из книги рецептов
  std::string desc; // описание программы для моб приложения (json)
  std::string recipeUuid;

  int16_t maxTemperature = 0;

  Parameter* paramNext = nullptr;             // NEXT:bool - 1-перейти на след блок, или конец
  Parameter* paramStop = nullptr;             // STOP:bool - 1-аварийная остановка
  Parameter* paramGoto = nullptr;             // GOTO:int  - если не "-1" - перейти на блок N
  Parameter* paramDisplayScreen = nullptr;    // DSPL:int - номер экрана на дисплее
  Parameter* paramStepCurrent = nullptr;      // STEP:int - текущий этап (номер)
  Parameter* paramStepCount = nullptr;        // CSTP:int - всего этапов (кол-во)
  Parameter* paramStepType = nullptr;         // TYPE:bool - тип шага (нагрев/готовка/охлад/мощн)
  Parameter* paramTimeType = nullptr;         // TTYP:bool - тип времени: 0-прошло 1-осталось
  Parameter* paramTimeLeft = nullptr;         // TIME:int - мсек - время осталось по этапу
  Parameter* paramTimePassed = nullptr;       // TPSS:int - мсек - время прошло по этапу
  Parameter* paramTimePassedTotal = nullptr;  // TTTL:int - общее время готовки (переместить в HARD?)
  Parameter* paramTimeScale = nullptr;        // TSCL:float - скорость времени
  Parameter* paramMaxTemp = nullptr;          // MAXT:float - макс. достигнутая темп-ра

  Program(Memory& memory, VTime& time) : time(time), memory(memory) {
    mainTimer = new VTimer(0, time);
    blockTimer = new VTimer(0, time);
  }

  Program(const Program&) = delete; // copy disable
  Program& operator=(const Program&) = delete; // copy disable
  ~Program() {
    cleanup();
    if (mainTimer != nullptr) {
      delete mainTimer;
      mainTimer = nullptr;
    }
    if (blockTimer != nullptr) {
      delete blockTimer;
      blockTimer = nullptr;
    }
  }

  void cleanupBlocks() {
    if (blocks != nullptr) {
      for (uint16_t i = 0; i < initedCount; ++i) {
        delete blocks[i];
      }
      delete[] blocks;
      blocks = nullptr;
    }
    blockCount = 0;
    initedCount = 0;
  }

  void cleanup() {
    cleanupBlocks();
    isLoaded = false;
    isRunning = false;
    
    unbindMemoryParameters();

    memory.cleanup();
  }

  void reset() {
    pid = 0;
    recipeId = 0;
    recipeUuid = "";
    desc = "";
    isStopped = false;
    isSuccess = false;

  }

  // перевести программу в не активный режим и удаляет программу
  void stop() {
    cleanup();
  }

  void bindMemoryParameters() {
    paramNext = memory.makeParameter(PARAMETER_TYPE_BOOL, "NEXT", "0");
    paramStop = memory.makeParameter(PARAMETER_TYPE_BOOL, "STOP", "0");
    paramGoto = memory.makeParameter(PARAMETER_TYPE_INT, "GOTO", "0");
    paramDisplayScreen = memory.makeParameter(PARAMETER_TYPE_INT, "DSPL", "0");
    paramStepCurrent = memory.makeParameter(PARAMETER_TYPE_INT, "STEP", "0");
    paramStepCount = memory.makeParameter(PARAMETER_TYPE_INT, "CSTP", "0");
    paramStepType = memory.makeParameter(PARAMETER_TYPE_INT, "TYPE", "0");
    paramTimeType = memory.makeParameter(PARAMETER_TYPE_BOOL, "TTYP", "0");
    paramTimeLeft = memory.makeParameter(PARAMETER_TYPE_INT, "TIME", "0");
    paramTimePassed = memory.makeParameter(PARAMETER_TYPE_INT, "TPSS", "0");
    paramTimePassedTotal = memory.makeParameter(PARAMETER_TYPE_INT, "TTTL", "0");
    paramTimeScale = memory.makeParameter(PARAMETER_TYPE_FLOAT, "TSCL", "0");
    paramMaxTemp = memory.makeParameter(PARAMETER_TYPE_FLOAT, "MAXT", "0");
    setNewProgramParameters();
  }

  void setNewProgramParameters() {
    paramNext->setValue(0);
    paramStop->setValue(0);
    paramGoto->setValue(-1);
    paramDisplayScreen->setValue(1);
    paramStepCurrent->setValue(1);
    paramStepCount->setValue(1);
    paramStepType->setValue(0);
    paramTimeType->setValue(0);
    paramTimeLeft->setValue(0);
    paramTimePassed->setValue(0);
    paramTimePassedTotal->setValue(0);
    paramTimeScale->setValue(1);
    paramMaxTemp->setValue(0);
  }

  void unbindMemoryParameters() {
    deleteParam(paramNext);
    deleteParam(paramStop);
    deleteParam(paramGoto);
    deleteParam(paramDisplayScreen);
    deleteParam(paramStepCurrent);
    deleteParam(paramStepCount);
    deleteParam(paramStepType);
    deleteParam(paramTimeType);
    deleteParam(paramTimeLeft);
    deleteParam(paramTimePassed);
    deleteParam(paramTimePassedTotal);
    deleteParam(paramTimeScale);
    deleteParam(paramMaxTemp);
  }

  void setNewBlockParameters() {
    paramNext->setFalse();
    paramStop->setFalse();
    paramGoto->setValue(-1);
    //paramTimeLeft->setValue(0);
    paramTimePassed->setValue(0);
    paramTimePassedTotal->setValue(0);
  }

  void deleteParam(Parameter*& param) {
    if (param != nullptr && param->isAnonymous) {
      delete param;
    }
    param = nullptr; // в любом случае обнуляем ссылку (если парам не анон - его удалит memory)
  }

  // инициализировать указанное количество блоков
  void initBlocks(uint16_t count) {
    cleanupBlocks(); // clear previous before
    blocks = new ProgramBlock * [count]; // Выделение памяти под массив указателей
    blockCount = count;
  }

  ProgramBlock* createBlock(uint16_t countControllers) {
    ProgramBlock* block = new ProgramBlock(memory, time);
    addBlock(block);
    block->init(countControllers);
    return block;
  }

  void addBlock(ProgramBlock* block) {
    if (initedCount >= blockCount) {
      Serial.println("Error: exceed blocks count");
      return;
    }
    blocks[initedCount] = block;
    initedCount++;
  }

  ProgramBlock* getBlock(uint8_t number) {
    if (number < initedCount) {
      return blocks[number];
    }
    else {
      return nullptr;
    }
  }

  ProgramBlock* currentBlock() {
    return blocks[currentStep];
  }

  // код программы загружен
  void programLoaded() {
    isLoaded = true;
  }

  // переход к другому блоку
  bool setCurrentStep(uint16_t step) {
    if (step < 0 || step >= initedCount) {
      return false;
    }
    currentStep = step;
    blockTimer->reset();

    return true;
  }

  // перевести программу в активный режим
  void start(bool fromBegin = true) {
    if (!isLoaded) {
      debugln("err: prog not loaded.");
      return;
    }
    isRunning = true;
    if (fromBegin) {
      currentStep = 0;
    }
    bindMemoryParameters();
    mainTimer->reset();
    blockTimer->reset();
  }

  // цикл обновления всех контроллеров активного блока
  void update() {
    if (!isRunning) {
      if (currentStep >= blockCount) {
        isRunning = false;
      }
      return;
    }

    ProgramBlock* block = currentBlock();
    if (block == nullptr) {
      debug("current block not defined");
      return;
    }

    // обновить время выполнения программы и текущего блока
    paramTimePassedTotal->setValue(static_cast<int32_t>(mainTimer->periodTime()));
    paramTimePassed->setValue(static_cast<int32_t>(blockTimer->periodTime()));

    if (!block->isWasExecuted) {
      runBeforeBlock(block);
    }

    runUpdateBlock(block);

    block->isWasExecuted = true;

    checkUpdateResult(block);
  }

  // проверяет состояние после выполнения update
  void checkUpdateResult(ProgramBlock* block) {
    bool isSwitched = false; // блок нужно переключить
    bool _isStopped = false; // аварийная остановка
    bool _isSuccess = false; // успешное завершение

    if (paramStop->isTrue()) { // аварийная программная остановка
      _isStopped = true;
    }

    if (paramNext->isTrue()) {
      if (setCurrentStep(currentStep + 1)) {
        isSwitched = true;
      }
      else {
        _isSuccess = true;
      }
    }

    int32_t nextBlock = paramGoto->getInt();
    if (nextBlock != -1) {
      if (setCurrentStep(nextBlock)) {
        isSwitched = true;
      }
      else {
        _isStopped = true;
      }
    }

    // Завершение текущего блока
    if (isSwitched || _isStopped || _isSuccess) {
      runAfterBlock(block);
      setNewBlockParameters();
    }

    if (_isStopped) {
      isStopped = _isStopped;
    }

    if (_isSuccess) {
      isSuccess = _isSuccess;
    }
  }

  void runUpdateBlock(ProgramBlock* block) {
    //if (_debug) debugln("block update");
    for (uint16_t i = 0; i < block->initedCount; i++) {
      BaseController* ctrl = block->controllers[i];

      ctrl->_isExecuted = ctrl->isActive();

      if (ctrl->_isExecuted) {
        ctrl->update();
        ctrl->_isWasExecuted = true;
      }
    }
  }

  void runBeforeBlock(ProgramBlock* block) {
    //if (_debug) debugln("block before");
    for (uint16_t i = 0; i < block->initedCount; i++) {
      BaseController* ctrl = block->controllers[i];
      ctrl->before();
    }
  }

  void runAfterBlock(ProgramBlock* block) {
    //if (_debug) debugln("block after");
    for (uint16_t i = 0; i < block->initedCount; i++) {
      BaseController* ctrl = block->controllers[i];
      ctrl->after();
    }
  }

};
