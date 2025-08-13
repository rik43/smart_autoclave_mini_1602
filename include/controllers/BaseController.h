#pragma once

// Базовый класс контроллера
class BaseController {
  public:
    std::string type;
    std::string name;
    bool _isOnce = false; // одноразовый запуск
    bool _isWasExecuted = false; // запускался ли хотя бы однажды
    bool _isExecuted = false; // запущен сейчас

    uint32_t id; // актуально??? внешний id команды для ссылки при взаимодействии с сервером (отправка ошибок)

    Parameter* _paramEnabled;        // bool
    Parameter* _paramBegin;      // bool
    Parameter* _paramEnd;        // bool

    BaseController() = default; // Явно указываем, что конструктор по умолчанию разрешен
    BaseController(const BaseController&) = delete; // copy disable
    BaseController& operator=(const BaseController&) = delete; // copy disable
    
    virtual ~BaseController() {
      deleteParam(_paramEnabled);
      deleteParam(_paramBegin);
      deleteParam(_paramEnd);
    }

    // Сброс состояния перед выполнением нового блока
    // выполняется каждый раз при запуске данного блока
    void reset() {
      _isWasExecuted = false;
      _isExecuted = false;
    }

    virtual bool setParam(const char* paramName, const char* paramValue, Memory &memory) {
      // Общая логика для обработки параметров в базовом классе
      if (strcmp(paramName, "enabled") == 0) {
        _paramEnabled = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
      } 
      else if (strcmp(paramName, "beginWhen") == 0) {
        _paramBegin = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
      } 
      else if (strcmp(paramName, "endWhen") == 0) {
        _paramEnd = memory.makeParameter(PARAMETER_TYPE_FLOAT, paramValue);
      } 
      else if (strcmp(paramName, "once") == 0) {
        _isOnce = strcmp(paramValue, "0") != 0;
      } 
      else {
        std::stringstream msg;
        msg << "unknown controller parameter: '" << paramName << "' = '" << paramValue << "'";
        throw CompilationError(msg);
        return false;
      }

      return true;
    }

    // TODO вместо исключения возвращать false;
    bool requiredParam(const char* paramName) {
      std::stringstream msg;
      msg << "not defined required parameter: '" << paramName << "'";
      throw CompilationError(msg);
      return false;
    }

    void deleteParam(Parameter*& param) {
      if ((param != nullptr) && param->isAnonymous) {
        delete param;
        param = nullptr;
      }
    }

    // один раз после загрузки параметров для их валидации и инициализации доп объектов
    virtual bool setup(Memory& memory, VTime& time) {
      return true;
    }

    // активен ли сейчас данный блок для выполнения
    bool isActive() {
      // однароазовый и уже был запущен
      if (_isOnce && _isWasExecuted) {
        return false;
      }

      // check enabled
      if ((_paramEnabled != nullptr) && (_paramEnabled->isFalse())) {
        return false;
      }

      // не выполнялся и есть параметр beginWhen = 0 - стоп
      // проверяется каждый раз до первого запуска
      if (! _isWasExecuted && (_paramBegin != nullptr) && _paramBegin->isFalse()) {
        return false;
      }

      // выполняется и есть параметр endWhen = 1 - стоп
      // если begin & end = 1 одновременно, то как минимум 1 раз блок выполнится
      // т.к. _isExecuted = 1 будет на след итерации loop
      if (_isWasExecuted && (_paramEnd != nullptr)) {
        if (_isExecuted) {
          if (_paramEnd->isTrue()) { // закончить
            return false;
          } // иначе не закончено - продолжаем выполнять
        }
        else {
          return false; // было завершено ранее
        }
      }

      return true;
    }

    // вызов перед выполнением блока
    virtual void before() {} 

    // вызов после выполнения блока
    virtual void after() {} 

    // вызов каждый тик
    virtual void update() {}

};
