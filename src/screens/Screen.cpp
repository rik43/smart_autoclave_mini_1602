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
void Screen::gotoCustomRecipeScreen() { 
    screenManager->setScreen(&screenManager->customRecipeScreen); 
    screenManager->customRecipeScreen.reset();
}
void Screen::gotoCompleteScreen() { 
    screenManager->setScreen(&screenManager->completeScreen); 
    screenManager->completeScreen.reset();
}
void Screen::gotoStoppedScreen() { 
    screenManager->setScreen(&screenManager->stoppedScreen); 
    screenManager->stoppedScreen.reset();
}
void Screen::gotoErrorScreen(int errorCode) { 
    screenManager->setScreen(&screenManager->errorScreen); 
    screenManager->errorScreen.setErrorCode(errorCode);
    screenManager->errorScreen.reset();
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






