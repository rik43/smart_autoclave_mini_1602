#pragma once

#include "Screen.h"
#include "ui/widget/Label.h"


#define MAIN_MENU_ITEMS_COUNT       5

#define MAIN_MENU_ITEM_AUTOCLAVE    1
#define MAIN_MENU_ITEM_SU_VIDE      2
#define MAIN_MENU_ITEM_DISTILLATOR  3
#define MAIN_MENU_ITEM_BACK     4


class MenuScreen : public Screen {
    private:
        Label line1;
        Label line2;
        Label cursor;

        int cursorPositionY = 1; // изначально курсор на второй строке
        int scrollOffset = 0; // смещение при скролле
        int activeMenuItem = 1; // номер активного пункта меню (позиция курсора)
        int minMenuItem = 1;
        int maxMenuItem = MAIN_MENU_ITEMS_COUNT - 1;

        const char* menuItems[MAIN_MENU_ITEMS_COUNT] = {
            "Режимы:", // нельзя выбрать
            "Автоклав",
            "Су-вид",
            "Дистиллятор",
            "Назад",
        };

    public:
        MenuScreen() :
            line1(2, 0, ""),
            line2(2, 1, ""),
            cursor(0, 1, "~", 1) // char 0x7E (на дисплее - стрелка вправо)
        {
            line1.setScrollable(true);
            line1.setScrollSpeed(950);
            line1.setScrollBy(10);
            line1.setText(menuItems[0]);

            line2.setScrollable(true);
            line2.setScrollSpeed(950);
            line2.setScrollBy(10);
            line2.setText(menuItems[1]);

            cursor.setBlink(true);
            cursor.setBlinkDelay(350, 350);
        }

        ~MenuScreen() {

        }

        void draw(BufferedLcd &lcd) override {
            line1.print(lcd);
            line2.print(lcd);
            cursor.print(lcd);
        }
        
        void reset() override {
            cursorPositionY = 1;
            scrollOffset = 0;
            activeMenuItem = 1;
            minMenuItem = 1;
            maxMenuItem = MAIN_MENU_ITEMS_COUNT - 1;
            line1.setText(menuItems[0]);
            line2.setText(menuItems[1]);
            cursor.setPosition(0, cursorPositionY);
        }

        void update() override {
        }

        void onEncoderTurn2(bool isRight, bool isFast) override {
            bool isNext = isRight;
            if (isNext) { 
                // листаем вниз
                if (activeMenuItem < maxMenuItem) {
                    activeMenuItem++; // выбираем следующий пункт
                }

                if (cursorPositionY == 0) { // если курсор на первой строке, то выбираем следующий пункт (видимый во второй строке), без скрола страницы
                    cursorPositionY = 1; // курсор на второй строке при листании вниз
                } else {
                    scrollOffset++; // скроллим страницу
                }
            } else {
                // листаем вверх
                if (activeMenuItem > minMenuItem) {
                    activeMenuItem--; // выбираем предыдущий пункт
                }

                if (activeMenuItem == minMenuItem) {
                    if (scrollOffset > 0) {
                        scrollOffset--;
                        if (scrollOffset == 0) {
                            cursorPositionY = 1;
                        }
                    }
                } else {
                    if (cursorPositionY == 1) { // если курсор на второй строке, то выбираем предыдущий пункт (видимый в первой строке), без скрола страницы    
                        cursorPositionY = 0; // курсор на первой строке при листании вверх
                    } else {
                        scrollOffset--; // скроллим страницу
                    }
                }
            }
            if (scrollOffset < 0) {
                scrollOffset = 0;
            }
            if (scrollOffset > maxMenuItem - 1) {
                scrollOffset = maxMenuItem - 1;
            }
            line1.setText(menuItems[scrollOffset]);
            line2.setText(menuItems[scrollOffset + 1]);
            cursor.setPosition(0, cursorPositionY);
        }

        void onEncoderButtonClick() override {
            switch (activeMenuItem) {
                case MAIN_MENU_ITEM_AUTOCLAVE:
                    gotoRecipesScreen();
                    break;
                // case MAIN_MENU_ITEM_SU_VIDE:
                //     screenManager->switchTo(new SuViewScreen());
                //     break;
                // case MAIN_MENU_ITEM_DISTILLATOR:
                //     screenManager->switchTo(new DistillatorScreen());
                //     break;
                // case MAIN_MENU_ITEM_SETTINGS:
                //     gotoSettingsScreen();
                //     break;
                case MAIN_MENU_ITEM_BACK:
                    gotoHomeScreen();
                    break;
                default:
                    Serial.println("Unknown item: " + String(activeMenuItem));
                    break;
            }
        }

};
