#pragma once
#include <string>
#include <stdint.h>
#include <cstddef>

struct Recipe {
    uint16_t id;
    uint16_t pageNumber; // страница с этим рецептом для "назад" из него
    uint16_t cookingTemperature;
    uint16_t cookingTimeMinutes;
    std::string name;
    std::string nameB64;
};

extern const Recipe recipes[];

extern const size_t numRecipes;

const Recipe* getRecipe(uint16_t id);
