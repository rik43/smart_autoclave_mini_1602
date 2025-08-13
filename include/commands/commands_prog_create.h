#pragma once
#include <sstream>
#include "app/app.h"
#include "commands/command.h"
#include "events/events_prog.h"

extern App app;


class CommandHelper {
public: 

  static void sendEventProgInfo() {
    EventProgInfo event(app.program.pid, app.program.recipeId);
    event.desc = app.program.desc;
    event.uuid = app.program.recipeUuid;
    event.maxTemperature = app.program.maxTemperature;
    event.errorCode = app.errorCode;
    event.stoppedStep = app.stoppedStep;
    event.stoppedSecond = app.stoppedSecond;
    
    if (app.program.isRunning) event.status = EventProgInfo::STATUS_RUNNING;
    else if (app.errorCode) event.status = EventProgInfo::STATUS_ERROR;
    else if (app.program.isSuccess) event.status = EventProgInfo::STATUS_COMPLETE;
    else if (app.program.isStopped) event.status = EventProgInfo::STATUS_STOPPED;
    else event.status = EventProgInfo::STATUS_IDLE;
    
    emit(event);
  }

};
 


class CommandProgNew : public Command {
public:
  uint32_t pid;
  uint32_t id;
  std::string uuid;

  bool setParam(const std::string& paramName, const std::string& paramValue) {
    if (paramName == "pid") {
      pid = std::stoi(paramValue);
    }
    else if (paramName == "id") {
      id = std::stoi(paramValue);
    }
    else if (paramName == "uuid") {
      uuid = paramValue;
    }
    else {
      return Command::setParam(paramName, paramValue);
    }
    return true;
  }

  void invoke() override {
    debugln("NEW!!!");

    app.compiler.create(pid, id, uuid);

    EventProgReading event(pid, id);
    emit(event);
  }

};

class CommandProgLine : public Command {
public:
  uint32_t pid;
  uint32_t lineNumber = 0;
  std::string code;

  bool setParam(const std::string& paramName, const std::string& paramValue) {
    if (paramName == "pid") {
      pid = std::stoi(paramValue);

    }
    else if (paramName == "line") {
      lineNumber = std::stoi(paramValue);
    }
    else if (paramName == "code") {
      code = paramValue;
    }
    else {
      return Command::setParam(paramName, paramValue);
    }
    return true;
  }

  void invoke() override {
    std::istringstream stream(code);
    std::string line;
    uint32_t currentLineNumber = lineNumber;

    // Разделяем входную строку на отдельные строки
    while (std::getline(stream, line)) {
      app.compiler.addLine(pid, currentLineNumber++, line.c_str());
    }

    // Проверяем наличие ошибок после добавления каждой строки
    if (app.compiler.hasError()) {
      EventProgError event(app.compiler.errorMsg, static_cast<uint32_t>(app.compiler.errorCode), pid, app.compiler.getId());
      event.setErrorLine(app.compiler.errorLineNumber, app.compiler.errorCharNumber);
      emit(event);
      return;
    }

    if (app.compiler.isProgramComplete) {
      EventProgReady event(pid, app.compiler.getId());
      emit(event);
    }
  }

};

class CommandProgStart : public Command {
public:
  uint32_t pid;

  bool setParam(const std::string& paramName, const std::string& paramValue) {
    if (paramName == "pid") {
      pid = std::stoi(paramValue);
    }
    else {
      return Command::setParam(paramName, paramValue);
    }
    return true;
  }

  void invoke() override {
    if (app.programStart(pid)) {
      // событие о успешном запуске
      EventProgStarted event(pid, app.program.recipeId);
      emit(event);
    }
    else {
      // TODO: send error
    }

    // публикуем для всех информацию о запущенной программе
    // EventProgInfo event(app.program.pid, app.program.recipeId);
    // event.desc = app.program.desc;
    // event.uuid = app.program.recipeUuid;
    // emit(event);
    CommandHelper::sendEventProgInfo();
  }

};


// сохраняет дескриптор запущенной программы перед запуском (для моб приложения)
class CommandProgDesc : public Command {
public:
  uint32_t pid;
  std::string desc;

  bool setParam(const std::string& paramName, const std::string& paramValue) {
    if (paramName == "pid") {
      pid = std::stoi(paramValue);
    }
    else if (paramName == "desc") {
      desc = paramValue;
    }
    else {
      return Command::setParam(paramName, paramValue);
    }

    return true;
  }

  void invoke() override {
    if (app.program.pid == pid) {
      app.program.desc = desc;
    }
  }

};


class CommandProgInfo : public Command {
public:
  bool setParam(const std::string& paramName, const std::string& paramValue) {
    return Command::setParam(paramName, paramValue);
  }

  void invoke() override {
    // EventProgInfo event(app.program.pid, app.program.recipeId);
    // event.desc = app.program.desc;
    // event.uuid = app.program.recipeUuid;
    // emit(event);
    CommandHelper::sendEventProgInfo();
  }

};
