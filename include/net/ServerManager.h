#pragma once

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <string.h>
#include "net/cert-le.h"
#include "global.h"


class ServerManager {
public:
    ServerManager() : wsPort(443), wifiConnected(false), wsConnected(false), isAuth(false), enabled(false),
                     reconnectWiFiInterval(10000), reconnectWebSocketInterval(15000),
                     connectWebSocketDelay(1000),
                     lastWiFiReconnectAttempt(0), lastWebSocketReconnectAttempt(0) {}

    // Настройка Wi-Fi
    void wifiSetup(const std::string& ssid_, const std::string& password_) {
        ssid = ssid_;
        password = password_;
    }

    // Установка токена авторизации
    void setToken(const std::string& _token) {
        token = _token;
    }

    // Установка параметров WebSocket-сервера
    void setServer(const std::string& host, const std::string& protocol, int port) {
        wsHost = host;
        wsProtocol = protocol;
        wsPort = port;
    }

    // Запуск менеджера
    void start() {
        Serial.println("start wifi");

        if (wifiConnected) {
            stop();
        }

        enabled = true;

        //if (! ssid.empty()) {
        //    connectWiFi();
        //}
    }

    // Остановка менеджера
    void stop() {
        Serial.println("stop wifi");
        wifiConnected = false;
        enabled = false;
        
        webSocket.disconnect();
        delay(50);
        webSocket.loop(); // catch disconnect event
        delay(50);
        WiFi.disconnect();
        delay(50);
    }

    bool isOnline() {
        return isAuth && wsConnected && wifiConnected;
    }

    bool isWifiConnected() {
        return wifiConnected;
    }

    // Отправка сообщения
    void send(const char* message) {
        if (isOnline() && webSocket.isConnected()) {
            webSocket.sendTXT(message);
        }
    }

    void send(const std::string& message) {
      send(message.c_str());
    }

    // Главный цикл для обработки соединений
    void loop() {
        bool currentWifiStatus = WiFi.status() == WL_CONNECTED;

        if (! enabled) return;

        // Проверка изменения статуса Wi-Fi
        if (currentWifiStatus != wifiConnected) {
            wifiConnected = currentWifiStatus;
            if (!wifiConnected) {
                Serial.println("Wi-Fi OFF");
                if (wsConnected) webSocket.disconnect();
                wsConnected = false;
                isAuth = false;
            } else {
                Serial.println("Wi-Fi ON");
                wifiConnectTime = millis();
            }
        }

        if (!wifiConnected) {
            if (millis() - lastWiFiReconnectAttempt >= reconnectWiFiInterval) {
                Serial1.println("loop connectWiFi");
                connectWiFi();
            }
        } else if (millis() - wifiConnectTime > connectWebSocketDelay) {
            // после включения wi-fi необходима задержка, иначе перезагружается (loop также нельзя)
            if (!wsConnected) {
                if (millis() - lastWebSocketReconnectAttempt >= reconnectWebSocketInterval) {
                    connectWebSocket();
                }
            }

            webSocket.loop();
        }
    }

private:
    // Параметры Wi-Fi
    std::string ssid;
    std::string password;

    // Параметры авторизации и WebSocket
    std::string token;
    std::string wsHost; // например, app.smartzag.ru
    std::string wsProtocol; // например, wss
    int wsPort;

    // Статусы соединений
    bool enabled;
    bool wifiConnected;
    bool wsConnected;
    bool isAuth;

    // Объект WebSocket клиента
    WebSocketsClient webSocket;

    // Интервалы переподключения (мс)
    const unsigned long reconnectWiFiInterval;
    const unsigned long reconnectWebSocketInterval;

    const unsigned long connectWebSocketDelay; // задержка после включения wifi перед подключением к websocket.
    unsigned long wifiConnectTime; // время включения wifi

    // Время последней попытки переподключения
    unsigned long lastWiFiReconnectAttempt;
    unsigned long lastWebSocketReconnectAttempt;

    // Подключение к Wi-Fi
    void connectWiFi() {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Wi-Fi already connected");
            return;
        }

        Serial.printf("Begin connecting to SSID: %s...\n", ssid.c_str());
        WiFi.begin(ssid.c_str(), password.c_str());
        // Auto reconnect is set true as default

        lastWiFiReconnectAttempt = millis();
    }

    // Подключение к WebSocket
    void connectWebSocket() {
        if (!wifiConnected) {
            return;
        }

        webSocket.setReconnectInterval(reconnectWebSocketInterval);

        bool isSecure = (wsProtocol == "wss");
        Serial.printf("Connect to WebSocket: %s://%s:%d (SSL: %s)\n", 
            wsProtocol.c_str(), wsHost.c_str(), wsPort, isSecure ? "yes" : "no");

        if (isSecure) {
            webSocket.beginSslWithCA(wsHost.c_str(), wsPort, "/", rootCACertificate, wsProtocol.c_str());
        } else {
            webSocket.begin(wsHost.c_str(), wsPort, "/", wsProtocol.c_str());
        }

        // Установка коллбеков
        webSocket.onEvent([this](WStype_t type, uint8_t* payload, size_t length) {
            this->onWebSocketEvent(type, payload, length);
        });

        lastWebSocketReconnectAttempt = millis();
    }

    // Обработка событий WebSocket
    void onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
        switch (type) {
            case WStype_DISCONNECTED:
                Serial.println("WebSocket disconnected event");
                wsConnected = false;
                isAuth = false;
                break;

            case WStype_CONNECTED:
                Serial.println("WebSocket connected event");
                wsConnected = true;
                sendAuth();
                break;

            case WStype_TEXT:
                handleMessage(payload, length);
                break;

            case WStype_ERROR:
                Serial.printf("WebSocket error: %s\n", payload);
                wsConnected = false;
                isAuth = false;
                break;

            default:
                break;
        }
    }

    // Отправка сообщения авторизации
    void sendAuth() {
        if (token.empty()) {
            Serial.println("no token");
            return;
        }

        std::string msg = "@auth@role=device&token=" + token;
        webSocket.sendTXT(msg.c_str());
    }

    // Обработка входящих сообщений
    void handleMessage(uint8_t* payload, size_t length) {
        // старый способ, может приводить к переполнению стека
        // char message[length + 1]; // Добавляем место для нулевого символа
        // memcpy(message, payload, length);
        // message[length] = '\0'; // Добавляем нулевой символ
        // dispatch(message);

        std::string message(reinterpret_cast<char*>(payload), length);

        if (handleInternalMessage(message.c_str())) {
            return;
        }

        dispatch(message.c_str());
    }

    // Обработка внутренних сообщений (например, авторизация)
    bool handleInternalMessage(const char* message) {
        if (strncmp(message, "@auth-ok@", 9) == 0) {
            isAuth = true;
            return true;
        }
        else if (strncmp(message, "@auth-fail@", 11) == 0) {
            isAuth = false;
            return true;
        }
        else {
            return false;
        }
    }
};
