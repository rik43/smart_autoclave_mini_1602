#pragma once

#include "Screen.h"
#include "ui/widget/Label.h"

class StoppedScreen : public Screen {    
    private:
        Label labelStopped;

    public:
        StoppedScreen() :
            labelStopped(0, 0, "Остановлено")
        {
            labelStopped.setAlignment(ALIGN_CENTER);
        }

        ~StoppedScreen() {
        }

        void draw(BufferedLcd &lcd) override {
            labelStopped.print(lcd);
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
