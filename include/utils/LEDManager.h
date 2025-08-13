#pragma once
#include <Arduino.h>

#define LED_MODE_OFF    0
#define LED_MODE_ON     1
#define LED_MODE_BLINK  2
#define LED_MODE_PULSE  3

#define MANAGER_MODE_NORMAL  0
#define MANAGER_MODE_BITS    1
#define MANAGER_MODE_TICK    2
#define MANAGER_MODE_PULSE   3

class LM_LED {
private:
    bool state;
    unsigned long startTime;
    unsigned long duration;
    unsigned long periodOn;
    unsigned long periodOff;
    uint8_t mode; // 0 - off, 1 - on, 2 - blink, 3 - pulse
    //uint8_t brightness;

public:
    uint8_t pin;

    LM_LED(uint8_t pin) : pin(pin), state(false), startTime(0), duration(0), periodOn(1000), periodOff(1000), mode(0) {
        pinMode(pin, OUTPUT);
        ledOff();
    }

    void ledOn() {
        analogWrite(pin, 100);
    }

    void ledOff() {
        analogWrite(pin, 0);
    }

    void set(uint8_t brightness) {
        analogWrite(pin, brightness);
    }

    void on(uint32_t timeMs = 0) {
        mode = LED_MODE_ON;
        state = true;
        startTime = millis();
        duration = timeMs;
        ledOn();
    }

    void off(uint32_t timeMs = 0) {
        mode = LED_MODE_OFF;
        state = false;
        startTime = millis();
        duration = timeMs;
        ledOff();
    }

    void blink(uint32_t timeMs = 0, uint32_t pOn = 500, uint32_t pOff = 500) {
        mode = LED_MODE_BLINK;
        startTime = millis();
        duration = timeMs;
        periodOn = pOn;
        periodOff = pOff;
    }

    void pulse(uint32_t timeMs = 0, uint32_t period = 1000) {
        mode = LED_MODE_PULSE;
        startTime = millis();
        duration = timeMs;
        periodOn = period;
    }

    void update() {
        unsigned long currentTime = millis();
        unsigned long elapsedTime = currentTime - startTime;

        if (duration > 0 && elapsedTime >= duration) {
            ledOff();
            return;
        }

        switch (mode) {
            case LED_MODE_ON: // on
                ledOn();
                break;
            case LED_MODE_BLINK: // blink
                if (state && elapsedTime >= periodOn) {
                    state = false;
                    ledOff();
                    startTime = currentTime;
                } else if (!state && elapsedTime >= periodOff) {
                    state = true;
                    ledOn();
                    startTime = currentTime;
                }
                break;
            case LED_MODE_PULSE: // pulse
                if (elapsedTime > 30) {
                    startTime = currentTime;
                    uint8_t brightness = (sin(2.0 * PI * currentTime / periodOn) + 1.0) * 50.0f; // 127.5; // 
                    set(brightness);
                }
                break;
            default: // off
                ledOff();
                break;
        }
    }
};


class LEDManager {
private:
    uint8_t currentMode = MANAGER_MODE_NORMAL;
    unsigned long tickStartTime = 0;
    uint8_t tickIndex = 0;
    uint16_t tickInterval = 200;

public:
    LM_LED leds[4] = {LM_LED(LED_1_PIN), LM_LED(LED_2_PIN), LM_LED(LED_3_PIN), LM_LED(LED_4_PIN)};

    LEDManager() {}

    void loop() {
        if (currentMode == MANAGER_MODE_NORMAL) {
            for (int i = 0; i < 4; i++) {
                leds[i].update();
            }
        } else if (currentMode == MANAGER_MODE_TICK) {
            unsigned long currentTime = millis();
            if (currentTime - tickStartTime >= tickInterval) {  // Example delay for tick effect
                ledOff(tickIndex);
                tickIndex = (tickIndex + 1) % 4;
                ledOn(tickIndex);
                tickStartTime = currentTime;
            }
        } else if (currentMode == MANAGER_MODE_PULSE) {
            unsigned long currentTime = millis();
            if (currentTime - tickStartTime > 30) {
                tickStartTime = currentTime;
                for (int i = 0; i < 4; i++) {
                    float phaseShift = i * 0.2f;  // Phase shift for each LED
                    int brightness = (sin(2.0 * PI * (currentTime / (float)tickInterval + phaseShift)) + 1.0) * 50; // 127.5; //
                    ledSet(i, brightness);
                }
            }

        }
    }

    void bits(uint8_t bitsValue) {
        currentMode = MANAGER_MODE_BITS;
        for (int i = 0; i < 4; i++) {
            if (bitsValue & (1 << i)) {
                ledOn(i);
            } else {
                ledOff(i);
            }
        }
    }

    void ticker(uint16_t interval = 200) {
        currentMode = MANAGER_MODE_TICK;
        tickStartTime = millis();
        tickIndex = 0;
        tickInterval = interval;
        ledOn(0);
    }

    void pulse(uint16_t interval = 1000) {
        currentMode = MANAGER_MODE_PULSE;
        tickInterval = interval;
    }

    void pulseIdle() {
        pulse(2400);
    }

    void pulseActive() {
        pulse(800);
    }

    void normalMode() {
        currentMode = MANAGER_MODE_NORMAL;
    }

    void ledOn(int i) {
        leds[i].ledOn();
    }

    void ledOff(int i) {
        leds[i].ledOff();
    }

    void ledSet(int i, int brightness) {
        leds[i].set(brightness);
    }
};
