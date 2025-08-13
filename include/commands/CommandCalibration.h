#pragma once
#include <string>
#include "command.h"
#include "events/EventDescriptor.h"

class CommandCalibration : public Command {
private:
  float valueT1CalibrationUser = 0.0f;
  bool hasT1CalibrationUser = false;

  float valueT1CalibrationFactory = 0.0f;
  bool hasT1CalibrationFactory = false;

  float valueH1MaxPower = 0.0f; // max power of heater
  bool hasH1MaxPower = false;

public:
  bool setParam(const std::string& paramName, const std::string& paramValue) {
    if (paramName == "t1_clbr") {
      valueT1CalibrationUser = atof(paramValue.c_str());
      if (valueT1CalibrationUser > TEMPERATURE_CALIBRATION_MIN && valueT1CalibrationUser < TEMPERATURE_CALIBRATION_MAX) {
        hasT1CalibrationUser = true;
      }
    }
    else if (paramName == "t1_clbr_factory") {
      valueT1CalibrationFactory = atof(paramValue.c_str());
      if (valueT1CalibrationFactory > TEMPERATURE_CALIBRATION_MIN && valueT1CalibrationFactory < TEMPERATURE_CALIBRATION_MAX) {
        hasT1CalibrationFactory = true;
      }
    }
    else if (paramName == "h1_max_power") {
      valueH1MaxPower = atof(paramValue.c_str());
      if ((valueH1MaxPower >= 0.1f) && (valueH1MaxPower <= 1.0f)) {
        hasH1MaxPower = true;
      }
    }
    else {
      Command::setParam(paramName, paramValue);
    }
    return true;
  }

  void invoke() override {
    if (hasT1CalibrationUser) {
      app.setTemperatureCalibration(valueT1CalibrationUser);
    }
    if (hasT1CalibrationFactory) {
      app.setTemperatureCalibrationFactory(valueT1CalibrationFactory);
    }
    if (hasH1MaxPower) {
      app.setHeaterMaxPower(valueH1MaxPower);
    }

    EventDescriptor evt;
    emit(evt);
  }
};

