#pragma once

#include "screens/Screen.h"
#include "screens/HomeScreen.h"
#include "screens/MenuScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/RecipesScreen.h"
#include "screens/RecipePreviewScreen.h"
#include "screens/RecipeConfirmScreen.h"
#include "screens/AutoclaveProcessScreen.h"
#include "screens/UpdateFirmwareScreen.h"
#include "screens/StopProcessConfirmScreen.h"
#include "screens/TestScreen.h"
#include <LCD_1602_RUS_ALL.h>
#include "ui/VirtualLcd.h"
#include "Timer.h"

class ScreenManager {
    public:
        ViewContext& viewContext;
        Screen *currentScreen = nullptr;
        Timer updateTimer1000ms;

        HomeScreen homeScreen;
        MenuScreen menuScreen;
        SettingsScreen settingsScreen;
        RecipesScreen recipesScreen;
        RecipePreviewScreen recipePreviewScreen;
        RecipeConfirmScreen recipeConfirmScreen;
		AutoclaveProcessScreen autoclaveProcessScreen;
        StopProcessConfirmScreen stopProcessConfirmScreen;
        TestScreen testScreen;
        UpdateFirmwareScreen updateFirmwareScreen;  
        ScreenManager(ViewContext &viewContext):
            viewContext(viewContext),
            updateTimer1000ms(1000),
            homeScreen(),
            menuScreen(),
            settingsScreen(),
            recipesScreen(),
            recipePreviewScreen(),
            recipeConfirmScreen(),
			autoclaveProcessScreen(),
            stopProcessConfirmScreen(),
            testScreen(),
            updateFirmwareScreen()
        {
            setScreen(&homeScreen);
        }

        ~ScreenManager() = default;

        void setScreen(Screen *screen) {
            if (currentScreen == screen) {
                return;
            }

            if (currentScreen != nullptr) {
                currentScreen->onBlur();
                currentScreen->onExit();
            }

            currentScreen = screen;
            currentScreen->attach(this, viewContext);
            
            currentScreen->onEnter();
            currentScreen->onFocus();            
        }

        void update() {
            currentScreen->update();

            // вызываем update1s текущего экрана каждые 1 секунду
            // полезно для обновления данных на экране из глобального контекста
            if (updateTimer1000ms.ready()) {
                currentScreen->update1s();
            }
        }

        void draw(LCD_1602_RUS &lcd) {
            static BufferedLcd vbuf(LCD_WIDTH, LCD_HEIGHT);
            vbuf.beginFrame();
            currentScreen->draw(vbuf);
            vbuf.flushTo(lcd);
        }
};