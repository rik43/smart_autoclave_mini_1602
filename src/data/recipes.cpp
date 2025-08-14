#include "data/recipes.h"

// Definition of the recipes array
// https://www.base64encode.org/ (with default options)
const Recipe recipes[] = {
    // id, page, temp, time, name, nameB64
    {0x01, 1, 120, 75, "Говядина", "0JPQvtCy0Y/QtNC40L3QsA=="},
    {0x02, 1, 120, 60, "Свинина", "0KHQstC40L3QuNC90LA="},

    {0x03, 2, 120, 40, "Птица", "0J/RgtC40YbQsA=="},
    {0x04, 2, 115, 60, "Рыба", "0KDRi9Cx0LA="},
    {0x05, 2,  97, 10, "Огурцы", "0J7Qs9GD0YDRhtGL"},
    {0x06, 2, 110, 20, "Овощи", "0J7QstC+0YnQuA=="},

    {0x07, 3, 120, 50, "Каши", "0JrQsNGI0Lg="},
    {0x08, 3, 110, 10, "Варенье", "0JLQsNGA0LXQvdGM0LU="},
};

// Количество рецептов
const size_t numRecipes = sizeof(recipes) / sizeof(recipes[0]);

const Recipe* getRecipe(uint16_t id) {
    for (size_t i = 0; i < numRecipes; ++i) {
        const Recipe* currentRecipe = &recipes[i];
        if (currentRecipe->id == id) return currentRecipe;
    }

    return nullptr;
}

