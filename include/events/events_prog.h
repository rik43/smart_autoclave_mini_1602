#pragma once
#include <string>
#include "events/event.h"

// ошибка загрузки и запуска программы
class EventProgError : public Event {
private:
  uint32_t pid;
  uint32_t id;
  uint32_t code;
  std::string textMessage;
  int errorLine = 0; // номер строки с ошибкой
  int errorChar = 0; // номер символа в строке с ошибкой

public:
  EventProgError(const std::string& msg, uint32_t code, uint32_t pid, uint32_t id) : Event("prog.error"), textMessage(msg), code(code), pid(pid), id(id) {}

  void setErrorLine(int _line, int _char) {
    errorLine = _line;
    errorChar = _char;
  }

  void toString(std::string& msg) override {
    addParam(msg, "pid", std::to_string(pid));
    addParam(msg, "id", std::to_string(id));
    addParam(msg, "code", std::to_string(code));
    addParam(msg, "msg", textMessage);
    addParam(msg, "line", std::to_string(errorLine));
    addParam(msg, "char", std::to_string(errorChar));
  }
};


// Общее событие по прогрессу записи программы
class EventProgCommon : public Event {
public:
  uint32_t pid;
  uint32_t id;

  EventProgCommon(const std::string& name, uint32_t pid, uint32_t id) : Event(name), pid(pid), id(id) {}

  void toString(std::string& msg) override {
    addParam(msg, "pid", std::to_string(pid));
    addParam(msg, "id", std::to_string(id));
  }
};

// запись программы начата
class EventProgReading : public EventProgCommon {
public:
  EventProgReading(uint32_t pid, uint32_t id) : EventProgCommon("prog.reading", pid, id) {}
};

// программа записана полностью, готова к запуску
class EventProgReady : public EventProgCommon {
public:
  EventProgReady(uint32_t pid, uint32_t id) : EventProgCommon("prog.ready", pid, id) {}
};

// программа запущена
class EventProgStarted : public EventProgCommon {
public:
  EventProgStarted(uint32_t pid, uint32_t id) : EventProgCommon("prog.started", pid, id) {}
};

// Информация о программе
class EventProgInfo : public EventProgCommon {
public:

  std::string desc;
  std::string uuid;

  enum Status {
    STATUS_IDLE = 0,
    STATUS_RUNNING = 1,
    STATUS_COMPLETE = 2,
    STATUS_STOPPED = 3, 
    STATUS_ERROR = 4, 
  } status;

  int16_t errorCode = 0;
  int16_t stoppedStep = 0; // номер шага на котором остановлено
  uint32_t stoppedSecond = 0; // время с начала шага на котором остановлено
  int16_t maxTemperature = 0;

  EventProgInfo(uint32_t pid, uint32_t id) : EventProgCommon("prog.info", pid, id) {}

  void toString(std::string& msg) override {
    EventProgCommon::toString(msg);
    addParam(msg, "desc", desc);
    addParam(msg, "uuid", uuid);

    addParam(msg, "status", statusToString(status));
    addParam(msg, "stop_step", std::to_string(stoppedStep));
    addParam(msg, "stop_sec", std::to_string(stoppedSecond));

    if (errorCode) addParam(msg, "error", std::to_string(errorCode));
    if (maxTemperature) addParam(msg, "maxt", std::to_string(maxTemperature));
  }

  std::string statusToString(EventProgInfo::Status status) {
    switch (status) {
        case EventProgInfo::STATUS_IDLE: return "IDLE";
        case EventProgInfo::STATUS_RUNNING: return "RUNNING";
        case EventProgInfo::STATUS_COMPLETE: return "COMPLETE";
        case EventProgInfo::STATUS_STOPPED: return "STOPPED";
        case EventProgInfo::STATUS_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
  }

};

// программа остановлена пользователем
class EventProgStopped : public EventProgCommon {
public:
  // Переменные из App:
  int16_t errorCode = 0;
  int16_t stoppedStep = 0; // номер шага на котором остановлено
  uint32_t stoppedSecond = 0; // время с начала шага на котором остановлено

  EventProgStopped(uint32_t pid, uint32_t id): EventProgCommon("prog.stopped", pid, id) {}

  void toString(std::string& msg) override {
    EventProgCommon::toString(msg);
    addParam(msg, "error", std::to_string(errorCode));
    addParam(msg, "stop_step", std::to_string(stoppedStep));
    addParam(msg, "stop_sec", std::to_string(stoppedSecond));
  }
};

// программа успешно завершена
class EventProgCompleted : public EventProgCommon {
public:
  EventProgCompleted(uint32_t pid, uint32_t id) : EventProgCommon("prog.completed", pid, id) {}
};

// программа временно приостановлена
//class EventProgPaused : public EventProgCommon {
//public:
//  EventProgPaused(uint32_t pid, uint32_t id) : EventProgCommon("prog.paused", pid, id) {}
//};
//
//// программа возобновлена после остановки
//class EventProgResumed : public EventProgCommon {
//public:
//  EventProgResumed(uint32_t pid, uint32_t id) : EventProgCommon("prog.resumed", pid, id) {}
//};


