#pragma once

#include "Screen.h"
#include "ui/widget/Label.h"


class TestScreen : public Screen {    
    private:
        Label labelHello;
        Label labelValue;
        int value = 0;

    public:
        TestScreen() :
            labelHello(0, 0, "Фабрика Заготовщика", 16),
            labelValue(0, 1, "0", 4)
        {
            labelHello.setScrollable(true);
            labelHello.setScrollSpeed(950);
            labelHello.setScrollBy(11);
            labelHello.setWidth(12);

            labelValue.setAlignment(ALIGN_RIGHT);
        }

        ~TestScreen() {

        }

        void reset() override {
            value = 0;
        }
        
        void draw(BufferedLcd &lcd) override {
            labelHello.print(lcd);
            labelValue.print(lcd);
        }

        void onEncoderTurn(bool isRight, bool isFast) {
            int delta = isFast ? 10 : 1;
            if (isRight) {
                value += delta;
            } else {
                value -= delta;
            }
        }

        void onEncoderButtonClick() override {
            gotoSettingsScreen();
        }

        void update() override {
            labelValue.setText(value);
        }

};
