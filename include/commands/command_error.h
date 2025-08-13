#pragma once
#include <string>

class CommandError : public std::exception {
private:
    std::string errorMessage;

public:
    CommandError(const std::string& message) : errorMessage(message) {}

    const char* what() const noexcept override {
        return errorMessage.c_str();
    }
};

