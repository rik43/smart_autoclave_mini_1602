#pragma once
#include <string>
#include "commands/commands.h"
#include "commands/command_error.h"
#include "utils.h"

class CommandFactory {
private:

public:

  Command* createNewCommand(const char* name) {
    if (strcmp(name, "prog.stop") == 0) {
      return new CommandProgStop();
    }
    else if (strcmp(name, "descriptor") == 0) {
      return new CommandDescriptor();
    }
    else if (strcmp(name, "set.param") == 0) {
      return new CommandSetParam();
    }
    else if (strcmp(name, "prog.new") == 0) {
      return new CommandProgNew();
    }
    else if (strcmp(name, "prog.line") == 0) {
      return new CommandProgLine();
    }
    else if (strcmp(name, "prog.start") == 0) {
      return new CommandProgStart();
    }
    else if (strcmp(name, "prog.desc") == 0) {
      return new CommandProgDesc();
    }
    else if (strcmp(name, "prog.info") == 0) {
      return new CommandProgInfo();
    }
    else if (strcmp(name, "wifi.forget") == 0) {
      return new CommandWifiForget();
    }
    else if (strcmp(name, "wifi.provisioning") == 0) {
      return new CommandWifiProvisioning();
    }
    else if (strcmp(name, "connection.wipe") == 0) {
      return new CommandConnectionWipe();
    }
    else if (strcmp(name, "update-firmware") == 0) {
      return new CommandUpdateFirmware();
    }
    else if (strcmp(name, "calibration") == 0) {
      return new CommandCalibration();
    }
    else if (strcmp(name, "hello") == 0) { // Ble auth check
      return new CommandHello();
    }
    else {
      std::string errorMessage = "Unknown command: ";
      errorMessage += name;
      throw CommandError(errorMessage);
    }
  }

  Command* makeFromString(const char* descr) {
    // Ищем первый символ @ в строке
    const char* firstAt = strchr(descr, '@');

    if (firstAt != nullptr) {
      // Ищем второй символ @, начиная с позиции после первого @
      const char* secondAt = strchr(firstAt + 1, '@');

      if (secondAt != nullptr) {
        // Находим имя команды, которое находится между первым и вторым @
        // @cmd:123@  // command with id
        // @cmd@      // command without id
        std::string name(firstAt + 1, secondAt - firstAt - 1);

        // Получаем параметры, которые начинаются после второго @
        std::string params(secondAt + 1);

        // извлекаем ид запроса, после ":" если есть
        std::string id;

        size_t colonIndex = name.find(':'); // Находим позицию двоеточия "command:1234"

        if (colonIndex != std::string::npos) {
          // Получаем подстроку после двоеточия
          id = name.substr(colonIndex + 1);

          // оставляя только имя (без ид)
          name = name.substr(0, colonIndex);
        }

        return makeCommand(name.c_str(), id.c_str(), params.c_str());
      }
      else {
        throw CommandError("Command parser: Second @ not found.");
      }
    }
    else {
      throw CommandError("Command parser: First @ not found.");
    }
  }

  Command* makeCommand(const char* name, const char* id, const char* params) {
    Command* cmd = createNewCommand(name);
    if (cmd != nullptr) {
      //if (id != nullptr) {
      //  cmd->setId(atoi(id));
      //}
      auto callback = [cmd](const std::string& param, const std::string& value) {
        //std::cout << "Param: " << param << ", Value: " << value << std::endl;
        cmd->setParam(param, value);
        };

      parseQueryString(params, callback);
    }

    return cmd;
  }

};