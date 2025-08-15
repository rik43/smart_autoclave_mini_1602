#pragma once

#include <string>
#include "data/recipes.h"


#define RECIPE_MODE_AUTOCLAVE       0
#define RECIPE_MODE_SU_VID          1

class ViewContext {
    private:
        // Параметры текущего режима работы
        int temperature = 0; // температура с датчика в градусах Цельсия
        int power = 0; // мощность ТЭН в процентах
        int timeMinutes = 0; // время в минутах (осталось либо прошло по программе)

        // Параметры текущего рецепта
        int recipeId = 0; // id рецепта
        std::string recipeName = ""; // название рецепта
        std::string recipeNameB64 = ""; // название рецепта в base64
        int customTemperature = 0; // температура в градусах Цельсия рецепта
        int customTimeMin = 0; // время в минутах рецепта
        byte recipeMode = RECIPE_MODE_AUTOCLAVE; // режим автоклав/су-вид

        // Статус соединения
        bool hasBluetooth = false;
        bool hasBluetoothClient = false;
        bool hasWifi = false;
        bool hasServer = false;

        // Параметры текущей работающей программы
        int stepNumber = 0; // номер текущего шага
        int stepCount = 0; // общее количество шагов
        int stepType = 0; // тип шага: нагрев/выдержка/охлаждение
        int timeType = 0; // тип времени: осталось/прошло
        uint32_t viewTimeMsec = 0; // время в миллисекундах для отображения на экране
        uint32_t viewTimeMinutes = 0; // время в минутах для отображения на экране

    public:
        ViewContext() {
        }

        void setRecipe(const Recipe& recipe) {
            recipeId = recipe.id;
            recipeName = recipe.name;
            recipeNameB64 = recipe.nameB64;
            customTemperature = recipe.cookingTemperature;
            customTimeMin = recipe.cookingTimeMinutes;
            recipeMode = RECIPE_MODE_AUTOCLAVE;
        }

        void setCustomRecipe(const int temperature, const int time, const bool isAutoclave) {
            customTemperature = temperature;
            customTimeMin = time;
            recipeMode = isAutoclave ? RECIPE_MODE_AUTOCLAVE : RECIPE_MODE_SU_VID;
            recipeId = 0;
            recipeName = "Свой";
            recipeNameB64 = "0KHQstC+0Lkg0YDQtdGG0LXQv9GC";
        }

        int getPower() const {
            return power;
        }

        void setPower(int power) {
            this->power = power;
        }

        int getTemperature() const {
            return temperature;
        }

        void setTemperature(int temperature) {
            this->temperature = temperature;
        }

        void setTimeMinutes(int timeMinutes) {
            this->timeMinutes = timeMinutes;
        }

        int getTimeMinutes() const {
            return timeMinutes;
        }

        bool getHasBluetooth() const {
            return hasBluetooth;
        }

        bool getHasBluetoothClient() const {
            return hasBluetoothClient;
        }

        bool getHasWifi() const {
            return hasWifi;
        }

        bool getHasServer() const {
            return hasServer;
        }

        void setHasBluetooth(bool hasBluetooth) {
            this->hasBluetooth = hasBluetooth;
        }

        void setHasBluetoothClient(bool hasBluetoothClient) {
            this->hasBluetoothClient = hasBluetoothClient;
        }

        void setHasWifi(bool hasWifi) {
            this->hasWifi = hasWifi;
        }

        void setHasServer(bool hasServer) {
            this->hasServer = hasServer;
        }

        int getRecipeId() const {
            return recipeId;
        }

        void setRecipeId(const int recipeId) {
            this->recipeId = recipeId;
        }

        std::string getRecipeMode() const {
            return recipeMode == RECIPE_MODE_AUTOCLAVE ? "autoclave" : "suvid";
        }

        void setRecipeMode(const int recipeMode) {
            this->recipeMode = recipeMode;
        }

        void setRecipeMode(const std::string& recipeMode) {
            this->recipeMode = recipeMode == "autoclave" ? RECIPE_MODE_AUTOCLAVE : RECIPE_MODE_SU_VID;
        }

        std::string getRecipeName() const {
            return recipeName;
        }

        void setRecipeName(const std::string& recipeName) {
            this->recipeName = recipeName;
        }

        std::string getRecipeNameB64() const {
            return recipeNameB64;
        }

        void setRecipeNameB64(const std::string& recipeNameB64) {
            this->recipeNameB64 = recipeNameB64;
        }

        int getCustomTemperature() const {
            return customTemperature;
        }

        void setCustomTemperature(const int customTemperature) {
            this->customTemperature = customTemperature;
        }

        int getCustomTimeMin() const {
            return customTimeMin;
        }

        void setCustomTimeMin(const int customTimeMin) {
            this->customTimeMin = customTimeMin;
        }

        int getStepNumber() const {
            return stepNumber;
        }

        void setStepNumber(const int stepNumber) {
            this->stepNumber = stepNumber;
        }

        int getStepCount() const {
            return stepCount;
        }

        void setStepCount(const int stepCount) {
            this->stepCount = stepCount;
        }

        int getStepType() const {
            return stepType;
        }

        void setStepType(const int stepType) {
            this->stepType = stepType;
        }

        int getTimeType() const {
            return timeType;
        }

        void setTimeType(const int timeType) {
            this->timeType = timeType;
        }

        uint32_t getViewTimeMsec() const {
            return viewTimeMsec;
        }

        void setViewTimeMsec(const uint32_t viewTimeMsec) {
            this->viewTimeMsec = viewTimeMsec;
            this->viewTimeMinutes = viewTimeMsec / 60 / 1000;
        }

        uint32_t getViewTimeMinutes() const {
            return viewTimeMinutes;
        }
};