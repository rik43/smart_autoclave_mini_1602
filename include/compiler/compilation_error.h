#pragma once
#include <sstream>
#include <string>

enum CompilationErrorCode {
  CPL_ERR_NO = 0,
  CPL_ERR_SYNTAX = 1,
  CPL_ERR_PARAM = 2,
  CPL_ERR_CTRL = 3,
  CPL_ERR_INCOMPLETE_PROGRAM_RUN = 4,
  CPL_ERR_UNEXPECTED_TOKEN = 5,
  CPL_ERR_NOT_CONSISTENT_LINE_NUMBER = 6, // номер загруженной строки не совпадает с ожидаемым (пропуск/повтор)
};

class CompilationError : public std::exception {
private:
    std::string errorMessage;

public:
    CompilationError(const std::string& message) : errorMessage(message) {}
    CompilationError(const std::stringstream& message) : errorMessage(message.str()) {}

    const char* message() const noexcept {
        return errorMessage.c_str();
    }
};

