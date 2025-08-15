#pragma once

#include "global.h"
#include "Screen.h"
#include "ui/widget/Label.h"


class StopProcessConfirmScreen : public Screen {    
    private:
        Label labelQuestion;
        Label labelOptions;
        Label cursor;
        bool isYesSelected = false;

        const uint8_t cursorXNo = 0;
        const uint8_t cursorXYes = 8; // визуально между "Нет" и "Да"

    public:
        StopProcessConfirmScreen() :
            labelQuestion(0, 0, "Остановить процесс?"),
            labelOptions(1, 1, "Нет     Да"), // по умолчанию выбран "Нет" чтобы не было ошибки при переходе с экрана предпросмотра (случайный двойной клик)
            cursor(0, 1, "~", 1)
        {
            cursor.setBlink(true);
            cursor.setBlinkDelay(350, 350);
        }

        ~StopProcessConfirmScreen() {

        }

        void reset() override {
            isYesSelected = false;
            cursor.setPosition(cursorXNo, 1);
        }

        void update() override {
        }

        void draw(BufferedLcd &lcd) override {
            labelQuestion.print(lcd);
            labelOptions.print(lcd);
            cursor.print(lcd);
        }

        void onEncoderTurn2(bool isRight, bool isFast) override {
            if (isRight) {
                isYesSelected = true;
                cursor.setPosition(cursorXYes, 1);
            } else {
                isYesSelected = false;
                cursor.setPosition(cursorXNo, 1);
            }
        }

        void onEncoderButtonClick() override {
            if (isYesSelected) {
                stopAutoclaveProcess();
                //gotoHomeScreen();
            } else {
                gotoAutoclaveProcessScreen(); // вернуться к процессу
            }
        }
};
 

