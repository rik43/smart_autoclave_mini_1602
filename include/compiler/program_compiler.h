#pragma once 
#include "compiler/compilation_error.h"
#include "controllers/controller_factory.h"

// Компилятор программы принимает программу построчно
class ProgramCompiler
{
private:
  Program& program;

  ControllerFactory controllerFactory;

  ParameterFactory parameterFactory;

  ProgramBlock* block; // текущий инициализируемый блок

  enum CodeSection {
    CODE_NO_SECTION = 0, // между секциями или до/после кода
    CODE_SECTION_PARAMS = 1, // секция параметров
    CODE_SECTION_BLOCKS = 2, // секция блоков
    CODE_SECTION_BLOCK = 3,  // внутри отдельного блока
  } section;

  bool isParamsLoaded = false;
  bool isBlocksLoaded = false;

  uint32_t _pid = 0; // ид процесса 
  uint32_t _id = 0; // ид рецепта
  int loadLineCount = 0;


public:
  CompilationErrorCode errorCode = CPL_ERR_NO;
  std::string errorMsg;
  int errorLineNumber; // номер строки с ошибкой
  int errorCharNumber; // номер символа в строке с ошибкой
  bool isProgramComplete = false;

  ProgramCompiler(Program& program) : program(program), controllerFactory(program.memory) {}

  ProgramCompiler(const ProgramCompiler&) = delete; // copy disable
  ProgramCompiler& operator=(const ProgramCompiler&) = delete; // copy disable

  bool hasError() {
    return errorCode != CPL_ERR_NO;
  }

  uint32_t getId() {
    return _id;
  }

  uint32_t getPid() {
    return _pid;
  }

  // подготовка к загрузке программы, старая удаляется
  void create(uint32_t pid, uint32_t id, const std::string& uuid) {
    reset();
    program.cleanup();
    program.reset();
    _pid = pid;
    _id = id;
    program.pid = pid;
    program.recipeId = id;
    program.recipeUuid = uuid;
  }

  // разрешение на запуск загруженной программы
  // TODO вернуть ошибку вместо исключения
  bool isAllowStart(uint32_t pid) {
    if (pid != _pid) {
      error(CPL_ERR_INCOMPLETE_PROGRAM_RUN, "program not ready: pid mismatch");
      return false;
    }
    else if (!isParamsLoaded) {
      error(CPL_ERR_INCOMPLETE_PROGRAM_RUN, "program not ready: incomplete params");
      return false;
    }
    else if (!isBlocksLoaded) {
      error(CPL_ERR_INCOMPLETE_PROGRAM_RUN, "program not ready: incomplete blocks");
      return false;
    }
    else if (hasError()) {
      error(CPL_ERR_INCOMPLETE_PROGRAM_RUN, "unable to run program: has errors");
      return false;
    }

    return true;
  }

  // добавление строки программы, компиляция
  void addLine(uint32_t pid, int lineNumber, const char* code) {

    if (pid != _pid) return; // другая программа

    if (hasError()) return; // после ошибки ничего не принимаем

    if (loadLineCount != lineNumber) { // указанный номер строки не совпадает с ожидаемым. 
      std::stringstream msg;
      msg << "not consistent line number. expected: " << loadLineCount << ", given: " << lineNumber;
      error(CPL_ERR_NOT_CONSISTENT_LINE_NUMBER, msg.str().c_str());
    }

    loadLineCount++;
    errorLineNumber = lineNumber;

    size_t len = strlen(code);
    if (len == 0) { // skip empty string
      return;
    }
    if ((code[0] == '-') && (code[1] == '-')) { // '--' skip comment lines
      return;
    }

    switch (section) {
    case CODE_NO_SECTION:     parseLineSectionBetween(code); break;
    case CODE_SECTION_PARAMS: parseLineSectionParams(code); break;
    case CODE_SECTION_BLOCKS: parseLineSectionBlocks(code); break;
    case CODE_SECTION_BLOCK:  parseLineSectionBlock(code); break;
    }
  }

private:
  void reset() {
    // если есть недозагруженная программа - удалить
    isParamsLoaded = false;
    isBlocksLoaded = false;
    isProgramComplete = false;
    errorCode = CPL_ERR_NO;
    section = CODE_NO_SECTION;
    _pid = 0;
    _id = 0;
    loadLineCount = 0;
    errorMsg = "";
    errorLineNumber = 0;
    errorCharNumber = 0;
  }

  void error(CompilationErrorCode code, std::string msg, int charNumber = 0) {
    errorCode = code;
    errorMsg = msg;
    errorCharNumber = charNumber;
    //prt(errorMsg.c_str(), errorCode);
  }

