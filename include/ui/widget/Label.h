#pragma once

#include <Arduino.h>
#include <string>
#include "timer.h"
#include "ui/VirtualLcd.h"

#define MAX_WIDTH 16

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2

#define SCROLL_PHASE_PRE_SCROLL 0   // фаза паузы перед скроллом
#define SCROLL_PHASE_SCROLL 1       // фаза скролла
#define SCROLL_PHASE_POST_SCROLL 2  // фаза паузы после скролла


/**
 * Класс для отображения текста на дисплее LCD1602.
 * Label всегда занимает 1 строку.
 * Если текст не помещается в ширину, то он может скроллиться если scrollable = true.
 */
class Label {

    private:
        Timer *blinkTimer = nullptr;
        Timer *scrollTimer = nullptr;
        Timer *scrollWaitTimer = nullptr;

        uint8_t x;
        uint8_t y;
        std::string text;
        uint8_t alignment = ALIGN_LEFT;
        uint8_t textWidth = 0; // ширина в ЮНИКОД-символах (UTF-8), не в байтах
        uint8_t width = 16;
        bool visible = true;
        bool scrollable = false;
        uint16_t scrollSpeed = 400;
        uint8_t scrollPosition = 0;
        uint8_t scrollBy = 1; // скролить на количество символов за один тик
        uint8_t scrollPhase = SCROLL_PHASE_PRE_SCROLL;
        bool blink = false;
        uint16_t blinkShowDelay = 500; // ms, задержка показа
        uint16_t blinkHideDelay = 500; // ms, задержка скрытия
        uint8_t blinkCount = 0; // счетчик
        uint8_t blinkCountLimit = 0; // сколько раз мигать (0 - бесконечно, иначе - количество раз)
        bool blinkState = false; // состояние (0 - скрыто, 1 - показано)

    public:
        Label(int x, int y, std::string text) { 
            this->x = x; 
            this->y = y; 
            this->text = text; 
            this->textWidth = utf8Length(text);
            this->width = MAX_WIDTH - x;
        }
        Label(int x, int y, std::string text, int width) 
            : Label(x, y, text) { 
            this->setWidth(width);
        }
        ~Label() {
            if (blinkTimer != nullptr) {
                delete blinkTimer;
            }
            if (scrollTimer != nullptr) {
                delete scrollTimer;
            }
            if (scrollWaitTimer != nullptr) {
                delete scrollWaitTimer;
            }
        }

        void setText(std::string text) { 
            this->text = text; 
            this->textWidth = utf8Length(text);
        }
        void setText(const char* text) { 
            this->setText(std::string(text));
        }
        void setText(int text) { this->setText(std::to_string(text)); }
        void setText(uint32_t text) { this->setText(std::to_string(text)); }
        void setText(float text) { this->setText(std::to_string(text)); }
        void setText(double text) { this->setText(std::to_string(text)); }
        void setText(bool text) { this->setText(std::to_string(text)); }
        void setText(char text) { this->setText(std::string(1, text)); }

