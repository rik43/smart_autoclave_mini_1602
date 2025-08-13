#include "utils.h"

void parseQueryString(const char* queryString, std::function<void(const std::string&, const std::string&)> callback) {
    while (*queryString) {
        std::string param;
        std::string value;

        // Чтение имени параметра
        while (*queryString && *queryString != '=' && *queryString != '&') {
            // Учитываем экранирование амперсандов
            if (*queryString == '\\' && *(queryString + 1) == '&') {
                param += '&';
                queryString += 2;
            } else {
                param += *queryString;
                ++queryString;
            }
        }

        if (*queryString == '=') {
            ++queryString; // Пропускаем символ '='

            // Чтение значения параметра
            while (*queryString && *queryString != '&') {
                // Учитываем экранирование амперсандов
                if (*queryString == '\\' && *(queryString + 1) == '&') {
                    value += '&';
                    queryString += 2;
                } else {
                    value += *queryString;
                    ++queryString;
                }
            }

            // Вызываем колбэк для каждого параметра
            callback(param, value);
        }

        if (*queryString == '&') {
            ++queryString; // Пропускаем символ '&'
        }
    }
}

uint16_t combineToUint16(uint8_t highByte, uint8_t lowByte) {
    uint16_t combined = (static_cast<uint16_t>(highByte) << 8) | lowByte;
    return combined;
}


std::string float2string(float floatValue, int decimals) {
    // Определим максимальный размер буфера, учитывая целую часть, точку и десятичные знаки
    char buffer[20]; // Достаточно большой буфер для большинства значений float

    // Форматируем строку с нужной точностью
    snprintf(buffer, sizeof(buffer), "%.*f", decimals, floatValue);

    // Возвращаем результат в виде std::string
    return std::string(buffer);
}


/*
// работает но может быть тяжеловестной
std::string float2string(float floatValue, int decimals) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(decimals) << floatValue;
    std::string result = stream.str();
    return result;
}
*/
