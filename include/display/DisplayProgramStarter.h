#pragma once

#include "global.h"
#include "commands/commands_prog_create.h"
#include "commands/CommandSetParam.h"
#include "display/display_programs.h"

class DisplayProgramStarter {
private:
    // Флаги меняются по событиям
    bool isReading = false; // Подтверждено начало чтения программы
    bool isReady = false;   // Подтвержден приём программы
    bool isStarted = false; // Пришел сигнал о запуске программы
    bool isRunning = false; // Запущена (и не пауза)

    uint32_t pid = 1;
    uint32_t id = 1; // TODO: id recipe
    uint32_t runningProgramId = 0;

public:
    uint32_t getRunningProgramId() {
        return runningProgramId;
    }

    bool isPowerMode() {
        return (runningProgramId == POWER_MODE_PROGRAM_ID);
    }

    bool isSuvidMode() {
        return (runningProgramId == SUVID_MODE_PROGRAM_ID);
    }

    bool isPowerModeRunning() {
        return isRunning && isPowerMode();
    }

    bool isProgramRunning() {
        return isRunning;
    }

    // Запуск дистилляции
    bool runPowerMode(uint16_t powerWatt) {
        prt("Run power", powerWatt);

        std::string desc = getPowerModeProgramInfo();

        bool isStarted = runProgram(POWER_MODE_PROGRAM_ID, powerModeProgramLines, powerModeProgramLinesCount, desc);

        bool isStartedThis = isStarted && isPowerMode();

        if (isStartedThis) {
            setPowerModePowerParam(powerWatt);
        }

        return isStartedThis;
    }

    // Установить мощность в режиме дистилляции
    void setPowerModePowerParam(uint16_t powerWatt) {
        CommandSetParam cmdParam;
        if (powerWatt < 0) powerWatt = 0;
        if (powerWatt > 100) powerWatt = 100;
        cmdParam.setParam("PWR1", std::to_string(powerWatt));
        dispatch(&cmdParam);
    }

    // Запуск рецепта (темп-ра, время)
    bool runTemperatureMode(uint16_t recipeId, uint16_t temperature, uint16_t timeMinutes, const std::string& recipeName, const std::string& subtype) {
        prt("Start", temperature);
        prt("Time", timeMinutes);
        prt("Name", recipeName.c_str());

        std::string desc = getTemperatureModeProgramInfo(recipeId, recipeName, subtype, temperature, timeMinutes * 60);

        bool isStarted = runProgram(recipeId, temperatureModeProgramLines, temperatureModeProgramLinesCount, desc);

        bool isStartedThis = isStarted && (runningProgramId == recipeId);

        if (isStartedThis) {
            CommandSetParam cmdParam;
            cmdParam.setParam("V001", std::to_string(temperature)); // темп-ра готовки
            cmdParam.setParam("V002", std::to_string(temperature)); // нагрев до
            //cmdParam.setParam("V002", std::to_string(temperature - 1));
            cmdParam.setParam("TM01", std::to_string(timeMinutes * 60 * 1000)); // в мсек

            // для сувид 120 (чтобы не было шага охлаждения), для остальных 80
            bool isSuvid = (recipeId == SUVID_MODE_PROGRAM_ID);
            std::string toff = isSuvid ? "120" : "80";
            cmdParam.setParam("TOFF", toff); // остывание до данной температуры (и выключение)
            if (isSuvid) {
                cmdParam.setParam("CSTP", "2"); // чтобы моб приложение видело 2 шага
            }

            dispatch(&cmdParam);
        }

        return isStartedThis;
    }

    void reset() {
        isReading = false;
        isReady = false;
        isStarted = false;
        isRunning = false;
        runningProgramId = 0;
    }

    void setReading(uint32_t _pid) {
        if (pid == _pid) {
            isReading = true;
        }
    }

    void setReady(uint32_t _pid) {
        if (pid == _pid) {
            isReady = true;
        }
    }

    void setStarted(uint32_t _pid, uint32_t _id) {
        isStarted = true;
        isRunning = true;
        runningProgramId = _id;
        //if (pid == _pid) {
        //}
    }

    void setStopped() {
        reset();
    }

    void setCompleted() {
        reset();
    }


    bool runProgram(uint32_t _id, const char** lines, int countLines, const std::string& desc) {
        
        reset();

        pid = esp_random() >> 10;

        // начать запись программы
        // аналог app.compiler.create(pid, 1);
        CommandProgNew cmdNew;
        cmdNew.pid = pid;
        cmdNew.id = id = _id;
        cmdNew.uuid = getDefaultUuidById(_id);
        dispatch(&cmdNew);

        if (!isReading) { // компилятор готов записывать
            debugln("not reading");
            return false;
        }

        // построчная запись программы
        CommandProgLine cmdLine;
        for (int i = 0; i < countLines; i++) {
            cmdLine.pid = pid;
            cmdLine.lineNumber = i;
            cmdLine.code = lines[i];
            dispatch(&cmdLine);
        }

        if (!isReady) { // компилятор принял и обработал программу
            debugln("not ready");
            return false;
        }

        // Отправка дескриптора
        CommandProgDesc cmdDesc;
        cmdDesc.pid = pid;
        cmdDesc.desc = desc;
        dispatch(&cmdDesc);

        // запуск настроенной программы
        // аналог app.compiler.start(pid);
        CommandProgStart cmdStart;
        cmdStart.pid = pid;
        dispatch(&cmdStart);

        return isStarted;
    }

};


