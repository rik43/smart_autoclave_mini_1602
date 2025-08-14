#pragma once

#include "const.h"
#include <LCD_1602_RUS_ALL.h>
#include <EncButton.h>
#include "utils/LEDManager.h"
#include "ui/ViewContext.h"
#include "ui/ScreenManager.h"
#include "Timer.h"
#include "display/DisplayProgramStarter.h"
#include "commands/CommandProgStop.h"
#include "events/events_prog.h"
#include "events/EventDisplay.h"
#include "events/EventHardState.h"
#include "events/EventSoftState.h"
#include "events/EventConnection.h"
#include "events/EventUpdateStatus.h"


extern App app;

/**
 * Класс для обработки пользовательских элементов управления и вывода на экран.
 * 
 * Он содержит текущий экран и переключается между экранами.
 * Формирует input events и отправляет их в текущий экран.
 * Отрисовывает текущий экран на дисплее.
 */
class AppController {
    private:
        ViewContext viewContext;
        ScreenManager screenManager;
        LEDManager ledManager;
        Timer displayUpdateTimer;
        Timer programUpdateTimer;

        // Для отправки событий вращения на 2 деления энкодера
        byte encoderPosition2current = 0; // для отслеживания положения энкодера и отправки событий вращения на 2 деления
        byte encoderPosition2previous = 0; // предыдущее событие turn2 отправленно по этому значению 

        DisplayProgramStarter programStarter;

    public:
        //LiquidCrystal_I2C lcd;
        LCD_1602_RUS lcd;

        EncButton enc;  

        AppController(LEDManager& ledManager):
            ledManager(ledManager),
            lcd(LCD_ADDRESS, LCD_WIDTH, LCD_HEIGHT),
            enc(ENCODER_S1, ENCODER_S2, ENCODER_KEY),
            displayUpdateTimer(200),
            programUpdateTimer(1000),
            screenManager(viewContext)
        {
        }

        ~AppController() = default;

        void begin() {
            Serial.println("AppController begin");
            lcd.init();
            lcd.backlight();
            lcd.clear();
        }

        void loop() {
            pollInput();
            update();
            draw();
        }

        void update() {
            screenManager.update();

            if (programUpdateTimer.ready()) {
                updateRunningProgram();
                ledManager.leds[1].setValue(static_cast<uint8_t>(viewContext.getPower() * 2.55));
            }
        }

        void draw() {
            if (displayUpdateTimer.ready()) {
                screenManager.draw(lcd);
            }
        }

        void pollInput() {
            if (enc.tick()) {
                if (enc.right()) {
                    screenManager.currentScreen->onEncoderTurn(true, enc.fast());
                    encoderPosition2current++;
                    if (encoderPosition2current % 2 == 0 && encoderPosition2current != encoderPosition2previous) {
                        encoderPosition2previous = encoderPosition2current;
                        screenManager.currentScreen->onEncoderTurn2(true, enc.fast());
                    }
                }
                if (enc.left()) {
                    screenManager.currentScreen->onEncoderTurn(false, enc.fast());
                    encoderPosition2current++;
                    if (encoderPosition2current % 2 == 0 && encoderPosition2current != encoderPosition2previous) {
                        encoderPosition2previous = encoderPosition2current;
                        screenManager.currentScreen->onEncoderTurn2(false, enc.fast());
                    }
                }
                if (enc.click()) {
                    encoderPosition2previous = encoderPosition2current; // сбросить счетчик, чтобы избежать срабатывания события вращения при нажатии на кнопку
                    screenManager.currentScreen->onEncoderButtonClick();
                }
                if (enc.hold()) {
                    bool allowBack = screenManager.currentScreen->onBack();
                    if (allowBack) {
                        screenManager.setScreen(&screenManager.homeScreen);
                    }
                }
            }
        }

        void startAutoclaveProcess() {
            bool isRun = programStarter.runTemperatureMode(
                viewContext.getRecipeId(),
                viewContext.getCustomTemperature(),
                viewContext.getCustomTimeMin(),
                viewContext.getRecipeName(),
                viewContext.getRecipeMode()
            );
        }

        void stopAutoclaveProcess() {
            CommandProgStop cmd;
            dispatch(&cmd);
        }

        void updateRunningProgram() {
            if (app.program.isRunning) {
                viewContext.setStepNumber(app.program.paramStepCurrent->getInt());
                viewContext.setStepCount(app.program.paramStepCount->getInt());
                viewContext.setStepType(app.program.paramStepType->getInt());
                viewContext.setTimeType(app.program.paramTimeType->getInt());

                uint32_t showTimeMsec = 0;
                if (viewContext.getTimeType() == 0) { // прошло
                    showTimeMsec = app.program.paramTimePassed->getInt();
                } else { // осталось
                    showTimeMsec = app.program.paramTimeLeft->getInt();
                }
                viewContext.setViewTimeMsec(showTimeMsec);
            }
        }

        void dispatchEvent(Event& event) {
            if (event.name == "hard-state") {
                EventHardState& eventHS = static_cast<EventHardState&>(event);
                viewContext.setTemperature(eventHS.temperature);
                viewContext.setPower(eventHS.pwm * 100.0f);
            }
    
            // Этапы записи программы
            else if (event.name == "prog.reading") { // начало записи программы
                EventProgReading& eventProg = static_cast<EventProgReading&>(event);
                programStarter.setReading(eventProg.pid);
            }
            else if (event.name == "prog.ready") { // программа готова
                EventProgReady& eventProg = static_cast<EventProgReady&>(event);
                programStarter.setReady(eventProg.pid);
            }
            else if (event.name == "prog.started") { // программа запущена
                EventProgStarted& eventProg = static_cast<EventProgStarted&>(event);
                programStarter.setStarted(eventProg.pid, eventProg.id);
                //prevProgramPage = 0;
                //isShowedError = false; // reset show error message
                screenManager.setScreen(&screenManager.autoclaveProcessScreen);
            }
            else if (event.name == "prog.stopped") { // аварийная/ручная остановка программы
                EventProgStopped& eventProg = static_cast<EventProgStopped&>(event);
                //stopTimeInfo(eventProg.stoppedStep, std::round(eventProg.stoppedSecond / 60));
                //showEndProgramPage(true, eventProg.errorCode);
                programStarter.setStopped();
            }
            else if (event.name == "prog.completed") { // успешное завершение программы
                //EventProgCompleted& eventProg = static_cast<EventProgCompleted&>(event);
                //showEndProgramPage(false);
                programStarter.setCompleted();
            }
            //else if (event.name == "prog.info") { // успешное завершение программы
            //    EventProgInfo& event = static_cast<EventProgInfo&>(event);
            //    handleProgInfo(event);
            //}
            else if (event.name == "connection") { // смена состояния подключений
                EventConnection& eventConn = static_cast<EventConnection&>(event);
                viewContext.setHasBluetooth(eventConn.ble);
                viewContext.setHasBluetoothClient(eventConn.ble_client);
                viewContext.setHasWifi(eventConn.wifi);
                viewContext.setHasServer(eventConn.server);
            }
            else if (event.name == "update-status") { // обновление ПО
                EventUpdateStatus& eventUpdate = static_cast<EventUpdateStatus&>(event);
                // updateFirmwareStatus(eventUpdate);
            }
            else if (event.name == "display") { // отображение на дисплее сообщений извне
                EventDisplay& eventDisplay = static_cast<EventDisplay&>(event);
                switch (eventDisplay.code) {
                    case DISPLAY_COMPLETE:
                        //showCompleteScreen();
                        break;
                }
            }
        }
};