        void setPosition(uint8_t x, uint8_t y) { 
            this->x = x; 
            this->y = y; 
            this->updateWidth();
        }
        void setAlignment(uint8_t alignment) { 
            this->alignment = alignment;
            this->updateWidth();
        }
        void setWidth(uint8_t width) { 
            this->width = width; 
            this->updateWidth();
        }
        void updateWidth() {
            this->width = min(this->width, static_cast<uint8_t>(MAX_WIDTH - x));
        }
        void setVisible(bool visible) { 
            this->visible = visible; 
        }
        void setScrollable(bool scrollable) { 
            this->scrollable = scrollable; 
            if (scrollable) {
                if (this->scrollTimer == nullptr) {
                    this->scrollTimer = new Timer(scrollSpeed);
                }
                if (this->scrollWaitTimer == nullptr) {
                    this->scrollWaitTimer = new Timer(1500);
                }
            }
            // не удаляем таймеры, если они существуют, чтобы потом не создавать их заново
        }
        void setScrollSpeed(uint16_t scrollSpeed) { 
            this->scrollSpeed = scrollSpeed; 
            if (this->scrollTimer != nullptr) {
                this->scrollTimer->setPeriod(this->scrollSpeed);
            }
        }
        void setScrollBy(uint8_t scrollBy) { 
            this->scrollBy = scrollBy; 
        }
        void resetScroll() {
            this->scrollPhase = SCROLL_PHASE_PRE_SCROLL;
            this->scrollPosition = 0;
        }
        void setBlink(bool blink) { 
            this->blink = blink; 
            if (blink) {
                if (this->blinkTimer == nullptr) {
                    this->blinkTimer = new Timer(blinkShowDelay);
                }
            }
            // не удаляем таймер, если он существует, чтобы потом не создавать их заново
        }
        // задать одинаковое значение для показа и скрытия
        void setBlinkDelay(uint16_t blinkDelay) { 
            this->blinkShowDelay = blinkDelay; 
            this->blinkHideDelay = blinkDelay; 
        }
        // задать разные значения для показа и скрытия
        void setBlinkDelay(uint16_t blinkShowDelay, uint16_t blinkHideDelay) { 
            this->blinkShowDelay = blinkShowDelay; 
            this->blinkHideDelay = blinkHideDelay; 
        }
        // задать количество миганий, 0 - бесконечно, иначе - количество раз. после достижения лимита, мигание останавливается.
        void setBlinkCountLimit(uint8_t blinkCountLimit) { 
            this->blinkCountLimit = blinkCountLimit; 
        }

        uint8_t getX() { return x; }
        uint8_t getY() { return y; }
        uint8_t getScrollable() { return scrollable; }

        // Подсчёт длины строки в UTF-8 кодовых точках
        static int utf8Length(const std::string &s) {
            int count = 0;
            const unsigned char* bytes = reinterpret_cast<const unsigned char*>(s.data());
            size_t i = 0;
            while (i < s.size()) {
                // начало символа, если не байт-продолжение (10xxxxxx)
                if ((bytes[i] & 0xC0) != 0x80) {
                    count++;
                }
                i++;
            }
            return count;
        }

        // Безопасный срез UTF-8 по символам: [charStart, charStart+charLen)
        static std::string utf8Substr(const std::string &s, int charStart, int charLen) {
            if (charStart < 0) charStart = 0;
            if (charLen < 0) charLen = 0;
            const unsigned char* bytes = reinterpret_cast<const unsigned char*>(s.data());
            const size_t n = s.size();

            auto byteIndexOfChar = [&](int targetCharIndex) -> size_t {
                if (targetCharIndex <= 0) return 0;
                int curChar = 0;
                size_t i = 0;
                while (i < n && curChar < targetCharIndex) {
                    if ((bytes[i] & 0xC0) != 0x80) {
                        // встретили начало символа → двигаемся к началу следующего символа
                        i++;
                        while (i < n && (bytes[i] & 0xC0) == 0x80) i++;
                        curChar++;
                    } else {
                        // внутри многобайтного символа, просто идём вперёд
                        i++;
                    }
                }
                return i;
            };

            size_t byteStart = byteIndexOfChar(charStart);
            if (byteStart >= n) return std::string();
            size_t byteEnd = byteIndexOfChar(charStart + charLen);
            if (byteEnd > n) byteEnd = n;
            if (byteEnd < byteStart) byteEnd = byteStart;
            return s.substr(byteStart, byteEnd - byteStart);
        }

