#pragma once
#include "app/hardware.h"
#include "controllers/controllers.h"
#include "compiler/compilation_error.h"

class ControllerFactory {
private:

public:
  Memory& memory;

  ControllerFactory(Memory& mem) : memory(mem) {}

  ControllerFactory(const ControllerFactory&) = delete; // copy disable
  ControllerFactory& operator=(const ControllerFactory&) = delete; // copy disable

  BaseController* getNewController(const char* name) {
    if (strcmp(name, "PID") == 0) {
      return new PIDController();
    }
    else if (strcmp(name, "Timer") == 0) {
      return new TimerController();
    }
    else if (strcmp(name, "S") == 0) { // shortcut for SetParam
      return new SetParamController();
    }
    else if (strcmp(name, "Divide") == 0) {
      return new DivideController();
    }
    else if (strcmp(name, "Compare") == 0) {
      return new CompareController();
    }
    else if (strcmp(name, "Next") == 0) {
      return new NextController();
    }
    else if (strcmp(name, "SetParam") == 0) { // deprecated variant of "S"
      return new SetParamController();
    }

    prt("Error: unknown controller: ", name);
    return nullptr;
  }

  // @controller@param1=value1&param2=value2
  BaseController* makeFromString(const char* descr) {
    // Ищем первый символ @ в строке
    const char* firstAt = strchr(descr, '@');

    if (firstAt != NULL) {
      // Ищем второй символ @, начиная с позиции после первого @
      const char* secondAt = strchr(firstAt + 1, '@');

      if (secondAt != NULL) {
        // Находим имя, которое находится между первым и вторым @
        std::string name(firstAt + 1, secondAt - firstAt - 1);

        // Получаем параметры, которые начинаются после второго @
        std::string params(secondAt + 1);

        return makeController(name.c_str(), params.c_str());
      }
      else {
        // Если второй @ не найден, выводим ошибку
        Serial.println("Error: Second @ not found.");
      }
    }
    else {
      // Если первый @ не найден, выводим ошибку
      Serial.println("Error: First @ not found.");
    }

    return nullptr;
  }

  BaseController* makeController(const char* name, const char* params) {
    BaseController* ctrl = getNewController(name);
    if (ctrl != nullptr) {
      parseAndSetParams(ctrl, params);
    }

    return ctrl;
  }

  // TODO заменить utils: parseQueryString
  void parseAndSetParams(BaseController* ctrl, const char* params) {
    // Создаем временные переменные для хранения названия и значения параметров
    char paramName[32];  // максимальная длина названия параметра
    char paramValue[32];  // максимальная длина значения параметра

    // Копируем строку параметров, чтобы не изменять оригинальную строку
    char paramsCopy[strlen(params) + 1];
    strcpy(paramsCopy, params);

    // Разбиваем строку параметров по символу '&'
    char* token = strtok(paramsCopy, "&");

    // Парсим каждую пару название-значение
    while (token != NULL) {
      // Разбиваем пару по символу '='
      char* equalSign = strchr(token, '=');

      if (equalSign != NULL) {
        // Получаем название параметра
        strncpy(paramName, token, equalSign - token);
        paramName[equalSign - token] = '\0';  // Добавляем завершающий нуль

        // Получаем значение параметра
        strcpy(paramValue, equalSign + 1);

        // Вызываем функцию setParam для каждой пары
        ctrl->setParam(paramName, paramValue, memory);
      }
      else {
        // no equal char between name and params
        // todo return error if needed
      }

      // Переходим к следующей паре
      token = strtok(NULL, "&");
    }

  }

};
