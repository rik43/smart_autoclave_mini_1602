#pragma once

#include "const.h"
#include <EncButton.h>
#include "ui/widget/Label.h"
#include "ui/InputEvent.h"
#include "ui/VirtualLcd.h"
#include "ui/ViewContext.h"


class ScreenManager; // forward declaration

/**
 * Абстрактный класс для страниц, которые будут отображаться на экране.
 * 
 * Каждая страница должна иметь методы update и draw, которые будут вызываться
 * для обновления (считывание кнопок и энкодера и др органов управления) и отрисовки страницы на экране.
 */

class Screen {
    protected:
        ScreenManager* screenManager = nullptr;
        ViewContext* viewContext = nullptr; // указатель на контекст отображения

    public:
        Screen() = default;
        virtual ~Screen() = default;
        virtual void update() {};
        virtual void update1s() {};
        virtual void reset() {};
        virtual void draw(BufferedLcd &lcd) {}; // отрисовка в абстрактный буфер
        virtual void onEnter() {}; // вызывается при переходе на экран
        virtual void onExit() {}; // вызывается при выходе с экрана
        virtual void onBlur() {}; // вызывается при снятии фокуса с экрана (например, при переходе на другой экран "поверх" текущего, то есть текущий экран не выключается, но не получает фокус)
        virtual void onFocus() {}; // вызывается при фокусе на экране
        virtual bool onBack() { return true; }; // вызывается при нажатии на кнопку "Назад", возвращает true, если экран должен быть закрыт, false - если экран сам обрабатывает нажатие на кнопку "Назад"

        void attach(ScreenManager* mgr, ViewContext& viewContext) { 
            screenManager = mgr; 
            this->viewContext = &viewContext; 
        }
        ScreenManager* getScreenManager() { return screenManager; }
        ViewContext* getViewContext() { return viewContext; }

        virtual void handleEvent(const InputEvent& e) {
            switch (e.device) {
                case InputEventDevice::Encoder:
                    onEncoderTurn(e.data.encoder.isRight, e.data.encoder.isFast);
                    break;
                case InputEventDevice::EncoderButton:
                    onEncoderButtonClick();
                    break;
                case InputEventDevice::System:
                    break;
            }
        };

        virtual void onEncoderTurn(bool isRight, bool isFast) {};
        virtual void onEncoderTurn2(bool isRight, bool isFast) {}; // вращение на 2 деления энкодера
        virtual void onEncoderButtonClick() {};
        virtual void onEncoderButtonLongPress() {};

        void gotoHomeScreen();
        void gotoMenuScreen(bool reset = true);
        void gotoSettingsScreen();
        void gotoRecipesScreen(bool reset = true);
        void gotoRecipePreviewScreen();
        void gotoRecipeConfirmScreen();
        void gotoCustomRecipeScreen();
        void gotoCompleteScreen();
        void gotoStoppedScreen();
        void gotoErrorScreen(int errorCode = 1);
		void gotoAutoclaveProcessScreen();
        void gotoStopProcessConfirmScreen();
        void gotoTestScreen();
        void gotoUpdateFirmwareScreen();
};