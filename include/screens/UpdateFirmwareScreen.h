#pragma once

#include "global.h"
#include "Screen.h"
#include "ui/widget/Label.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

#define SERVICE_WIFI_SSID "DZ"
#define SERVICE_WIFI_PASSWORD "Dz17042024"

// локальные адреса для отладки
#define SERVICE_UPDATE_ESP_URL "http://192.168.1.35:3535/a1-lcd/esp/firmware.bin"

  
class UpdateFirmwareScreen : public Screen {    
    private:
        Label labelStatus;
        Label labelProgress;
        Label labelProgressPercent;
        Label labelWifiRssi;
        Label labelWifiRssiText;

        int progress = 0;
        bool isStarted = false;

    public:
        UpdateFirmwareScreen() :
            labelStatus(0, 0, ""),
            labelProgress(0, 1, "---", 3),
            labelProgressPercent(3, 1, "%", 1),
            labelWifiRssiText(8, 1, "WiFi:", 5),
            labelWifiRssi(13, 1, "", 3)
        {
            labelStatus.setScrollable(true);
            labelStatus.setScrollSpeed(950);
            labelStatus.setScrollBy(16);

            labelStatus.setAlignment(ALIGN_RIGHT);

            // labelProgress.setBlink(true);
            // labelProgress.setBlinkDelay(350, 350);
            reset();
        }

        ~UpdateFirmwareScreen() {

        }

        void draw(BufferedLcd &lcd) override {
            labelStatus.print(lcd);
            labelProgress.print(lcd);
            labelProgressPercent.print(lcd);
            labelWifiRssiText.print(lcd);
            labelWifiRssi.print(lcd);
        }

        void reset() override {
            setStatus("Press button to update firmware");
            labelProgress.setText("---");
            labelProgressPercent.setText("%");
        }

        void update() override {
        }

        void onEncoderTurn2(bool isRight, bool isFast) override {
        }

        void onEncoderButtonClick() override {
            setStatus("Starting...");
            labelProgress.setText("0");
            isStarted = true;

            vTaskDelay(pdMS_TO_TICKS(1000));
            setStatus("Connecting to WiFi...");
            wifiConnect();

            vTaskDelay(pdMS_TO_TICKS(1000));
            setStatus("Updating firmware...");
            startEspUpgrade();
        }

        void setStatus(const char* status) {
            labelStatus.setText(status);
            // lcdDraw();
        }


        void startEspUpgrade() {
            // httpUpdate создан глобально в cpp файле библ
            httpUpdate.rebootOnUpdate(true);
            httpUpdate.onProgress([this](int current, int total) {
                this->onUpdateProgress(current, total);
            });
        
            const int TIMEOUT = 120000; // 120 секунд
            std::string firmwareUrl = SERVICE_UPDATE_ESP_URL;
            Serial.printf("url: %s\n", firmwareUrl.c_str());
        
            WiFiClientSecure httpsClient;
            WiFiClient httpClient;
        
            t_httpUpdate_return ret;
            if (firmwareUrl.substr(0, 5) == "https") {
                //httpsClient.setCACert(rootCACertificate);
                httpsClient.setTimeout(TIMEOUT);
                ret = httpUpdate.update(httpsClient, firmwareUrl.c_str());
            } else {
                httpClient.setTimeout(TIMEOUT);
                ret = httpUpdate.update(httpClient, firmwareUrl.c_str());
            }
        
            switch (ret) {
                case HTTP_UPDATE_FAILED: {
                    //eventUpdate.errorCode = httpUpdate.getLastError();
                    //emit(eventUpdate);
                    // getLastError: ошибки http client + 9 от update:
                    // https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPUpdate/src/HTTPUpdate.h#L35
                    // https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPUpdate/src/HTTPUpdate.cpp#L115
                    Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                    }
                    break;
            
                case HTTP_UPDATE_NO_UPDATES: {
                    Serial.println("HTTP_UPDATE_NO_UPDATES");
                    //eventUpdate.errorCode = -1000;
                    //emit(eventUpdate);
                    }
                    break;
            
                case HTTP_UPDATE_OK: {
                    Serial.println("HTTP_UPDATE_OK");
                    return; // true
                    }
                    break;
            }
        
            return; // false
        }

        void onUpdateProgress(int current, int total) {
            byte newProgress = total > 0 ? current * 100 / total : 100;
            if (newProgress <= progress) {
                return;
            }
            progress = newProgress;
            prt("progress", progress);

            // Обновляем UI 
            labelProgress.setText(progress);
            updateRssi();

            lcdDraw();
        }

        void updateRssi() {
            int rssi = WiFi.RSSI();
            labelWifiRssi.setText(rssi);
        }        
   
        bool wifiConnect() {
            wl_status_t status = WiFi.status();
            
            // Проверяем не только статус подключения, но и наличие валидного IP
            if (status == WL_CONNECTED && WiFi.localIP() != INADDR_NONE) {
                return true;
            }
        
            // Если в процессе подключения - ждем результата
            if (status == WL_IDLE_STATUS) {
                const int CONNECT_TIMEOUT = 60000; // 60 секунд
                unsigned long startTime = millis();
                
                while (millis() - startTime < CONNECT_TIMEOUT) {
                    status = WiFi.status();
                    if (status == WL_CONNECTED && WiFi.localIP() != INADDR_NONE) {
                        return true;
                    }
                    if (status != WL_IDLE_STATUS) {
                        break;
                    }
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }
        
            // Если дошли сюда - надо переподключаться
            debugln("Reconnecting to WiFi...");
            
            // Отключаемся если не в состоянии DISCONNECTED
            if (status != WL_DISCONNECTED) {
                WiFi.disconnect(true, false);  // disconnect but keep config
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        
            // Проверяем есть ли сохраненные креды
            // if (settings.getWifiSsid().length() == 0) {
            //     debugln("No saved WiFi settings");
            //     vTaskDelay(pdMS_TO_TICKS(1000));
            //     return false;
            // }
        
            // Пробуем подключиться используя сохраненные настройки
            WiFi.mode(WIFI_STA);
        
            // std::string ssid = settings.getWifiSsid();
            // std::string password = settings.getWifiPassword();
            // if (!settings.hasWifiSettings()) {
            std::string ssid = SERVICE_WIFI_SSID;
            std::string password = SERVICE_WIFI_PASSWORD;
            // }
            prt("ssid", ssid.c_str());
            prt("password", password.c_str());
        
            WiFi.begin(ssid.c_str(), password.c_str());
        
            // Ждем подключения и получения IP
            const int RECONNECT_TIMEOUT = 5000; // 5 секунд
            unsigned long startTime = millis();
            
            while (millis() - startTime < RECONNECT_TIMEOUT) {
                status = WiFi.status();
                if (status == WL_CONNECTED && WiFi.localIP() != INADDR_NONE) {
                    debugln("WiFi connected with IP");
                    return true;
                }
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        
            debugln("Failed to connect to WiFi or get IP");
            return false;
        }
        
};
