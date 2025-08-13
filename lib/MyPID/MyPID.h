#pragma once

class MyPID {
public:
    float Kp, Ki, Kd;
    float setpoint = 0.0;
    float output = 0.0;
    float lastInput = 0.0;
    float iTerm = 0.0;
    unsigned long lastTime;
    float input = 0.0;
    float limitImin = 0; // I-koeff limit min (может быть с минусом)
    float limitImax = 0.2; // I-koeff limit max

    // D-koeff
    float lastDValue = 0.0; // Для хранения прошлого значения для D
    const int dDelay = 1000; // Время между сборами данных ошибок для D
    unsigned long lastDTime = 0; // Время последнего сохранения dValue
    float errors[10] = { 0,0,0,0,0,0,0,0,0,0 };
    uint32_t errorsSize = 10;
    uint32_t errorLastIndex = 0;

    float valueP = 0;
    float valueI = 0;
    float valueD = 0;

    MyPID() {
        lastTime = millis();
    }

    void setValueI(float val) {
        valueI = val;
        iTerm = val;
    }

    // todo: добавить dErr учёт времени (для производной)
    // http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/
    float update() {
        unsigned long now = millis();
        unsigned long timeChange = (now - lastTime);

        if (timeChange >= 100) { // Обновление каждые 100 мс
            // PID "P"
            float error = setpoint - input;
            valueP = Kp * error;

            // PID "I"
            iTerm += (Ki * error);
            iTerm = constrain(iTerm, limitImin, limitImax); // Ограничение интегральной составляющей
            valueI = iTerm;

            // PID "D"
            valueD = 0;
            if (Kd) {
                if (now - lastDTime >= dDelay) { // Сравнение сглаженных значений каждые 10 секунд
                    lastDValue = errors[errorLastIndex];
                    if (lastDValue == 0) {
                        // нивелируем ошибку D если PID только запущен и не собрал данных
                        lastDValue = input;
                    }
                    errors[errorLastIndex] = input;
                    ++errorLastIndex;
                    if (errorLastIndex >= errorsSize) {
                        errorLastIndex = 0;
                    }
                    lastDTime = now;
                }
                float dInput = input - lastDValue;
                valueD = -Kd * dInput;
            }

            output = valueP + valueI + valueD;
            output = constrain(output, 0, 1); // Ограничение выходного сигнала

            lastInput = input;
            lastTime = now;
        }

        return output;
    }
};