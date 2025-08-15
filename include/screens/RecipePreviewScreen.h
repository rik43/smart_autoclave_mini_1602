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
        Label labelAction;
        int recipeId;
        
        // Перечисление для действий
        enum class Action {
            NEXT,    // далее
            BACK,    // назад  
            EDIT     // измен
        };
        
        Action currentAction = Action::NEXT;

    public:
        RecipePreviewScreen() :
            labelName(0, 0, "", 10),
            labelTemperature(0, 1, "", 3),
            labelTemperatureUnit(3, 1, "°C", 2),
            labelTime(8, 1, "", 3),
            labelTimeUnit(11, 1, "мин", 3),
            labelAction(11, 0, "далее", 5)
        {
            labelName.setScrollable(true);
            labelName.setScrollSpeed(950);
            labelName.setScrollBy(8);

            labelAction.setBlink(true);
            labelAction.setBlinkDelay(300, 300);
        }

        ~RecipePreviewScreen() {

        }

        void draw(BufferedLcd &lcd) override {
            labelName.print(lcd);
            labelTemperature.print(lcd);
            labelTemperatureUnit.print(lcd);
            labelTime.print(lcd);
            labelTimeUnit.print(lcd);
            labelAction.print(lcd);
        }
        
        void update() override {
        }

        void reset() override {
            currentAction = Action::NEXT;
            updateActionLabel();

            labelName.setText(viewContext->getRecipeName());
            labelTemperature.setText(viewContext->getCustomTemperature());
            labelTime.setText(viewContext->getCustomTimeMin());
        }

        // Обновляет текст действия в зависимости от текущего состояния
        void updateActionLabel() {
            switch (currentAction) {
                case Action::NEXT:
                    labelAction.setText("далее");
                    break;
                case Action::BACK:
                    labelAction.setText("назад");
                    break;
                case Action::EDIT:
                    labelAction.setText("измен");
                    break;
            }
        }

        // Переключает на следующее действие по кругу
        void nextAction() {
            switch (currentAction) {
                case Action::NEXT:
                    currentAction = Action::BACK;
                    break;
                case Action::BACK:
                    currentAction = Action::EDIT;
                    break;
                case Action::EDIT:
                    currentAction = Action::NEXT;
                    break;
            }
            updateActionLabel();
        }

        // Переключает на предыдущее действие по кругу
        void previousAction() {
            switch (currentAction) {
                case Action::NEXT:
                    currentAction = Action::EDIT;
                    break;
                case Action::BACK:
                    currentAction = Action::NEXT;
                    break;
                case Action::EDIT:
                    currentAction = Action::BACK;
                    break;
            }
            updateActionLabel();
        }

        void onEncoderTurn2(bool isRight, bool isFast) {
            if (isRight) {
                nextAction();
            } else {
                previousAction();
            }
        }

        void onEncoderButtonClick() override {
            switch (currentAction) {
                case Action::NEXT:
                    gotoRecipeConfirmScreen();
                    break;
                case Action::BACK:
                    gotoRecipesScreen(false);
                    break;
                case Action::EDIT:
                    // Копируем параметры текущего рецепта в ViewContext для редактирования
                    // viewContext->setCustomTemperature(viewContext->getCustomTemperature());
                    // viewContext->setCustomTimeMin(viewContext->getCustomTimeMin());
                    // Переходим к экрану редактирования рецепта
                    gotoCustomRecipeScreen();
                    break;
            }
        }
};
