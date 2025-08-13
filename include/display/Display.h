#pragma once

#include <cmath>
#include "global.h"
#include "utils.h"
#include "data/recipes.h"
#include "settings/SettingsStore.h"
#include "display_const.h"
#include "display/DwinDisplay.h"
#include "display/DisplayProgramStarter.h"
#include "events/events_prog.h"
#include "events/EventDisplay.h"
#include "events/EventHardState.h"
#include "events/EventSoftState.h"
#include "events/EventConnection.h"
#include "events/EventUpdateStatus.h"
#include "commands/CommandProgStop.h"
#include "commands/CommandPairing.h"
#include "commands/CommandConnectionWipe.h"

extern App app;

// Наличие данных для соединения (для показа правильного экрана "соединения")
enum ConnectionDataState {
    CONNECTION_DATA_UNKNOWN,
    CONNECTION_DATA_EMPTY,
    CONNECTION_DATA_WAITING,
    CONNECTION_DATA_STORED,
};

enum WaterCheckSource {
    WATER_CHECK_FOR_COOKING_PROGRAM, // запуск готовой программы (из книги рецептов)
    WATER_CHECK_FOR_COOKING_SELF, // запуск собственной программы
    WATER_CHECK_FOR_DISTILLATION, // запуск дистилляции
    WATER_CHECK_FOR_SUVID, // запуск сувид-рецепта
};

class Display {
private:
    DwinDisplay& dwin;

    SettingsStore& settings;

    int16_t customTemperature = 120; // градусы
    int16_t customTimeMin = 60; // минуты
    int16_t customPower = 50; // 0-100%

    int16_t customTemperatureSuvid = 70; // градусы
    int16_t customTimeHoursSuvid = 6; // часы
    int16_t customTimeMinutesSuvid = 0; // минуты
    int16_t customTimeSuvid = 0x0000; // время в формате 0xHHMM

    ConnectionDataState connectionDataState = CONNECTION_DATA_EMPTY;
    
    // откуда попали на экран проверки наличия воды
    // чтобы понимать, куда отправляться после проверки (и в случае продолжения и при отмене)
    WaterCheckSource waterCheckSource = WATER_CHECK_FOR_COOKING_PROGRAM;

    uint16_t lastRecipeId = 0;
    uint16_t lastRecipeBookPageNumber = 1;
    std::string recipeName;

    uint8_t statusIcons = 0; // bits: 0-ble, 1-ble_with_client, 2-wifi, 3-server
    uint8_t wifiSignalStrength = 0; // 0-4, 0-unknown, 4-good, 3-medium, 2-bad, 1-very bad

    uint8_t screenBrightness = 100; // 0-100  0x64

    uint16_t displayFirmwareVersion = 0; // версия прошивки дисплея (из дисплея)

    // предыдущее состояние программы готовки, для отслеживания и отправки только измененных данных
    uint16_t lastPage = 0; // последний открытый экран
    int32_t prevProgramPage = 0; // последний открытый экран программы
    int32_t prevStepType = 0; // последний тип шага
    int32_t prevTimeType = 0; // последний тип времени (прошло/осталось)
    int32_t prevStep = 0; // предыдущий шаг
    int32_t prevStepCount = 0; // предыдущие количество шагов

    float lastTemperature = 0;

    DisplayProgramStarter programStarter;

    float prevPwmValue; // предыдущее значение ШИМ

    Timer* timerProgramUpdate = nullptr;

    // для страницы подключение
    bool hasConnectionConfig = false;
    bool isInPairingMode = false;

    bool isShowedError = false; // после запуска программы был показан экран ошибки (показываем его 1 раз за готовку)
    bool isFirmwareUpdateMode = false; // была запущена прошивка

    //uint32_t beepTimeout = 0; // время повтора писка
    uint32_t beepCount = 0; // кол-во писков
    uint32_t beepDuration = 1000; // продолжительность писка
    Timer beepTimer = Timer(2000); // таймер повтора писка

public:
    Display(DwinDisplay& dwin, SettingsStore& settings) : dwin(dwin), settings(settings) {
        timerProgramUpdate = new Timer(1000);
    }

    ~Display() {
        delete timerProgramUpdate;
    }

    void setup() {
        dwin.setVarCallback([this](uint16_t address, uint16_t value, uint8_t* buffer, uint8_t bufferLength) {
            this->onVarChange(address, value, buffer, bufferLength);
        });

        //showSplashScreen();
        dwin.sendReset();
    }

