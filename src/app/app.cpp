#include "app/app.h"
#include "global.h"
#include "compiler/parameter_factory.h"
#include "compiler/compilation_error.h"
#include "settings/SettingsStore.h"
#include "events/EventSoftState.h"
#include "events/events_prog.h"
#include "utils/LEDManager.h"


extern LEDManager ledManager;


App::App(SettingsStore& settings) :
  program(memory, vtime),
  hardware(vtime),
  compiler(program),
  settings(settings)
{
  timerSendState.setPeriod(1000);
  timerAppUpdate.setPeriod(200);
}

void App::setup() {
  applyCallibration();
}

void App::applyCallibration() {
  hardware.setTemperatureCalibration(settings.getCalibration());
  hardware.setHeaterMaxPower(settings.getHeaterMaxPower());
}

void App::setTemperatureCalibration(float value) {
  settings.setUserCalibration(value);
  applyCallibration();
}

void App::setTemperatureCalibrationFactory(float value) {
  settings.setFactoryCalibration(value);
  applyCallibration();
}

void App::setHeaterMaxPower(float value) {
  settings.setHeaterMaxPower(value);
  applyCallibration();
}

void App::updateProgramSensors() {
  auto temperature = hardware.temperature;
  // обновить макс темп-ру
  if (temperature > program.paramMaxTemp->getFloat()) {
    program.paramMaxTemp->setValue(temperature);
    program.maxTemperature = static_cast<uint16_t>(round(temperature));
  }
}

void App::checkEndState() {
  // проверка на завершение программы
  if (program.isStopped || program.isSuccess) {
    hardware.stop(); // также будет выключена позже, но на всякий случай выкл в самом начале
    debugln(program.isStopped ? "Stopped" : "Success");
    publishSoftState();
    programStop(program.isStopped);
  } 
  else {
    hardware.applyState();  
  }
}

void App::loop() {

  if (timerAppUpdate.ready()) {
    hardware.readSensorData();

    if (program.isRunning) {
      updateProgramSensors();

      if (hardware.isValidTemperature) {
        program.update();
        checkEndState();
      }
      else {
        // аварийная остановка
        errorCode = ERROR_CODE_NOT_VALID_TEMPERATURE;
        programStop(true);
      }
    }
    else {
      hardware.stop();
    }
  }

  if (timerSendState.ready()) {
    publishState();
  }
}

void App::publishState() {
  // hard-state
  hardware.publishState();

  // soft-state
  if (program.isRunning) {
    publishSoftState();
  }
}

void App::publishSoftState() {
    EventSoftState state(memory);
    emit(state);
}

bool App::programStart(uint32_t pid) {
  if (!compiler.isAllowStart(pid)) {
    Serial.println("Not allowed start");
    return false;
  }
  //Serial.println("START");

  hardware.bindMemoryParameters(memory);
  program.start();

  errorCode = 0;
  stoppedStep = 0;
  stoppedSecond = 0;

  ledManager.pulseActive();

  return true;
}

void App::programStop(bool isStopped) {
  if (! (program.isLoaded || program.isRunning)) return;

  ledManager.pulseIdle();

  stoppedStep = static_cast<int16_t>(program.paramStepCurrent->getInt());
  stoppedSecond = static_cast<uint32_t>(program.paramTimePassed->getInt() / 1000);

  if (isStopped) {
    program.isStopped = true;
  } else {
    program.isSuccess = true;
  }

  if (isStopped) {
    EventProgStopped event(program.pid, program.recipeId);
    event.errorCode = errorCode;
    event.stoppedStep = stoppedStep;
    event.stoppedSecond = stoppedSecond;
    emit(event);
  }
  else {
    EventProgCompleted event(program.pid, program.recipeId);
    emit(event);
  }  

  hardware.stop();
  hardware.unbindMemoryParameters();
  program.stop();
}
