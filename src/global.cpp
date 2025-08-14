#include "global.h"
#include "const.h"
#include "events/event.h"
#include "commands/command.h"
#include "net/connector.h"
#include "commands/command_factory.h"
#include "ui/AppController.h"

extern Connector connector;

// отправить событие на контроллера/телефон
void emit(Event& event) {
  connector.send(event);
}

// обработка внутренней команды
void dispatch(Command* cmd) {
  if (cmd != nullptr) {
    //connector.dispatch(*cmd);
    cmd->invoke();
  }
}

static CommandFactory commandFactory;

// получение команды с контроллера/телефона
void dispatch(const char* message) {
  Command* cmd = nullptr;
  try {
    prt("CMD", message);

    cmd = commandFactory.makeFromString(message);
    if (cmd != nullptr) {
      dispatch(cmd);
    }
    else {
      debugln("No command created");
    }
  }
  catch (const CommandError& e) {
    prt("Command error: ", e.what());
  }

  if (cmd != nullptr) {
    delete cmd;
  }
}

extern AppController appController;
void lcdDraw() {
  appController.draw();
}
void startAutoclaveProcess() {
  appController.startAutoclaveProcess();
}
void stopAutoclaveProcess() {
  appController.stopAutoclaveProcess();
}