    void start() {
        // смена заставки на главый экран
        showPageHome();

        // установить яркость из настроек
        int16_t bright = settings.getScreenBrightness();
        if (bright >= 5) {
            screenBrightness = bright;
        }
        dwin.setVar(DWIN_VAR_BRIGHTNESS, screenBrightness); // ползунок
        setBrightness(screenBrightness); // яркость

        // запросить себе версию дисплея
        dwin.sendReadVar(DWIN_VAR_VERSION_DISPLAY);

        // обновить на дисплее версии прошивки и платы
        dwin.setVar(DWIN_VAR_VERSION_FIRMWARE, static_cast<uint16_t>(settings.getFirmwareVersion()));
        dwin.setVar(DWIN_VAR_VERSION_HARDWARE, static_cast<uint16_t>(settings.getHardwareVersion()));

        // Дефолтные пользовательские настройки
        sendCustomTemperature();
        sendCustomTimeMin();
        sendCustomPower();

        // Сувид пользовательские настройки
        sendCustomTemperatureSuvid();
        sendCustomTimeHoursSuvid();
        sendCustomTimeMinutesSuvid();
        sendCustomTimeSuvid();

        stopTimeInfo();
        //delay(50); // ждём версию дисплея для descriptor
    }

    void loop() {
        dwin.loop();

        // можно заменить эту часть на приём переменных из soft-state
        if (timerProgramUpdate->ready() && app.program.isRunning) {
            updateProgramScreens();
        }

        // сменить финальный экран при понижении температуры ниже опасной
        if (lastPage == DWIN_PAGE_COMPLETE_HOT) {
            if (lastTemperature < HOT_WARNING_TEMPERATURE + 1) {
                setPage(DWIN_PAGE_COMPLETE_COLD);
            }
        }
        if (lastPage == DWIN_PAGE_STOPPED_HOT) {
            if (lastTemperature < HOT_WARNING_TEMPERATURE + 1) {
                setPage(DWIN_PAGE_STOPPED_COLD);
            }
        }

        if (beepCount > 0 && beepTimer.ready()) {
            beepCount--;
            dwin.beep(beepDuration);
        }
    }

    void startBeep(uint32_t count = 1, uint32_t duration = 1000, uint32_t interval = 2000) {
        beepCount = count;
        beepDuration = duration;
        beepTimer.setPeriod(interval);
        beepTimer.reset();
    }

    void stopBeep() {
        beepCount = 0;
    }

    void setPage(uint16_t page) {
        dwin.setPage(page);
        lastPage = page;
    }

    void showSplashScreen() {
        setPage(DWIN_PAGE_SPLASH_SCREEN);
    }

    void showPageHome() {
        setPage(DWIN_PAGE_HOME);
    }

    void showCompleteScreen() {
        setPage(DWIN_PAGE_COMPLETE);
    }

    // выход из рецепта в книгу рецептов на посл использованную страницу
    void showPageBookRecipe() {
        uint16_t page = DWIN_PAGE_RECIPE_BOOK_1;
        if (lastRecipeBookPageNumber == 2) page = DWIN_PAGE_RECIPE_BOOK_2;
        if (lastRecipeBookPageNumber == 3) page = DWIN_PAGE_RECIPE_BOOK_3;

        setPage(page);
    }

    // показать экран соединения соответствующий текущему статусу
    void showPagePairing() {
        switch (connectionDataState) {
        case CONNECTION_DATA_UNKNOWN:
        case CONNECTION_DATA_EMPTY:
            setPage(DWIN_PAGE_CONNECTION_INFO);
            break;

        case CONNECTION_DATA_WAITING:
            setPage(DWIN_PAGE_CONNECTION_WAIT);
            break;

        case CONNECTION_DATA_STORED:
            setPage(DWIN_PAGE_CONNECTION_ACTIVE);
            break;
        }
    }