  // Меж секций: ищем начало блока
  void parseLineSectionBetween(const char* code) {
    // Поиск начала секции '<P17' or '<B2' 
    byte sectionItemsCount;

    switch (code[0]) {
    case '<':
      if (strlen(code) < 3) {
        error(CPL_ERR_UNEXPECTED_TOKEN, "too short open block command line");
        return;
      }

      sectionItemsCount = atoi(code + 2); // количество вложенных элементов в секции (параметров или блоков)

      switch (code[1]) {
      case 'P': // секция Parameters
        if (isBlocksLoaded) {
          error(CPL_ERR_UNEXPECTED_TOKEN, "parameters must be before blocks");
          return;
        }
        section = CODE_SECTION_PARAMS;
        initParams(sectionItemsCount);
        //Serial.print("Start params section "); Serial.println(sectionItemsCount);
        break;

      case 'B': // секция Blocks
        section = CODE_SECTION_BLOCKS;
        initBlocks(sectionItemsCount);
        Serial.print("Start blocks section "); Serial.println(sectionItemsCount);
        break;

      default:
        // новые виды блоков вызывают ошибку в старых прошивках
        error(CPL_ERR_UNEXPECTED_TOKEN, "unknown block type");
      }
      break;

    default:
      error(CPL_ERR_UNEXPECTED_TOKEN, "unknown token at string start, expected only '<'");
    }
  }

  // Секция внутри параметров: собираем параметры ожидаем конец параметров
  void parseLineSectionParams(const char* code) {
    switch (code[0]) {
    case '>': // '>P' - closing parameters section
      if (code[1] == 'P') {
        section = CODE_NO_SECTION;
        isParamsLoaded = true;
      }
      else {
        error(CPL_ERR_UNEXPECTED_TOKEN, "expecting 'P' token for closing parameters block", 1);
      }
      break;

    case '@': // next param defination line
      //Serial.print("PARAM: "); Serial.println(code);
      parseParameterLine(code);
      break;

    default:
      error(CPL_ERR_UNEXPECTED_TOKEN, "unknown token at parameters section, strings may be start only with '@' or '>'");
    }
  }

  // Парсим блоки контроллеров, до строки окончания секции блоков '>B'
  // блоки контроллеров между строками '{3' и '}'
  void parseLineSectionBlocks(const char* code) {
    switch (code[0]) {
    case '>': // '>B' - closing parameters section
      if (code[1] == 'B') {
        section = CODE_NO_SECTION;
        isBlocksLoaded = true;

        //debugln("BLOCK END");
        if (isParamsLoaded) {
          //debugln("PROG COMPLETE");
          isProgramComplete = true;
          program.programLoaded();
        }
      }
      else {
        error(CPL_ERR_UNEXPECTED_TOKEN, "expecting 'P' token for closing parameters block", 1);
      }
      break;

    case '{': // '{5' open new block line
    {
      section = CODE_SECTION_BLOCK;
      int blockControllersCount = atoi(code + 1);
      openBlock(blockControllersCount);
      Serial.print("BLOCK: "); Serial.print(blockControllersCount); Serial.println(" controllers");
    }
    break;

    default:
      error(CPL_ERR_UNEXPECTED_TOKEN, "unknown token in blocks, strings may be start only with '{' or '>'");
    }
  }

  // Парсим контроллеры блока, до строки окончания '}'
  void parseLineSectionBlock(const char* code) {
    switch (code[0]) {
    case '}': // '}' - closing block
      section = CODE_SECTION_BLOCKS;
      closeBlock();
      break;

    case '@': // controller
      //Serial.print("CTRL: "); Serial.println(code);
      parseControllerLine(code);
      break;

    default:
      error(CPL_ERR_UNEXPECTED_TOKEN, "unknown token in block, strings may be start only with '@' or '}'");
    }
  }

  // инициализируем память параметров
  void initParams(byte countParameters) {
    program.memory.init(countParameters);
  }

  // инициализируем память под блоки программы
  void initBlocks(byte countBlocks) {
    program.initBlocks(countBlocks);
  }

  // инициализируем память под контроллеры блока
  void openBlock(byte countControllers) {
    block = program.createBlock(countControllers);
  }

  void closeBlock() {
    block = nullptr;
  }

  // строка с кодом инициализации параметра
  void parseParameterLine(const char* code) {
    try {
      Parameter* param = parameterFactory.makeFromString(code);
      if (param != nullptr) {
        program.memory.addParam(param);
      }
      else {
        error(CPL_ERR_PARAM, "not recognized parameter");
      }
    }
    catch (const CompilationError& e) {
      error(CPL_ERR_PARAM, e.message());
    }
  }

  // строка с кодом инициализации контроллера
  void parseControllerLine(const char* code) {
    if (block == nullptr) {
      error(CPL_ERR_CTRL, "not exist block for new controller");
      return;
    }
    try {
      BaseController* ctrl = controllerFactory.makeFromString(code);
      if (ctrl != nullptr) {
        block->addController(ctrl);
      }
      else {
        error(CPL_ERR_CTRL, std::string("not recognized controller: ") + code);
      }
    }
    catch (const CompilationError& e) {
      error(CPL_ERR_CTRL, e.message());
    }
  }

};
