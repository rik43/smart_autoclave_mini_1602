#pragma once

#include <string>
#include "global.h"
#include "app/program.h"
#include "app/hardware.h"
#include "app/memory.h"
#include "timer.h"
#include "app/time.h"
#include "compiler/program_compiler.h"

class SettingsStore;

class App {
private:
  bool _debug = true;

  Timer timerAppUpdate;

  Timer timerSendState;

  void publishState();
  void publishSoftState();

public:
  VTime vtime;
  Memory memory;
  Program program;
  Hardware hardware;
  ProgramCompiler compiler;
  SettingsStore& settings;

  int16_t errorCode = 0; // ошибка выполнения программы
  int16_t stoppedStep = 0; // номер шага на котором остановлено
  uint32_t stoppedSecond = 0; // время с начала шага на котором остановлено

  App(SettingsStore& settings);
  App(const App&) = delete; // copy disable
  App& operator=(const App&) = delete; // copy disable
  void setup();
  void loop();
  void programStop(bool isFail = false);
  bool programStart(uint32_t pid);
  void updateProgramSensors();
  void checkEndState();
  void applyCallibration();
  void setTemperatureCalibration(float value);
  void setTemperatureCalibrationFactory(float value);
  void setHeaterMaxPower(float value);
};