    // обновление данных программы на дисплее
    void updateProgramScreens() {

        updateProgramScreenPage(app.program.paramDisplayScreen->getInt());

        // если шаг изменился
        if (prevStep != app.program.paramStepCurrent->getInt()) {
            prevStep = app.program.paramStepCurrent->getInt();
            dwin.setVar(DWIN_VAR_STEP_NUMBER, prevStep);
            dwin.beep();
        }
        // если кол-во шагов изменилось
        if (prevStepCount != app.program.paramStepCount->getInt()) {
            prevStepCount = app.program.paramStepCount->getInt();
            dwin.setVar(DWIN_VAR_STEP_COUNT, prevStepCount);
        }
        // если тип шага изменился
        if (prevStepType != app.program.paramStepType->getInt()) {
            prevStepType = app.program.paramStepType->getInt();
            dwin.setVar(DWIN_VAR_STEP_TYPE, prevStepType);
        }
        // если тип времени изменился (прошло/осталось)
        if (prevTimeType != app.program.paramTimeType->getInt()) {
            prevTimeType = app.program.paramTimeType->getInt();
            dwin.setVar(DWIN_VAR_TIME_TYPE, prevTimeType);
        }

        int32_t showTimeMsec = 0;

        if (prevTimeType == 0) { // прошло
            showTimeMsec = app.program.paramTimePassed->getInt();
        } else { // осталось
            showTimeMsec = app.program.paramTimeLeft->getInt();
        }
        // перевод из мс в мин. 30000 для округления (1 мин выводится с 31 сек)
        int32_t showTimeMinutes = (showTimeMsec + 30000) / 1000 / 60;
        dwin.setVar(DWIN_VAR_CURRENT_TIME_REST, showTimeMinutes); // старый формат (только минуты), оставляем для обратной совместимости
        dwin.setVar(DWIN_VAR_CURRENT_TIME_HHMM, secondsToDisplayTime(showTimeMinutes * 60)); // новый формат (часы и минуты)
    }

    void updateProgramScreenPage(int32_t programPage) {
        if (programPage && (prevProgramPage != programPage)) {
            prevProgramPage = programPage;
            switch (programPage) {
            case 10: setPage(DWIN_PAGE_PROGRAM_11); break; // нагрев (DWIN_PAGE_PROGRAM_10 is Deprecated)
            case 11: setPage(DWIN_PAGE_PROGRAM_11); break; // универсальный для готовки по шагам
            case 21: setPage(DWIN_PAGE_PROGRAM_21); break; // дистилляция на паузе
            case 22: setPage(DWIN_PAGE_PROGRAM_22); break; // дистилляция в работе
                //case 91: setPage(DWIN_PAGE_PROGRAM_91); break;
                //case 92: setPage(DWIN_PAGE_PROGRAM_92); break;
                //case 93: setPage(DWIN_PAGE_PROGRAM_93); break;
            default: setPage(DWIN_PAGE_PROGRAM_11);
            }
        }
    }

    void showEndProgramPage(bool emergencyStop = false, int16_t errorCode = 0) {
        
        // не показывать экраны завершения при остановке по ошибке
        if (errorCode) {
            showError(errorCode);
            return;
        }
        if (programStarter.isPowerMode()) {
            setPage(DWIN_PAGE_DISTILL_INACTIVE); // дистилляция - остановлена
        } else {

            bool isSuvidMode = programStarter.isSuvidMode();
            bool isHot = lastTemperature > HOT_WARNING_TEMPERATURE;

            if (emergencyStop) {
                if (isSuvidMode) {
                    setPage(DWIN_PAGE_SUVID);
                } else {
                    if (isHot) {
                        setPage(DWIN_PAGE_STOPPED_HOT);
                    } else {
                        setPage(DWIN_PAGE_STOPPED_COLD);
                    }
                }
            } else {
                if (isSuvidMode) {
                    setPage(DWIN_PAGE_COMPLETE_SUVID);
                    startBeep(3600, 512, 4000); // долго пищать при окончании сувид-рецепта (пока не нажата кнопка "ок" на экране "Готово")
                } else {
                    startBeep(1, 1024); // 1 бип при окончании готовки
                    if (isHot) {
                        setPage(DWIN_PAGE_COMPLETE_HOT);
                    } else {
                        setPage(DWIN_PAGE_COMPLETE_COLD);
                    }
                }
            }

            //dwin.longBeep();
        }
    }

    // передача в дисплей информации о времени остановки (шаг, минута)
    void stopTimeInfo(int16_t step = 0, int16_t minute = 0) {
        dwin.setVar(DWIN_VAR_STOPPED_STEP, step);
        dwin.setVar(DWIN_VAR_STOPPED_MINUTE, minute);
    }

    void setBrightness(uint8_t value) {
        if (value < 5) value = 5;
        if (value > 100) value = 100;
        screenBrightness = value;
        uint16_t data = combineToUint16(value, value); // одинаковая яркость в акт и ждущ режимах
        // 5A A5 07 82 0082 6432 03E8
        // 64 - ярк в акт реж, 32 - ярк в ждущ реж.
        // 03E8 = 10000 = 10сек ждущий режим
        dwin.setVar(0x0082, data, 0);
    }

