#pragma once

#include "Screen.h"
#include "ui/widget/Label.h"


class RecipePreviewScreen : public Screen {    
    private:
        Label labelName;
        Label labelTemperature;
        Label labelTemperatureUnit;
        Label labelTime;
        Label labelTimeUnit;
        Label labelNext;
        int recipeId;
        bool isNextCancel = false; // признак того, что кнопка "Далее" означает "Назад"

    public:
        RecipePreviewScreen() :
            labelName(0, 0, "", 10),
            labelTemperature(0, 1, "", 3),
            labelTemperatureUnit(3, 1, "°C", 2),
            labelTime(8, 1, "", 3),
            labelTimeUnit(11, 1, "мин", 3),
            labelNext(11, 0, "далее", 5)
        {
            labelName.setScrollable(true);
            labelName.setScrollSpeed(950);
            labelName.setScrollBy(8);

            labelNext.setBlink(true);
            labelNext.setBlinkDelay(300, 300);
        }

        ~RecipePreviewScreen() {

        }

        void draw(BufferedLcd &lcd) override {
            labelName.print(lcd);
            labelTemperature.print(lcd);
            labelTemperatureUnit.print(lcd);
            labelTime.print(lcd);
            labelTimeUnit.print(lcd);
            labelNext.print(lcd);
        }
        
        void update() override {
        }

        void reset() override {
            isNextCancel = false;
            labelNext.setText("далее");

            labelName.setText(viewContext->getRecipeName());
            labelTemperature.setText(viewContext->getCustomTemperature());
            labelTime.setText(viewContext->getCustomTimeMin());
        }

        // void setRecipeId(int recipeId) {
        //     this->recipeId = recipeId;
        //     const Recipe* recipe = getRecipe(recipeId);
            
        //     Serial.println("Recipe: " + String(recipeId));
        //     Serial.println("Recipe name: " + String(recipe->name.c_str()));
        //     Serial.println("Recipe cooking temperature: " + String(recipe->cookingTemperature));
        //     Serial.println("Recipe cooking time: " + String(recipe->cookingTimeMinutes));

        //     labelName.setText(recipe->name);
        //     labelTemperature.setText(recipe->cookingTemperature);
        //     labelTime.setText(recipe->cookingTimeMinutes);
        // }

        void onEncoderTurn2(bool isRight, bool isFast) {
            if (isRight) {
                isNextCancel = true;
                labelNext.setText("назад");
            } else {
                isNextCancel = false;
                labelNext.setText("далее");
            }
        }

        void onEncoderButtonClick() override {
            if (isNextCancel) {
                gotoRecipesScreen(false);
            } else {
                gotoRecipeConfirmScreen();
            }
        }
};
