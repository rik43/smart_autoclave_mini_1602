#include "screens/Screen.h"
#include "ui/ScreenManager.h"

void Screen::gotoHomeScreen() { 
    Serial.println("gotoHomeScreen");
    screenManager->setScreen(&screenManager->homeScreen); 
    screenManager->homeScreen.reset();
}
void Screen::gotoMenuScreen(bool reset) { 
    Serial.println("gotoMenuScreen");
    screenManager->setScreen(&screenManager->menuScreen); 
    if (reset) {
        screenManager->menuScreen.reset();
    }
}
void Screen::gotoSettingsScreen() { 
    screenManager->setScreen(&screenManager->settingsScreen); 
    screenManager->settingsScreen.reset();
}
void Screen::gotoRecipesScreen(bool reset) { 
    screenManager->setScreen(&screenManager->recipesScreen); 
    if (reset) {
        screenManager->recipesScreen.reset();
    }
}
void Screen::gotoRecipePreviewScreen() { 
    screenManager->setScreen(&screenManager->recipePreviewScreen); 
    screenManager->recipePreviewScreen.reset();
}
void Screen::gotoRecipeConfirmScreen() { 
    screenManager->setScreen(&screenManager->recipeConfirmScreen); 
    screenManager->recipeConfirmScreen.reset();
}
void Screen::gotoAutoclaveProcessScreen() {
    screenManager->setScreen(&screenManager->autoclaveProcessScreen);
    screenManager->autoclaveProcessScreen.reset();
}
void Screen::gotoStopProcessConfirmScreen() {
    screenManager->setScreen(&screenManager->stopProcessConfirmScreen);
    screenManager->stopProcessConfirmScreen.reset();
}
void Screen::gotoTestScreen() { 
    screenManager->setScreen(&screenManager->testScreen); 
    screenManager->testScreen.reset();
}
void Screen::gotoUpdateFirmwareScreen() {
    screenManager->setScreen(&screenManager->updateFirmwareScreen); 
    screenManager->updateFirmwareScreen.reset();
}
// void gotoAutoclaveScreen() { getScreenManager()->setScreen(&getScreenManager()->autoclaveScreen); }
// void gotoSuViewScreen() { getScreenManager()->setScreen(&getScreenManager()->suViewScreen); }
// void gotoDistillatorScreen() { getScreenManager()->setScreen(&getScreenManager()->distillatorScreen); }








