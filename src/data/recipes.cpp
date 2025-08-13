#include "data/recipes.h"

// Definition of the recipes array
// https://www.base64encode.org/ (with default options)
const Recipe recipes[] = {
    // id, page, temp, time, name
    {0x01, 1, 120, 75, "0JPQvtCy0Y/QtNC40L3QsA=="}, // Говядина
    {0x02, 1, 120, 60, "0KHQstC40L3QuNC90LA="},     // Свинина

    {0x03, 2, 120, 40, "0J/RgtC40YbQsA=="},         // Птица
    {0x04, 2, 115, 60, "0KDRi9Cx0LA="},             // Рыба
    {0x05, 2,  97, 10, "0J7Qs9GD0YDRhtGL"},         // Огурцы
    {0x06, 2, 110, 20, "0J7QstC+0YnQuA=="},         // Овощи

    {0x07, 3, 120, 50, "0JrQsNGI0Lg="},             // Каши
    {0x08, 3, 110, 10, "0JLQsNGA0LXQvdGM0LU="},     // Варенье
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

