#pragma once

#include "Screen.h"
#include "ui/widget/Label.h"

// Перечисление для параметров редактирования
enum class EditParameter {
    TEMPERATURE,    // Температура
    TIME,           // Время
    CONTINUE,       // Продолжить
    CANCEL          // Отмена
};

// Перечисление для режима редактирования
enum class EditMode {
    SELECT_PARAM,   // Выбор параметра
    EDIT_VALUE      // Редактирование значения
};

class CustomRecipeScreen : public Screen {    
    private:
        Label labelParamName;    // Название параметра (первая строка)
        Label labelParamValue;   // Значение параметра (вторая строка)
        Label cursor;            // Курсор для навигации
        
        EditParameter currentParam = EditParameter::TEMPERATURE;
        EditMode currentMode = EditMode::SELECT_PARAM;
        
        int tempTemperature;     // Временные значения для редактирования
        int tempTime;
        
        // Ограничения для параметров
        const int MIN_TEMPERATURE = 20;
        const int MAX_TEMPERATURE = 123;
        const int MIN_TIME = 1;
        const int MAX_TIME = 120;

    public:
        CustomRecipeScreen() :
            labelParamName(2, 0, ""),
            labelParamValue(2, 1, ""),
            cursor(0, 0, "~", 1)
        {
            labelParamName.setScrollable(true);
            labelParamName.setScrollSpeed(950);
            labelParamName.setScrollBy(10);
            
            //labelParamValue.setAlignment(ALIGN_RIGHT);
            
            cursor.setBlink(true);
            cursor.setBlinkDelay(300, 300);
        }

        ~CustomRecipeScreen() {
        }

        void draw(BufferedLcd &lcd) override {
            labelParamName.print(lcd);
            labelParamValue.print(lcd);
            cursor.print(lcd);
        }
        
        void update() override {
            updateDisplay();
        }

        void reset() override {
            // Копируем текущие значения из ViewContext
            tempTemperature = viewContext->getCustomTemperature();
            tempTime = viewContext->getCustomTimeMin();
            
            // Если значения равны 0, устанавливаем значения по умолчанию
            if (tempTemperature == 0) {
                tempTemperature = 80; // Значение по умолчанию
            }
            if (tempTime == 0) {
                tempTime = 60; // Значение по умолчанию
            }
            
            currentParam = EditParameter::TEMPERATURE;
            currentMode = EditMode::SELECT_PARAM;
            
            updateDisplay();
        }

        // Обновляет отображение в зависимости от текущего состояния
        void updateDisplay() {
            switch (currentMode) {
                case EditMode::SELECT_PARAM:
                    updateParamSelectionDisplay();
                    break;
                case EditMode::EDIT_VALUE:
                    updateValueEditDisplay();
                    break;
            }
        }

        // Обновляет отображение при выборе параметра
        void updateParamSelectionDisplay() {
            cursor.setPosition(0, 0);
            
            switch (currentParam) {
                case EditParameter::TEMPERATURE:
                    labelParamName.setText("Температура, С");
                    labelParamValue.setText(tempTemperature);
                    break;
                case EditParameter::TIME:
                    labelParamName.setText("Время, мин");
                    labelParamValue.setText(tempTime);
                    break;
                case EditParameter::CONTINUE:
                    labelParamName.setText("Продолжить");
                    labelParamValue.setText("");
                    break;
                case EditParameter::CANCEL:
                    labelParamName.setText("Отмена");
                    labelParamValue.setText("");
                    break;
            }
        }

        // Обновляет отображение при редактировании значения
        void updateValueEditDisplay() {
            cursor.setPosition(0, 1);
            
            switch (currentParam) {
                case EditParameter::TEMPERATURE:
                    labelParamName.setText("Температура, С");
                    labelParamValue.setText(tempTemperature);
                    break;
                case EditParameter::TIME:
                    labelParamName.setText("Время, мин");
                    labelParamValue.setText(tempTime);
                    break;
                default:
                    // Не должно происходить
                    break;
            }
        }

        // Переключает на следующий параметр
        void nextParameter() {
            switch (currentParam) {
                case EditParameter::TEMPERATURE:
                    currentParam = EditParameter::TIME;
                    break;
                case EditParameter::TIME:
                    currentParam = EditParameter::CONTINUE;
                    break;
                case EditParameter::CONTINUE:
                    currentParam = EditParameter::CANCEL;
                    break;
                case EditParameter::CANCEL:
                    currentParam = EditParameter::TEMPERATURE;
                    break;
            }
        }

        // Переключает на предыдущий параметр
        void previousParameter() {
            switch (currentParam) {
                case EditParameter::TEMPERATURE:
                    currentParam = EditParameter::CANCEL;
                    break;
                case EditParameter::TIME:
                    currentParam = EditParameter::TEMPERATURE;
                    break;
                case EditParameter::CONTINUE:
                    currentParam = EditParameter::TIME;
                    break;
                case EditParameter::CANCEL:
                    currentParam = EditParameter::CONTINUE;
                    break;
            }
        }

        // Изменяет значение текущего параметра
        void changeValue(bool isRight, bool isFast) {
            int delta = isRight ? 1 : -1;
            if (isFast) {
                delta *= 5;
            }
            
            switch (currentParam) {
                case EditParameter::TEMPERATURE:
                    tempTemperature += delta;
                    if (tempTemperature < MIN_TEMPERATURE) tempTemperature = MIN_TEMPERATURE;
                    if (tempTemperature > MAX_TEMPERATURE) tempTemperature = MAX_TEMPERATURE;
                    break;
                case EditParameter::TIME:
                    tempTime += delta;
                    if (tempTime < MIN_TIME) tempTime = MIN_TIME;
                    if (tempTime > MAX_TIME) tempTime = MAX_TIME;
                    break;
                default:
                    break;
            }
        }

        void onEncoderTurn2(bool isRight, bool isFast) {
            if (currentMode == EditMode::SELECT_PARAM) {
                if (isRight) {
                    nextParameter();
                } else {
                    previousParameter();
                }
            } else if (currentMode == EditMode::EDIT_VALUE) {
                changeValue(isRight, isFast);
            }
        }

        void onEncoderButtonClick() override {
            if (currentMode == EditMode::SELECT_PARAM) {
                if (currentParam == EditParameter::TEMPERATURE || currentParam == EditParameter::TIME) {
                    // Переходим в режим редактирования значения
                    currentMode = EditMode::EDIT_VALUE;
                } else if (currentParam == EditParameter::CONTINUE) {
                    // Сохраняем изменения и возвращаемся к предварительному просмотру
                    saveChanges();
                    gotoRecipePreviewScreen();
                } else if (currentParam == EditParameter::CANCEL) {
                    // Отменяем изменения и возвращаемся к списку рецептов
                    gotoRecipesScreen(false);
                }
            } else if (currentMode == EditMode::EDIT_VALUE) {
                // Завершаем редактирование значения и возвращаемся к выбору параметра
                currentMode = EditMode::SELECT_PARAM;
            }
        }

        // Сохраняет изменения в ViewContext
        void saveChanges() {
            viewContext->setCustomRecipe(tempTemperature, tempTime, true);
        }
};