    void onVarChange(uint16_t address, uint16_t value, uint8_t* buffer, uint8_t bufferLength) {
        //Serial.println(buffer_get_float32(buffer));
        switch (address) {
        case 0x0000: // todo temp - deprecated buttons var 
            debugln("[WARN] button VP=0x0000");
            break;

        case DWIN_VAR_BUTTON:
            onButtonPress(value);
            break;

        case DWIN_VAR_RECIPE_NUMBER: // выбор рецепта
            onRecipeSelect(value);
            break;

        case DWIN_VAR_VERSION_DISPLAY:
            displayFirmwareVersion = value;
            settings.setDisplayVersion(value); // for descriptor
            prt("display version", value);
            break;

        case DWIN_VAR_BRIGHTNESS:
            setBrightness(value);
            settings.setScreenBrightness(static_cast<int32_t>(value));
            prt("brightness", value);
            break;

        case DWIN_VAR_CUSTOM_TEMPERATURE:
            setCustomTemperature(value);
            break;

        case DWIN_VAR_CUSTOM_TIME:
            setCustomTimeMin(value);
            break;

        case DWIN_VAR_CUSTOM_POWER:
            setCustomPower(value);
            applyCustomPower();
            break;

        case DWIN_VAR_CUSTOM_SUVID_TEMPERATURE:
            setCustomTemperatureSuvid(value);
            break;

        case DWIN_VAR_CUSTOM_SUVID_TIME_HOURS:
            setCustomTimeHoursSuvid(value);
            sendCustomTimeSuvid();
            break;

        case DWIN_VAR_CUSTOM_SUVID_TIME_MINUTES:
            setCustomTimeMinutesSuvid(value);
            sendCustomTimeSuvid();
            break;
            
        }
    }

