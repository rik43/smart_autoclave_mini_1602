#pragma once

#include "Screen.h"
#include "ui/widget/Label.h"

class CompleteScreen : public Screen {    
    private:
        Label labelComplete;

    public:
        CompleteScreen() :
            labelComplete(0, 0, "Готово")
        {
            labelComplete.setAlignment(ALIGN_CENTER);
        }

        ~CompleteScreen() {
        }

        void draw(BufferedLcd &lcd) override {
            labelComplete.print(lcd);
        }
        
        void update() override {
        }

        void reset() override {
            // Ничего не нужно делать при сбросе
        }

        void onEncoderButtonClick() override {
            // При нажатии на энкодер возвращаемся на главный экран
            gotoHomeScreen();
        }
};
