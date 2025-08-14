#pragma once

#include <Arduino.h>
#include <string>
#include <vector>
#include <algorithm>
#include <LCD_1602_RUS_ALL.h>

// Базовый интерфейс для отрисовки. Совместим по API с LCD_1602_RUS (только нужные методы)
class ILcdGfx {
public:
    virtual ~ILcdGfx() = default;
    virtual void setCursor(uint8_t col, uint8_t row) = 0;
    virtual void print(const char* s) = 0;
    virtual void print(const wchar_t* ws) = 0;
    virtual uint8_t width() const = 0;
    virtual uint8_t height() const = 0;
};

// Буферизованный адаптер, накапливает кадр в wchar_t и выводит на LCD только отличия
class BufferedLcd : public ILcdGfx {
public:
    explicit BufferedLcd(uint8_t cols, uint8_t rows)
        : cols_(cols), rows_(rows), cursor_col_(0), cursor_row_(0) {
        last_.assign(rows_, std::wstring(cols_, L' '));
        curr_.assign(rows_, std::wstring(cols_, L' '));
    }

    void beginFrame() {
        for (auto& row : curr_) {
            std::fill(row.begin(), row.end(), L' ');
        }
        cursor_col_ = 0;
        cursor_row_ = 0;
    }

    void setCursor(uint8_t col, uint8_t row) override {
        cursor_col_ = col;
        cursor_row_ = row;
    }

    void print(const char* s) override {
        // Конвертация ASCII + 2-байт UTF-8 → wchar_t
        std::wstring ws;
        ws.reserve(strlen(s));
        const unsigned char* b = reinterpret_cast<const unsigned char*>(s);
        size_t i = 0, n = strlen(s);
        while (i < n) {
            if (b[i] < 0x80) {
                ws.push_back(static_cast<wchar_t>(b[i]));
                i++;
            } else if ((b[i] & 0xE0) == 0xC0 && (i + 1 < n) && (b[i+1] & 0xC0) == 0x80) {
                wchar_t cp = static_cast<wchar_t>(((b[i] & 0x1F) << 6) | (b[i+1] & 0x3F));
                ws.push_back(cp);
                i += 2;
            } else {
                // Игнорируем/заменяем неподдерживаемые последовательности
                ws.push_back(L' ');
                i++;
                while (i < n && (b[i] & 0xC0) == 0x80) i++;
            }
        }
        print(ws.c_str());
    }

    void print(const wchar_t* ws) override {
        if (cursor_row_ >= rows_) return;
        uint8_t x = cursor_col_;
        const wchar_t* p = ws;
        while (*p && x < cols_) {
            curr_[cursor_row_][x] = *p;
            ++x;
            ++p;
        }
        cursor_col_ = x;
    }

    uint8_t width() const override { return cols_; }
    uint8_t height() const override { return rows_; }

    // Обновляет весь экран, если есть хотя бы одно отличие в любом символе (конвертируя wchar_t → UTF-8 для LCD_1602_RUS::print(const char*))
    void flushTo(LCD_1602_RUS &lcd) {
        bool isChanged = false;
        for (uint8_t row = 0; row < rows_; ++row) {
            if (last_[row] != curr_[row]) {
                isChanged = true;
                break;
            }
        }
        //if (!isChanged) return;

        for (uint8_t row = 0; row < rows_; ++row) {
            lcd.setCursor(0, row);
            lcd.print(wstringToUtf8(curr_[row]).c_str());
        }
        // Обновить last
        last_ = curr_;
    }
    
    // Выводит только изменённые участки (конвертируя wchar_t → UTF-8 для LCD_1602_RUS::print(const char*))
    void flushToPartial(LCD_1602_RUS &lcd) {
        for (uint8_t row = 0; row < rows_; ++row) {
            const std::wstring& prev = last_[row];
            const std::wstring& now = curr_[row];

            uint8_t col = 0;
            while (col < cols_) {
                // найти начало изменения
                while (col < cols_ && prev[col] == now[col]) col++;
                if (col >= cols_) break;
                // найти конец непрерывного изменения
                uint8_t start = col;
                while (col < cols_ && prev[col] != now[col]) col++;
                uint8_t end = col; // [start, end)

                // Отправляем изменённый блок
                lcd.setCursor(start, row);
                std::wstring segment = now.substr(start, end - start);
                std::string utf8 = wstringToUtf8(segment);
                lcd.print(utf8.c_str());
            }
            // Обновить last
            last_[row] = now;
        }
    }

private:
    uint8_t cols_;
    uint8_t rows_;
    uint8_t cursor_col_;
    uint8_t cursor_row_;
    std::vector<std::wstring> last_;
    std::vector<std::wstring> curr_;

    static std::string wstringToUtf8(const std::wstring &ws) {
        std::string out;
        out.reserve(ws.size() * 2);
        for (wchar_t wc : ws) {
            uint32_t cp = static_cast<uint32_t>(wc);
            if (cp <= 0x7F) {
                out.push_back(static_cast<char>(cp));
            } else if (cp <= 0x7FF) {
                out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            } else if (cp <= 0xFFFF) {
                // В теории на ESP32 wchar_t = 16 бит, этот путь не должен часто срабатывать для >0x07FF
                out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            } else {
                // Замена неподдерживаемых/вне BMP
                out.push_back('?');
            }
        }
        out.push_back('\0');
        return out;
    }
};


