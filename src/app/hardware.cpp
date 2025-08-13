#include "app/hardware.h"


PWMrelay heaterRelay(PIN_RELAY, HIGH, 2000);  // HIGH — вкл высоким, период 2000 мс
SemaphoreHandle_t pwmHeaterMutex = nullptr;

void tickHeaterTask(void *parameter) {
    while (true) {
        if (xSemaphoreTake(pwmHeaterMutex, portMAX_DELAY)) { // Получаем доступ к ресурсу
            heaterRelay.tick();
            xSemaphoreGive(pwmHeaterMutex); // Освобождаем ресурс
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);  // Задержка 10 мс
    }
}