    // нажатие кнопки на экране
    void onButtonPress(uint16_t buttonCode) {
        switch (buttonCode) {
        case DWIN_BTN_PRESTART_PROGRAM: { // открыть окно проверки наличия воды (для готовой программы)
            debugln("PRESTART_PROGRAM");
            waterCheckSource = WATER_CHECK_FOR_COOKING_PROGRAM;
            setPage(DWIN_PAGE_WATER_CHECK);
        } break;

        case DWIN_BTN_PRESTART_SELF: { // открыть окно проверки наличия воды (для собственного режима)
            debugln("PRESTART_SELF");
            waterCheckSource = WATER_CHECK_FOR_COOKING_SELF;
            setPage(DWIN_PAGE_WATER_CHECK);
        } break;

        case DWIN_BTN_PRESTART_POWER: { // открыть окно проверки наличия воды
            debugln("PRESTART_POWER");
            waterCheckSource = WATER_CHECK_FOR_DISTILLATION;
            setPage(DWIN_PAGE_WATER_CHECK);
        } break;

        case DWIN_BTN_PRESTART_SUVID: { // открыть окно проверки наличия воды
            debugln("PRESTART_SUVID");
            waterCheckSource = WATER_CHECK_FOR_SUVID;
            setPage(DWIN_PAGE_WATER_CHECK);
        } break;

        case DWIN_BTN_WATER_SUBMIT: { // подтвердить наличие воды (реальный запуск)
            debugln("WATER_SUBMIT");
            if ((waterCheckSource == WATER_CHECK_FOR_COOKING_PROGRAM) || (waterCheckSource == WATER_CHECK_FOR_COOKING_SELF)) {
                // запуск готовой программы или собственного режима
                bool isRun = programStarter.runTemperatureMode(lastRecipeId, customTemperature, customTimeMin, recipeName, "autoclave");

            } else if (waterCheckSource == WATER_CHECK_FOR_SUVID) {
                // запуск сувид-рецепта
                setOwnRecipeSuvid();
                int32_t timeMinutes = customTimeHoursSuvid * 60 + customTimeMinutesSuvid;
                bool isRun = programStarter.runTemperatureMode(SUVID_MODE_PROGRAM_ID, customTemperatureSuvid, timeMinutes, recipeName, "suvid");

            } else if (waterCheckSource == WATER_CHECK_FOR_DISTILLATION) {
                // запуск дистилляции
                bool isRun = programStarter.runPowerMode(customPower);
            }
        } break;

        case DWIN_BTN_WATER_CANCEL: { // отмена подтверждения наличия воды (вернуться назад)
            debugln("WATER_CANCEL");
            if (waterCheckSource == WATER_CHECK_FOR_COOKING_PROGRAM) {
                setPage(DWIN_PAGE_RECIPE_PREVIEW);
            } else if (waterCheckSource == WATER_CHECK_FOR_COOKING_SELF) {
                setPage(DWIN_PAGE_RECIPE_SELF);
            } else if (waterCheckSource == WATER_CHECK_FOR_DISTILLATION) {
                setPage(DWIN_PAGE_DISTILL_INACTIVE);
            } else if (waterCheckSource == WATER_CHECK_FOR_SUVID) {
                setPage(DWIN_PAGE_SUVID);
            }
        } break;


        // Deprecated (если к esp подключен дисплей со старой прошивкой)
        case DWIN_BTN_START: {
            bool isRun = programStarter.runTemperatureMode(lastRecipeId, customTemperature, customTimeMin, recipeName, "autoclave");
        } break;

        case DWIN_BTN_START_POWER: {
            bool isRun = programStarter.runPowerMode(customPower);
        } break;
        // END: Deprecated

        case DWIN_BTN_STOP: {
            debugln("STOP");
            //run app.programStop();
            CommandProgStop cmd;
            dispatch(&cmd);
        } break;

        case DWIN_BTN_RECIPE_BACK: // Назад из рецепта в книгу
            showPageBookRecipe();
            break;

        case DWIN_BTN_HOME: // нажатие домой
            lastPage = 1;
            isShowedError = false;
            break;


        // BEGIN: ПОДКЛЮЧЕНИЕ
        case DWIN_BTN_PAIRING_SCREEN: // Экран "подключение"
            showPagePairing();
            break;

        case DWIN_BTN_PAIRING_START: { // Начать сопряжение
            connectionDataState = CONNECTION_DATA_WAITING;
            showPagePairing();
            CommandPairing cmd;
            cmd.active = true;
            dispatch(&cmd);
        } break;

        case DWIN_BTN_PAIRING_STOP: { // Остановить сопряжение
            connectionDataState = CONNECTION_DATA_EMPTY;
            showPagePairing();
            CommandPairing cmd;
            cmd.active = false;
            dispatch(&cmd);
        } break;

        case DWIN_BTN_PAIRING_FORGET: { // Отключиться
            connectionDataState = CONNECTION_DATA_EMPTY;
            showPagePairing();
            CommandConnectionWipe cmd;
            cmd.isInternal = true;
            dispatch(&cmd);
        } break;
        // END: ПОДКЛЮЧЕНИЕ


        // BEGIN: +/- Слайдеров
        case DWIN_BTN_TIME_SLIDER_DEC:
            setCustomTimeMin(customTimeMin - 1);
            sendCustomTimeMin();
            break;

        case DWIN_BTN_TIME_SLIDER_INC:
            setCustomTimeMin(customTimeMin + 1);
            sendCustomTimeMin();
            break;

        case DWIN_BTN_TEMPERATURE_SLIDER_DEC:
            setCustomTemperature(customTemperature - 1);
            sendCustomTemperature();
            break;

        case DWIN_BTN_TEMPERATURE_SLIDER_INC:
            setCustomTemperature(customTemperature + 1);
            sendCustomTemperature();
            break;

        case DWIN_BTN_POWER_SLIDER_DEC:
            setCustomPower(getPrevRound(customPower, STEP_CUSTOM_POWER));
            sendCustomPower();
            applyCustomPower();
            break;

        case DWIN_BTN_POWER_SLIDER_INC:
            setCustomPower(getNextRound(customPower, STEP_CUSTOM_POWER));
            sendCustomPower();
            applyCustomPower();
            break;

        // Сувид
        case DWIN_BTN_SUVID_HOURS_SLIDER_DEC:
            setCustomTimeHoursSuvid(customTimeHoursSuvid - 1);
            sendCustomTimeHoursSuvid();
            sendCustomTimeSuvid();
            break;

        case DWIN_BTN_SUVID_HOURS_SLIDER_INC:
            setCustomTimeHoursSuvid(customTimeHoursSuvid + 1);
            sendCustomTimeHoursSuvid();
            sendCustomTimeSuvid();
            break;

        case DWIN_BTN_SUVID_MINUTES_SLIDER_DEC:
            setCustomTimeMinutesSuvid(customTimeMinutesSuvid - 1);
            sendCustomTimeMinutesSuvid();
            sendCustomTimeSuvid();
            break;

        case DWIN_BTN_SUVID_MINUTES_SLIDER_INC:
            setCustomTimeMinutesSuvid(customTimeMinutesSuvid + 1);
            sendCustomTimeMinutesSuvid();
            sendCustomTimeSuvid();
            break;

        case DWIN_BTN_SUVID_TEMPERATURE_SLIDER_DEC:
            setCustomTemperatureSuvid(customTemperatureSuvid - 1);
            sendCustomTemperatureSuvid();
            break;

        case DWIN_BTN_SUVID_TEMPERATURE_SLIDER_INC:
            setCustomTemperatureSuvid(customTemperatureSuvid + 1);
            sendCustomTemperatureSuvid();
            break;

            
            // END: +/- Слайдеров

        case DWIN_BTN_STOP_REJECTION: // НЕТ - отклонить остановку программы
            setPage(lastPage);
            break;

        case DWIN_BTN_COMPLETE_OK: // ОК - подтвердить окончание сувид-рецепта (экран "Готово")
            setPage(DWIN_PAGE_HOME);
            stopBeep(); // прекращаем писк когда пользователь обратил внимание и нажал кнопку "ок" на экране "Готово"
            break;
        }
    }

