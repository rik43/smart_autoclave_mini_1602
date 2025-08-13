#pragma once
#include "app/hardware.h"
#include "app/parameter.h"
#include "compiler/compilation_error.h"

class ParameterFactory {
  private:

  public:
    ParameterFactory() = default; // Явно указываем, что конструктор по умолчанию разрешен
    ParameterFactory(const ParameterFactory&) = delete; // copy disable
    ParameterFactory& operator=(const ParameterFactory&) = delete; // copy disable

    Parameter* makeFromString(const char* descr) {
      // Ищем первый символ @ в строке
      const char* firstAt = strchr(descr, '@');

      if (firstAt != NULL) {
        // Ищем второй символ @, начиная с позиции после первого @
        const char* secondAt = strchr(firstAt + 1, '@');

        if (secondAt != NULL) {
          // Находим имя, которое находится между первым и вторым @
          // @VAR1:float=50.45@
          // @VAR1=50.45@  // float by default
          // @VAR1@  // =0 by default
          std::string name(firstAt + 1, secondAt - firstAt - 1);

          // Получаем параметры, которые начинаются после второго @
          std::string params(secondAt + 1);

          // Извлекаем значение переменной, после символа "=" если есть
          size_t equalsIndex = name.find('='); // Находим позицию равно "T001:int=45"

          std::string initialValue;

          if (equalsIndex != std::string::npos) {
            // Получаем подстроку после двоеточия
            initialValue = name.substr(equalsIndex + 1);

            // Обрезаем оригинальную строку, оставляя только имя:тип
            name = name.substr(0, equalsIndex);
          } else {
            initialValue = "0";
          }

          // извлекаем тип переменной, после ":" если есть
          std::string typeName;

          size_t colonIndex = name.find(':'); // Находим позицию двоеточия "T001:int"

          if (colonIndex != std::string::npos) {
            // Получаем подстроку после двоеточия
            typeName = name.substr(colonIndex + 1);

            // Обрезаем оригинальную строку, оставляя только имя
            name = name.substr(0, colonIndex);
          }

          ParameterType paramType = parseParameterTypeName(typeName.c_str());

          return makeParameter(paramType, name.c_str(), initialValue.c_str(), params.c_str());
        } else {
          // Если второй @ не найден, выводим ошибку
          //Serial.println("Error: Second @ not found.");
          throw CompilationError("Param parser: Second @ not found.");
        }
      } else {
        // Если первый @ не найден, выводим ошибку
        //Serial.println("Error: First @ not found.");
        throw CompilationError("Param parser: First @ not found.");
      }
    }

    Parameter* makeParameter(ParameterType type, const char* name, const char* initialValue, const char* props) {
      Parameter* param = Parameter::create(type);
      if (param != nullptr) {
        param->id = Parameter::idFromString(name);
        param->setValue(initialValue);
        parseAndSetProps(param, props);
      }

      return param;
    }
    
    ParameterType parseParameterTypeName(const char* type) {
      if (strcmp(type, "float") == 0) {
        return PARAMETER_TYPE_FLOAT;
      } else if (strcmp(type, "int") == 0) {
        return PARAMETER_TYPE_INT;
      } else if (strcmp(type, "bool") == 0) {
        return PARAMETER_TYPE_BOOL;
      } else if (strcmp(type, "string") == 0) {
        return PARAMETER_TYPE_STRING;
      } else {
        std::stringstream msg;
        msg << "unknown param type: '" << type << "'";
        throw CompilationError(msg);
      }
    }

    void parseAndSetProps(Parameter* param, const char* params) {
      // Создаем временные переменные для хранения названия и значения параметров
      char paramName[32];  // максимальная длина названия параметра
      char paramValue[128];  // максимальная длина значения параметра

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
          param->setProp(paramName, paramValue);
        } else {
          // no equal char between name and params
          // todo return error if needed
        }

        // Переходим к следующей паре
        token = strtok(NULL, "&");
      }

    }

};
