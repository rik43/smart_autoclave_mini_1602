#pragma once

#include <sstream>
#include <string>
#include <iomanip> // for std::setprecision
#include "compiler/compilation_error.h"
#include <Arduino.h>

enum ParameterType {
  PARAMETER_TYPE_FLOAT = 0,
  PARAMETER_TYPE_INT = 1,
  PARAMETER_TYPE_BOOL = 2,
  PARAMETER_TYPE_STRING = 3
};

class Parameter {

  private:
    //float prevValue = 0;
    //float value = 0;
    //int read_at = 0;
    bool _isNullable = true;
    bool _isNull = true;

    //unsigned long changed_at = 0;
    //unsigned long published_at = 0;
    //int publish_period = 200;

  public:
    uint32_t id;
    bool isPublished = false; // публикуется на сервер/телефон
    bool isAnonymous = false; // анонимный/частный параметр, не в общем стеке, создает и уничтожает сам контроллер

    Parameter() = default; // Явно указываем, что конструктор по умолчанию разрешен
    Parameter(const Parameter&) = delete; // copy disable
    Parameter& operator=(const Parameter&) = delete; // copy disable
    virtual ~Parameter() = default; // Виртуальный деструктор для нормального удаления через delete

    static Parameter* create(ParameterType type);

    static uint32_t idFromString(const char* idStr) {
      // копируем 4 байта в int
      uint32_t id;
      memcpy(&id, idStr, sizeof(id)); // little-endian - младший байт идет первым
      return id;
    }

    const char* idString() {
      static char str[5] = "0000";
      memcpy(str, &id, 4);
      return str;
    }

//    unsigned long getPublishedAt() {
//      return published_at;
//    }
//
//    unsigned long getChangedAt() {
//      return changed_at;
//    }
//
//    float getPublishPeriod() {
//      return publish_period;
//    }

    void updated() {
      _isNull = false;
    }

    virtual float getFloat() {
      return 0.0;
    };

    virtual int32_t getInt() {
      return 0;
    };

    virtual bool getBool() {
      return false;
    };

    virtual std::string getString() {
      return "";
    };

    virtual void setValue(float value) {};

    virtual void setValue(int32_t value) {};

    virtual void setValue(bool value) {};

    virtual void setValue(const char* value) {};

    void setValue(std::string value) {
      setValue(value.c_str());
    }

    void addInt(int32_t val) {
      int32_t sum = getInt() + val;
      setValue(sum);
    }

    //void add(float val) {
    //  setValue(getFloat() + val);
    //}

    bool isTrue() {
      return getBool();
    }

    bool isFalse() {
      return !getBool();
    }

    bool isZero() {
      return getInt() == 0;
    }

    bool isPositive() {
      return getInt() > 0;
    }

    bool isNegative() {
      return getInt() < 0;
    }

    void setTrue() {
      setValue(true);
    }

    void setFalse() {
      setValue(false);
    }

    void setNullable(bool nullable) {
      _isNullable = nullable;
      if (! _isNullable) {
        _isNull = false;
      }
    }

    void setNull() {
      if (_isNullable) {
        _isNull = true;
      }
    }

    bool isNull() {
      return _isNullable && _isNull;
    }

    virtual bool setProp(const char* paramName, const char* paramValue) {
      if (paramName == nullptr || paramValue == nullptr) {
        return false;
      }
      //int32_t intValue = atoi(paramValue);

      // Общая логика для обработки параметров в базовом классе
      if (strcmp(paramName, "public") == 0) {
        isPublished = paramValue[0] == '1';
      } else {
        std::stringstream msg;
        msg << "unknown parameter prop: '" << paramName << "' = '" << paramValue << "'";
        throw CompilationError(msg);
        //Serial.print(F("Error: unknown parameter prop: "));
        //Serial.print(paramName);
        //Serial.print(" = ");
        //Serial.println(paramValue);
        //return false;
      }

      return true;
    }
};

class ParameterFloat: public Parameter {
  private:
    float floatValue = 0.0;

  public:
    float getFloat() override {
      return floatValue;
    }

    int32_t getInt() override {
      return round(floatValue);
    }

    bool getBool() override {
      return abs(floatValue) > 0.001;
    }

    std::string getString() override {
      std::ostringstream stream;
      stream << std::fixed << std::setprecision(3) << floatValue;
      std::string result = stream.str();
      return result;
    }

    // SET
    void setValue(float value) override {
      floatValue = value;
      updated();
    }

    void setValue(int32_t value) override {
      floatValue = static_cast<float>(value);
      updated();
    }

    void setValue(bool value) override {
      floatValue = static_cast<float>(value);
      updated();
    }

    void setValue(const char* value) override {
      floatValue = atof(value);
      updated();
    }
};


class ParameterInt: public Parameter {
  private:
    int32_t intValue = 0;

  public:
    float getFloat() override {
      return intValue;
    }

    int32_t getInt() override {
      return intValue;
    }

    bool getBool() override {
      return intValue != 0;
    }

    std::string getString() override {
      return std::to_string(intValue);
    }

    // SET
    void setValue(float value) override {
      intValue = static_cast<int32_t>(value);
      updated();
    }

    void setValue(int32_t value) override {
      intValue = value;
      updated();
    }

    void setValue(bool value) override {
      intValue = static_cast<int32_t>(value);
      updated();
    }

    void setValue(const char* value) override {
      intValue = atoi(value);
      updated();
    }
};


class ParameterBool: public Parameter {
  private:
    float boolValue = false;

  public:
    float getFloat() override {
      return boolValue ? 1.0 : 0.0;
    }

    int32_t getInt() override {
      return boolValue ? 1 : 0;
    }

    bool getBool() override {
      return boolValue;
    }

    std::string getString() override {
      return (boolValue ? "1" : "0");
    }

    // SET
    void setValue(float value) override {
      boolValue = static_cast<bool>(value);
      updated();
    }

    void setValue(int32_t value) override {
      boolValue = static_cast<bool>(value);
      updated();
    }

    void setValue(bool value) override {
      boolValue = value;
      updated();
    }

    void setValue(const char* value) override {
      boolValue = static_cast<bool>(atoi(value));
      updated();
    }
};


class ParameterString: public Parameter {
  private:
    std::string stringValue;

  public:
    float getFloat() override {
      // std::stof can be used to convert a string to a float
      // Be aware of potential exceptions if stringValue cannot be converted
      try {
          return std::stof(stringValue);
      } catch (...) {
          return 0.0f; // Return a default value if conversion fails
      }
    }

    int32_t getInt() override {
      // std::stoi can convert a string to an int
      // Like std::stof, std::stoi can throw exceptions
      try {
          return std::stoi(stringValue);
      } catch (...) {
          return 0; // Return a default value if conversion fails
      }
    }

    bool getBool() override {
      return !stringValue.empty() && stringValue != "0";
    }

    std::string getString() override {
      return stringValue;
    }

    // SET
    void setValue(float value) override {
      std::ostringstream stream;
      stream << std::fixed << std::setprecision(4) << value;
      stringValue = stream.str();
      updated();
    }

    void setValue(int32_t value) override {
      stringValue = std::to_string(value);
      updated();
    }

    void setValue(bool value) override {
      stringValue = (value ? '1' : '0');
      updated();
    }

    void setValue(const char* value) override {
      stringValue = value;
      updated();
    }
};