    int16_t getNextRound(int value, int16_t step) {
        return ((value / step) + 1) * step;
    }

    int16_t getPrevRound(int value, int16_t step) {
        return ((value - 1) / step) * step;
    }

    void setCustomTemperature(int16_t value) {
        if (value < MIN_CUSTOM_TEMPERATURE) value = MIN_CUSTOM_TEMPERATURE;
        if (value > MAX_CUSTOM_TEMPERATURE) value = MAX_CUSTOM_TEMPERATURE;
        customTemperature = value;
        setOwnRecipe();
    }

    void sendCustomTemperature() {
        dwin.setVar(DWIN_VAR_CUSTOM_TEMPERATURE, customTemperature);
    }

    void setCustomTimeMin(int16_t value) {
        if (value < MIN_CUSTOM_TIME) value = MIN_CUSTOM_TIME;
        if (value > MAX_CUSTOM_TIME) value = MAX_CUSTOM_TIME;
        customTimeMin = value;
        setOwnRecipe();
    }

    void sendCustomTimeMin() {
        dwin.setVar(DWIN_VAR_CUSTOM_TIME, customTimeMin);
    }

    void setCustomPower(int16_t value) {
        if (value < MIN_CUSTOM_POWER) value = MIN_CUSTOM_POWER;
        if (value > MAX_CUSTOM_POWER) value = MAX_CUSTOM_POWER;
        customPower = value;
    }

    void sendCustomPower() {
        dwin.setVar(DWIN_VAR_CUSTOM_POWER, customPower);
    }

    void applyCustomPower() {
        // send to running program
        if (programStarter.isPowerMode()) {
            programStarter.setPowerModePowerParam(customPower);
        }
    }

    void setCustomTemperatureSuvid(int16_t value) {
        if (value < MIN_CUSTOM_TEMPERATURE_SUVID) value = MIN_CUSTOM_TEMPERATURE_SUVID;
        if (value > MAX_CUSTOM_TEMPERATURE_SUVID) value = MAX_CUSTOM_TEMPERATURE_SUVID;
        customTemperatureSuvid = value;
    }

    void sendCustomTemperatureSuvid() {
        dwin.setVar(DWIN_VAR_CUSTOM_SUVID_TEMPERATURE, customTemperatureSuvid);
    }

    void setCustomTimeHoursSuvid(int16_t value) {
        if (value < MIN_CUSTOM_TIME_SUVID_HOURS) value = MIN_CUSTOM_TIME_SUVID_HOURS;
        if (value > MAX_CUSTOM_TIME_SUVID_HOURS) value = MAX_CUSTOM_TIME_SUVID_HOURS;
        customTimeHoursSuvid = value;
    }

    void sendCustomTimeHoursSuvid() {
        dwin.setVar(DWIN_VAR_CUSTOM_SUVID_TIME_HOURS, customTimeHoursSuvid);
    }

    void setCustomTimeMinutesSuvid(int16_t value) {
        if (value < MIN_CUSTOM_TIME_SUVID_MINUTES) value = MIN_CUSTOM_TIME_SUVID_MINUTES;
        if (value > MAX_CUSTOM_TIME_SUVID_MINUTES) value = MAX_CUSTOM_TIME_SUVID_MINUTES;
        customTimeMinutesSuvid = value;
    }

    void sendCustomTimeMinutesSuvid() {
        dwin.setVar(DWIN_VAR_CUSTOM_SUVID_TIME_MINUTES, customTimeMinutesSuvid);
    }

    void sendCustomTimeSuvid() {
        int16_t time = secondsToDisplayTime(customTimeHoursSuvid * 3600 + customTimeMinutesSuvid * 60);
        dwin.setVar(DWIN_VAR_CUSTOM_SUVID_TIME, time);
    }

