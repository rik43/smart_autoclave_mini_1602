#pragma once

#include "Screen.h"
#include "ui/widget/Label.h"

class ErrorScreen : public Screen {    
    private:
        Label labelError;
        Label labelErrorCode;
        int errorCode;

    public:
        ErrorScreen() :
            labelError(0, 0, "Ошибка", 6),
            labelErrorCode(0, 1, "", 4),
            errorCode(1)
        {
            labelError.setAlignment(ALIGN_CENTER);
            labelErrorCode.setAlignment(ALIGN_CENTER);
        }

        ~ErrorScreen() {
        }

        void draw(BufferedLcd &lcd) override {
            labelError.print(lcd);
            labelErrorCode.print(lcd);
        }
        
        void update() override {
        }

        void reset() override {
            // Обновляем отображение кода ошибки
            labelErrorCode.setText("Код: " + std::to_string(errorCode));
        }

        void setErrorCode(int code) {
            errorCode = code;
        }

        void onEncoderButtonClick() override {
            // При нажатии на энкодер возвращаемся на главный экран
            gotoHomeScreen();
        }
};
