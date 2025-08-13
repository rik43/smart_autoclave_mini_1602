#include <Arduino.h>
#include <functional>
#include <vector>

// Using:
//    timer.setTimeout([]() {
//        Serial.println("5 seconds passed!");
//    }, 5000);
//
class TimerTimeout {
public:
    // Структура для хранения информации о задаче
    struct Task {
        std::function<void()> callback; // Коллбэк функция
        unsigned long time; // Время, когда коллбэк должен быть вызван
        unsigned long id; // Идентификатор задачи (можно использовать для сравнения, если > 0)
    };

private:
    std::vector<Task> tasks; // Список задач

public:
    // id для уникальности, если id>0 и задача уже в стеке, то только обновит время у нее
    void setTimeout(std::function<void()> cb, unsigned long delay, unsigned long id = 0) {
        if (id > 0) {
            // Поиск задачи с тем же идентификатором
            for (auto &task : tasks) {
                if (task.id == id) {
                    task.time = millis() + delay; // Обновить время вызова
                    task.callback = cb; // Обновить коллбэк, на случай если он изменился
                    return;
                }
            }
        }

        // Если задача с таким идентификатором не найдена, добавляем новую
        tasks.push_back({cb, millis() + delay, id});
    }

    // Проверка и выполнение коллбэков
    void loop() {
        unsigned long now = millis();
        auto it = tasks.begin();
        while (it != tasks.end()) {
            // Если текущее время больше или равно времени вызова коллбэка
            if (now >= it->time) {
                it->callback(); // Вызвать коллбэк
                it = tasks.erase(it); // Удалить задачу из списка
            }
            else {
                ++it; // сдвигаем итератор только если не удаляли задачу
            }
        }
    }

};