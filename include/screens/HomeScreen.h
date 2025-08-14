#pragma once

#include "Screen.h"
#include "ui/widget/Label.h"


#define HOME_MENU_ITEMS_COUNT       2

#define HOME_MENU_ITEM_MENU         0
#define HOME_MENU_ITEM_SETTINGS     1


class HomeScreen : public Screen {
    private:
        Label labelTemperature;

        Label labelConnectionStatus;

        Label labelMenu;
        Label labelSettings;

        Label cursor;

        int activeMenuItem = 0; // номер активного пункта меню (позиция курсора)
        int minMenuItem = 0;
        int maxMenuItem = HOME_MENU_ITEMS_COUNT - 1;

    public:
        HomeScreen() :
            labelTemperature(0, 0, "", 5),
            labelConnectionStatus(10, 0, "BWC", 6),
            labelMenu(0, 1, "Меню", 4),
            labelSettings(7, 1, "Настройки", 9),
            cursor(5, 1, "~", 1) // char 0x7E (на дисплее - стрелка вправо)
        {
            labelConnectionStatus.setAlignment(ALIGN_RIGHT);

            cursor.setBlink(true);
            cursor.setBlinkDelay(250, 250);

            reset();
        }

        ~HomeScreen() {

        }

        void draw(BufferedLcd &lcd) override {
            labelTemperature.print(lcd);
            labelConnectionStatus.print(lcd);
            labelMenu.print(lcd);
            labelSettings.print(lcd);
            cursor.print(lcd);
        }
        
        void reset() override {
            activeMenuItem = 0;
            minMenuItem = 0;
            maxMenuItem = HOME_MENU_ITEMS_COUNT - 1;
            cursor.setText(char(0x7F));
        }

        void update1s() override {
            labelTemperature.setText(std::to_string(viewContext->getTemperature()) + "°C");
        }

        void onEncoderTurn2(bool isRight, bool isFast) override {
            if (isRight) {
                activeMenuItem = 1;
            } else {
                activeMenuItem = 0;
            }
            cursor.setText(activeMenuItem == 0 ? char(0x7F) : char(0x7E)); // <- or ->
        }

        void onEncoderButtonClick() override {
            switch (activeMenuItem) {
                case HOME_MENU_ITEM_MENU:
                    gotoMenuScreen();
                    break;
                case HOME_MENU_ITEM_SETTINGS:
                    gotoSettingsScreen();
                    break;
                default:
                    Serial.println("Unknown item: " + String(activeMenuItem));
                    break;
            }
        }

};