    // перевод времени в секундах в формат для дисплея (часы:минуты)
    int16_t secondsToDisplayTime(int32_t seconds) {
        int8_t hours = seconds / 3600;
        int8_t minutes = (seconds % 3600) / 60;
        // если 1 символ то добавляем 100 для корректного отображения последних двух символов 100 = "00"
        if (minutes < 10) {
            minutes += 100;
        }
        return combineToUint16(hours, minutes);
    }

    int32_t displayTimeToSeconds(int16_t time) {
        int8_t hours = time >> 8;
        int8_t minutes = time & 0xFF;
        if (minutes >= 100) {
            minutes -= 100;
        }

        return hours * 3600 + minutes * 60;
    }

    // Пометить рецепт как "свой рецепт"
    void setOwnRecipe() {
        recipeName = "0KHQstC+0Lkg0YDQtdGG0LXQv9GC"; // "Свой рецепт";
    }

    // https://www.base64encode.org/
    void setOwnRecipeSuvid() {
        recipeName = "0KHRgy3QstC40LQ="; // "Су-вид";
    }

    // выбор рецепта иконкой на дисплее
    void onRecipeSelect(uint16_t recipeId) {
        prt("Selected recipe", recipeId);

        const Recipe* recipe = getRecipe(recipeId);
        if (recipe != nullptr) {
            setCustomTemperature(recipe->cookingTemperature);
            setCustomTimeMin(recipe->cookingTimeMinutes);
            lastRecipeBookPageNumber = recipe->pageNumber;
            lastRecipeId = recipeId;
            recipeName = recipe->name;
            prt("lastRecipeBookPageNumber", lastRecipeBookPageNumber);
        }
        else {
            prt("no recipe", recipeId);
            setCustomTemperature(0);
            setCustomTimeMin(0);
        }
        sendCustomTemperature();
        sendCustomTimeMin();
    }

    void updateConnections(const EventConnection& event) {
        statusIcons = 0;
        if (event.ble_client) { // подключен клиент
            statusIcons |= 2;
        } else if (event.ble_config){ // ble активен, но нет подключенного клиента
            statusIcons |= 1;
        }
        if (event.wifi) { // wifi активен
            statusIcons |= 4;
        }
        if (event.server) { // сервер активен
            statusIcons |= 8;
        }

        // для страницы подключение
        hasConnectionConfig = event.ble_config || event.wifi_config;
        if (hasConnectionConfig) {
            connectionDataState = CONNECTION_DATA_STORED;
        }

        if (isInPairingMode != event.pairing) {
            isInPairingMode = event.pairing;
            if (!isInPairingMode) { // режим сопряжения отключился
                if (hasConnectionConfig) { // и данные соединения есть
                    showPagePairing();
                }
            }
        }

        updateConnectionsIcons();
    }

    void updateConnectionsIcons() {
        int16_t icons[3] = { DWIN_ICON_EMPTY, DWIN_ICON_EMPTY, DWIN_ICON_EMPTY };
        int iconFill = 0;
        if (statusIcons & 1) {
            icons[iconFill++] = DWIN_ICON_BLE;
        }
        if (statusIcons & 2) {
            icons[iconFill++] = DWIN_ICON_BLE_CONNECTED;
        }
        if (statusIcons & 4) {
            switch (wifiSignalStrength) {
                case 0: icons[iconFill++] = DWIN_ICON_WIFI; break;
                case 1: icons[iconFill++] = DWIN_ICON_WIFI_1; break;
                case 2: icons[iconFill++] = DWIN_ICON_WIFI_2; break;
                case 3: icons[iconFill++] = DWIN_ICON_WIFI_3; break;
                case 4: icons[iconFill++] = DWIN_ICON_WIFI_4; break;
                default: icons[iconFill++] = DWIN_ICON_WIFI; break;
            }
        }
        if (statusIcons & 8) {
            icons[iconFill++] = DWIN_ICON_CLOUD;
        }
        dwin.setVar(DWIN_VAR_STATUS_ICONS, icons[0], icons[1], icons[2]);
    }

    // включить на дисплее экран обновления ПО и выводить общий прогресс
    void updateFirmwareStatus(EventUpdateStatus& eventUpdate) {
        if (eventUpdate.filesCount == 0) return; // Avoid division by zero
        
        uint16_t overallProgress = ((eventUpdate.fileNum - 1) * 100 + eventUpdate.progress) / eventUpdate.filesCount;
        
        dwin.setVar(DWIN_VAR_FIRMWARE_PROGRESS, overallProgress);
        
        if (! isFirmwareUpdateMode) {
            isFirmwareUpdateMode = true;
            setPage(DWIN_PAGE_FIRMWARE_UPDATE);
        }
    }