        // Оставляем только ASCII и валидные двухбайтовые последовательности UTF-8 (кириллица)
        static std::string normalizeForLCD(const std::string &s) {
            std::string out;
            out.reserve(s.size());
            const unsigned char* b = reinterpret_cast<const unsigned char*>(s.data());
            size_t i = 0, n = s.size();
            while (i < n) {
                if (b[i] < 0x80) {
                    // ASCII
                    out.push_back(static_cast<char>(b[i]));
                    i++;
                } else if ((b[i] & 0xE0) == 0xC0 && (i + 1 < n) && (b[i+1] & 0xC0) == 0x80) {
                    // 2-байтовый UTF-8 (кириллица и пр.)
                    out.push_back(static_cast<char>(b[i]));
                    out.push_back(static_cast<char>(b[i+1]));
                    i += 2;
                } else {
                    // 3+ байт или битые последовательности — заменим пробелом
                    out.push_back(' ');
                    // пропустить хвост многобайтовой последовательности
                    i++;
                    while (i < n && (b[i] & 0xC0) == 0x80) i++;
                }
            }
            return out;
        }

        
        // получить текст для вывода на дисплей
        // модуль дисплея будет вызывать этот метод каждый кадр и выводить указанный текст по координатам x, y
        // важно выдавать полностью весь текст, чтобы заполнять пробелами если текста стало меньше
        std::string getOutputText() { 
            std::string outputText = "";
            if (visible) {
                outputText = text;

                if (blink) {
                    if (blinkTimer->ready()) {
                        blinkState = !blinkState;
                    }
                    if (blinkState) {
                        outputText = text; 
                    } else {
                        outputText = "";
                    }
                }

                // если текст слишком широкий, то обрезать его до ширины (по символам UTF-8)
                if (textWidth > width) {
                    // если текст скроллится, то выводить только часть текста по указанной ширине
                    if (scrollable) {
                        switch (scrollPhase) {
                            case SCROLL_PHASE_PRE_SCROLL: // пауза перед скроллом, чтобы прочитать начало текста
                                scrollPosition = 0;
                                if (scrollWaitTimer->ready()) {
                                    scrollPhase = SCROLL_PHASE_SCROLL;
                                    scrollTimer->reset();
                                }
                                break;
                            case SCROLL_PHASE_SCROLL: // скролл
                                if (scrollTimer->ready()) {
                                    scrollPosition += scrollBy;
                                    if (scrollPosition > textWidth - width) {
                                        scrollPhase = SCROLL_PHASE_POST_SCROLL;
                                        scrollWaitTimer->reset();
                                    }
                                }
                                break;
                            case SCROLL_PHASE_POST_SCROLL: // пауза после скролла, чтобы прочитать конец текста
                                if (scrollWaitTimer->ready()) {
                                    scrollPhase = SCROLL_PHASE_PRE_SCROLL;
                                }
                                break;
                            default:
                                break;
                        }

                        // ограничиваем и зацикливаем позицию прокрутки (в символах)
                        int maxStart = textWidth - width;
                        if (maxStart < 0) { maxStart = 0; }
                        if (scrollPosition > maxStart) { 
                            scrollPosition = maxStart;
                        }
                    }
                    // безопасный срез по символам UTF-8
                    int startChar = scrollable ? scrollPosition : 0;
                    if (startChar < 0) startChar = 0;
                    outputText = utf8Substr(text, startChar, width);
                }
            }

            // всегда дополнять пробелами до ширины с нужной стороны
            int displayChars = utf8Length(outputText);
            if (displayChars < width) {
                if (alignment == ALIGN_LEFT) {
                    outputText = outputText + std::string(width - displayChars, ' ');
                } else if (alignment == ALIGN_CENTER) {
                    int spaces = width - displayChars;
                    int leftPadding = spaces / 2;
                    int rightPadding = spaces - leftPadding;
                    outputText = std::string(leftPadding, ' ') + outputText + std::string(rightPadding, ' ');
                } else if (alignment == ALIGN_RIGHT) {
                    outputText = std::string(width - displayChars, ' ') + outputText;
                }
            }

            return outputText;
        }

        void print(BufferedLcd &lcd) {
            std::string out = normalizeForLCD(getOutputText());
            lcd.setCursor(static_cast<uint8_t>(x), static_cast<uint8_t>(y));
            lcd.print(out.c_str());
        }




};

