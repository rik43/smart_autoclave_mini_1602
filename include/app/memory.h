#pragma once
#include <cstring>
#include "app/parameter.h"
#include "compiler/compilation_error.h"
#include <global.h>

class Memory {

  private:
    uint8_t initedCount = 0;
    uint8_t parameterCount = 0;
    Parameter** parameters;

  public:

    Memory(): parameters(nullptr) {}

    Memory(const Memory&) = delete; // copy disable
    Memory& operator=(const Memory&) = delete; // copy disable
    ~Memory() {
      cleanup();
    }

    void init(uint8_t count) {
      cleanup(); // clear previous before
      parameters = new Parameter*[count]; // Выделение памяти под массив указателей
      parameterCount = count;
    }

    void cleanup() {
      if (parameters != nullptr) {
        for (uint8_t i = 0; i < initedCount; ++i) {
          delete parameters[i];
        }
        delete[] parameters;
        parameters = nullptr;
      }
      parameterCount = 0;
      initedCount = 0;
    }

    void addParam(Parameter* param) {
      if (initedCount >= parameterCount) {
        debugln("Error: exceed parameters count");
        return;
      }
      parameters[initedCount] = param;
      initedCount++;
    }

    uint8_t getInitedCount() {
      return initedCount;
    }

    Parameter* getParamByIndex(uint8_t index) {
      if (index < initedCount) {
        return parameters[index];
      } else {
        return nullptr;
      }
    }

    Parameter* findParam(uint32_t id) {
      for (uint8_t i = 0; i < initedCount; ++i) {
        if (parameters[i]->id == id) {
          return parameters[i];
        }
      }
      return nullptr;
    }

    Parameter* findParam(const char* idStr) {
      return findParam(Parameter::idFromString(idStr));
    }

    Parameter* findParam(std::string idStr) {
      return findParam(idStr.c_str());
    }


    // находит готовый или создает параметр
    // строка value может быть или значением переменной "123" или ссылкой на параметр "{VAR1}"
    // новые ячейки в основную память не добавляются
    Parameter* makeParameter(ParameterType parameterType, const char* value) {
      Parameter* param;
      if (value[0] == '{' && value[strlen(value) - 1] == '}') {
        // Переменная: "{VAR1}". Извлекаем текст между '{' и '}'
        size_t length = strlen(value);
        char paramVariable[length - 1];  // -1 для исключения '{' и '}'
        strncpy(paramVariable, value + 1, length - 2);
        paramVariable[length - 2] = '\0';
        param = findParam(paramVariable);
        if (param == nullptr) {
          prt("Variable not found: ", value);
        }
      } else {
        // Значение переменной: "123" - создать новую анонимную переменную
        param = Parameter::create(parameterType);
        param->isAnonymous = true;
        param->setValue(value);
      }

      return param;
    }

    // возвращает переменную из памяти, если не найдена - создаётся новый
    // новые ячейки в основную память не добавляются
    Parameter* makeParameter(ParameterType desiredParameterType, const char* name, const char* value) {
      Parameter* param = findParam(name);
      if (param == nullptr) {
        param = Parameter::create(desiredParameterType);
        param->id = Parameter::idFromString(name);
        param->isAnonymous = true;
        param->setValue(value);
      }
      return param;
    }

    // возвращает переменную из памяти, если не найдена - создаётся новый
    // новые ячейки в основную память не добавляются
    Parameter* anonParameter(ParameterType parameterType, const char* value) {
      Parameter* param = Parameter::create(parameterType);
      param->isAnonymous = true;
      param->setValue(value);
      return param;
    }

};