    void showError(uint16_t errorCode) {
        isShowedError = true;
        setPage(DWIN_PAGE_ERROR);
        dwin.setVar(DWIN_VAR_ERROR_CODE, errorCode);
    }

    uint8_t getWifiSignalStrengthFromRssi(int8_t rssi) {
        if (rssi == 0) return 0; // неизвестно
        else if (rssi < -70) return 1; // плохо
        else if (rssi < -60) return 2; // средне
        else if (rssi < -50) return 3; // хорошо
        else return 4; // отлично
    }

    void updateWifiSignalStrength(int8_t rssi) {
      uint8_t strength = getWifiSignalStrengthFromRssi(rssi);
      if (strength != wifiSignalStrength) {
        wifiSignalStrength = strength;
        updateConnectionsIcons();
      }
    }

    //void handleProgInfo(const EventProgInfo& event) {
    //    if (event.errorCode && ! isShowedError) {
    //        showError(event.errorCode);
    //    }
    //}

    void dispatchHardState(EventHardState& event) {
        float temperature = event.temperature;
        lastTemperature = event.temperature;
        dwin.setVar(DWIN_VAR_CURRENT_TEMPERATURE, static_cast<uint16_t>(std::round(temperature)));

        if (programStarter.isPowerModeRunning()) {
            // обновить бегунок на дисплее по текущей мощности 
            // изменение извне (с другого контроллера)
            // todo значение 0 не будет устанавливаться
            if ((event.pwm > 0.0001) && std::abs(prevPwmValue - event.pwm) > 0.005) {
                prevPwmValue = event.pwm;
                setCustomPower(std::round(event.pwm * MAX_CUSTOM_POWER));
                sendCustomPower();
            }
        }

        // показать ошибку которая возникла не во время запуска программы
        if (!programStarter.isProgramRunning() && !event.isValidTemperature && !isShowedError) {
            stopTimeInfo(0, 0);
            showError(ERROR_CODE_NOT_VALID_TEMPERATURE);
        }
        
        updateWifiSignalStrength(event.wifiRssi);
    }

    void dispatchEvent(Event& event) {
        if (event.name == "hard-state") {
            EventHardState& eventHS = static_cast<EventHardState&>(event);
            dispatchHardState(eventHS);
        }

        // Этапы записи программы
        else if (event.name == "prog.reading") { // начало записи программы
            EventProgReading& eventProg = static_cast<EventProgReading&>(event);
            programStarter.setReading(eventProg.pid);
            stopBeep();
        }
        else if (event.name == "prog.ready") { // программа готова
            EventProgReady& eventProg = static_cast<EventProgReady&>(event);
            programStarter.setReady(eventProg.pid);
        }
        else if (event.name == "prog.started") { // программа запущена
            EventProgStarted& eventProg = static_cast<EventProgStarted&>(event);
            programStarter.setStarted(eventProg.pid, eventProg.id);
            prevProgramPage = 0;
            isShowedError = false; // reset show error message
        }
        else if (event.name == "prog.stopped") { // аварийная/ручная остановка программы
            EventProgStopped& eventProg = static_cast<EventProgStopped&>(event);
            stopTimeInfo(eventProg.stoppedStep, std::round(eventProg.stoppedSecond / 60));
            showEndProgramPage(true, eventProg.errorCode);
            programStarter.setStopped();
        }
        else if (event.name == "prog.completed") { // успешное завершение программы
            //EventProgCompleted& eventProg = static_cast<EventProgCompleted&>(event);
            showEndProgramPage(false);
            programStarter.setCompleted();
        }
        //else if (event.name == "prog.info") { // успешное завершение программы
        //    EventProgInfo& event = static_cast<EventProgInfo&>(event);
        //    handleProgInfo(event);
        //}
        else if (event.name == "connection") { // смена состояния подключений
            EventConnection& eventConn = static_cast<EventConnection&>(event);
            updateConnections(eventConn);
        }
        else if (event.name == "update-status") { // обновление ПО
            EventUpdateStatus& eventUpdate = static_cast<EventUpdateStatus&>(event);
            updateFirmwareStatus(eventUpdate);
        }
        else if (event.name == "display") { // отображение на дисплее сообщений извне
            EventDisplay& eventDisplay = static_cast<EventDisplay&>(event);
            switch (eventDisplay.code) {
                case DISPLAY_COMPLETE:
                    showCompleteScreen();
                    break;
            }
        }
    }
};