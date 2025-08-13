#pragma once

#include <string>
#include <cstddef>

#define TEMPERATURE_MODE_PROGRAM_ID 1
#define POWER_MODE_PROGRAM_ID 999
#define SUVID_MODE_PROGRAM_ID 977

extern const char* temperatureModeProgramLines[];
extern const char* powerModeProgramLines[];

extern size_t temperatureModeProgramLinesCount;
extern size_t powerModeProgramLinesCount;

std::string getDefaultUuidById(uint32_t id);

// get program desc
std::string getTemperatureModeProgramInfo(uint32_t recipeId, const std::string& recipeName, const std::string& subtype, int temperature, int timeSec);
std::string getPowerModeProgramInfo();
