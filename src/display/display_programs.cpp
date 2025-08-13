
#include <string>
#include <sstream>
#include <iomanip>
#include "display/display_programs.h"


std::string getDefaultUuidById(uint32_t id) {
    std::stringstream ss;
    ss << std::setw(8) << std::setfill('0') << std::dec << id;
    std::string strId = ss.str();
    std::string uuid = strId + "-1111-0000-0000-000000000000";

    return uuid;
}

// дескриптор рецепта для моб приложения
std::string getTemperatureModeProgramInfo(uint32_t recipeId, const std::string& recipeName, const std::string& subtype, int temperature, int timeSec) {
    std::ostringstream jsonStream;

    jsonStream << "{\"v\":1,\"type\":\"recipe\",\"rev\":1,\"name\":\"" << recipeName
               << "\",\"subtype\":\"" << subtype // подтип: "suvid" или "autoclave"
               << "\",\"uuid\":\"" << getDefaultUuidById(recipeId)
               << "\",\"steps\":[{\"type\":\"warmup\",\"tmp\":" << (temperature)
               << "},{\"type\":\"hold\",\"tmp\":" << temperature
               << ",\"time\":" << timeSec
               << "}"; // добавлено первые 2 шага

    // если не сувид, то добавляем шаг охлаждения
    if (recipeId != SUVID_MODE_PROGRAM_ID) {
        int temperatureMin = temperature < 80 ? temperature : 80;
        jsonStream << ",{\"type\":\"cooling\",\"tmp\":" << temperatureMin << "}";
    }

    jsonStream << "]}"; // закрываем массив шагов

    return jsonStream.str();
}


const char* temperatureModeProgramLines[] = {
    "<P25",
    "@TMP1:float@public=1",
    "@PWM1:float@public=1",
    "@V001:float=0@", // темп готовки
    "@V002:float=0@", // темп нагрева (переход на готовку)
    "@TOFF:float=80@", // остывание до данной температуры (и выключение)
    "@B002:bool=0@",
    "@TM01:int=65000@",
    "@CMPT:bool=0@",
    "@R_KP:float=0.2@",
    "@R_KI:float=0.001@",
    "@R_KD:float=0.1@",
    "@LIMN:float=0@",
    "@LIMX:float=0.7@",
    "@VALP:float=0@",
    "@VALI:float=0@",
    "@VALD:float=0@",
    "@NEXT:bool@",
    "@DSPL:int@",
    "@STEP:int@public=1",
    "@CSTP:int=3@public=1",
    "@TIME:int@public=1",
    "@TPSS:int@public=1",
    "@TTYP:bool=0@public=1",
    "@TYPE:int=0@public=1",
    "@MAXT:float=0@public=1",
    ">P",
    "<B6",
    "{4",
        "@S@set={STEP}&a=1",
        "@S@set={DSPL}&a=11",
        "@S@set={TYPE}&a=1",
        "@Next@",
    "}",
    "{4",
        "@PID@input={TMP1}&target={V001}&output={PWM1}&kp={R_KP}&ki={R_KI}&kd={R_KD}&value_p={VALP}&value_i={VALI}&value_d={VALD}&i_limit_min={LIMN}&i_limit_max={LIMX}",
        "@Timer@time={TPSS}",
        "@Compare@set={B002}&a={TMP1}&sign=>=&b={V002}",
        "@Next@enabled={B002}",
    "}",
    "{8",
        "@S@set={STEP}&a=2",
        "@S@set={TYPE}&a=3",
        "@S@set={DSPL}&a=11",
        "@S@set={TTYP}&a=1",
        "@S@set={CMPT}&a=0",
        "@S@set={TPSS}&a=0",
        "@S@set={TIME}&a={TM01}",
        "@Next@",
    "}",
    "{4",
        "@PID@input={TMP1}&target={V001}&output={PWM1}&kp={R_KP}&ki={R_KI}&kd={R_KD}&value_p={VALP}&value_i={VALI}&value_d={VALD}&i_limit_min={LIMN}&i_limit_max={LIMX}",
        "@Timer@time={TIME}&countdown=1",
        "@Compare@set={CMPT}&a={TIME}&sign=<&b=0",
        "@Next@enabled={CMPT}",
    "}",
    "{6",
        "@S@set={STEP}&a=3",
        "@S@set={DSPL}&a=11",
        "@S@set={TYPE}&a=2",
        "@S@set={TTYP}&a=0",
        "@S@set={PWM1}&a=0",
        "@Next@",
    "}",
    "{3",
        "@Timer@time={TPSS}",
        "@Compare@set={B002}&a={TMP1}&sign=<&b={TOFF}",
        "@Next@enabled={B002}",
    "}",
    ">B",
};



// дескриптор дистилляции для моб приложения
std::string getPowerModeProgramInfo() {
    std::ostringstream jsonStream;

    jsonStream << "{\"v\":1,\"type\":\"power\",\"rev\":1,\"name\":\"Дистилляция\",\"uuid\":\"00000999-1111-0000-0000-000000000000\",\"steps\":[{\"type\":\"power\",\"value\":0}]}";

    return jsonStream.str();
}

const char* powerModeProgramLines[] = {
    "<P5", // Параметры (кол-во)
    "@TMP1:float@public=1", // градусы для лога
    "@PWM1:float@public=1", // нагреватель, ШИМ 0-1
    "@PWR1:int=0@public=1", // мощность 0-100, %
    "@DSPL:int@public=1",
    "@NEXT:bool@",
    ">P", // конец параметров

    "<B2", // Начало блоков программы (кол-во блоков)
    "{2", // Блок 1
    "@S@set={DSPL}&a=22",
    "@Next@",
    "}",
    "{1", // Блок 2
    "@Divide@set={PWM1}&a={PWR1}&b=100",
    "}",
    ">B", // конец блоков
};


size_t temperatureModeProgramLinesCount = sizeof(temperatureModeProgramLines) / sizeof(temperatureModeProgramLines[0]);

size_t powerModeProgramLinesCount = sizeof(powerModeProgramLines) / sizeof(powerModeProgramLines[0]);;
