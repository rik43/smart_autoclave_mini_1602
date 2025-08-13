#pragma once
#include <string>

// публикация состояния соединения и наличия настроек
class EventConnection : public Event {
public:
    // активность подключения
    bool ble = false;
    bool wifi = false;
    bool server = false;
    // наличие конфигурации
    bool ble_config = false;
    bool wifi_config = false;
    // наличие активных клиентов
    bool ble_client = false;
    bool wifi_checking = false;
    bool pairing = false;

    EventConnection() : Event("connection") {}

    void toString(std::string& msg) override {
        addParam(msg, "ble", std::to_string(ble));
        addParam(msg, "wifi", std::to_string(wifi));
        addParam(msg, "server", std::to_string(server));
        
        addParam(msg, "ble_config", std::to_string(ble_config));
        addParam(msg, "wifi_config", std::to_string(wifi_config));

        addParam(msg, "ble_client", std::to_string(ble_client));
        addParam(msg, "wifi_checking", std::to_string(wifi_checking));
        addParam(msg, "pairing", std::to_string(pairing));
    }
};


