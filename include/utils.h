#pragma once
#include <string>
#include <functional>

void parseQueryString(const char* queryString, std::function<void(const std::string&, const std::string&)> callback);

uint16_t combineToUint16(uint8_t highByte, uint8_t lowByte);

std::string float2string(float floatValue, int decimals);
