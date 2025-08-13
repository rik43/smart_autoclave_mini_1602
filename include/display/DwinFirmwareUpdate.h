#pragma once

#include <string>
#include <algorithm>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "net/cert-le.h"
#include <mbedtls/md.h> // для md5

class DwinFirmwareUpdate {
private: 
    HardwareSerial* port;

    // номер блока (0-31 для 8mb flash. 0-63 для 16mb flash), каждый блок 256кб
    // каждый блок состоит из 8 flashBlock по 32кб
    byte blockNumber; 

    std::string flashUrl; // url bin/icl файла

    std::function<void(int, int)> updateCallback; // Callback function

    // для md5
    uint8_t calculatedhash[16]; // MD5 дает 16-байтовый хеш

public: 
    DwinFirmwareUpdate(HardwareSerial& serialPort, byte blockNumber, std::string flashUrl) 
        :port(&serialPort), blockNumber(blockNumber), flashUrl(flashUrl) {}

    void setUpdateCallback(std::function<void(int, int)> callback) {
        updateCallback = callback;
    }

    int updateBlock() {
        HTTPClient http;
        WiFiClientSecure httpsClient;
        WiFiClient httpClient;

        // Determine if the URL is HTTPS or HTTP
        if (flashUrl.substr(0, 5) == "https") {
            httpsClient.setCACert(rootCACertificate);
            //httpsClient.setInsecure(); // Отключаем проверку сертификата
            httpsClient.setTimeout(60000);
            if (!http.begin(httpsClient, flashUrl.c_str())) {
                return -901;
            }
        } else {
            httpClient.setTimeout(60000);
            if (!http.begin(httpClient, flashUrl.c_str())) {
                return -901;
            }
        }
        
        debugln("display update");

        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            int contentLength = http.getSize(); // (is -1 when Server sends no Content-Length header)
            if (contentLength <= 0) {
                http.end();
                return -902; // Ошибка: нет данных или неверный размер файла
            }

            WiFiClient* stream = http.getStreamPtr();
            if (!stream) {
                http.end();
                return -903; // Ошибка: не удалось получить поток данных
            }

            int result = sendStreamToFlash(stream, contentLength);
            if (result != 0) {
                http.end();
                return result;
            }
        } else {
            Serial.printf("request failed: %d %s\n", httpCode, http.errorToString(httpCode).c_str());
            http.end();
            return httpCode;
        }

        http.end();

        clearIncomingQueue();

        // проверяем md5
        return checkHashSuccess();
    }

    int sendStreamToFlash(WiFiClient* stream, int contentLength) {
        mbedtls_md_context_t ctx;
        mbedtls_md_type_t md_type = MBEDTLS_MD_MD5;
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
        mbedtls_md_starts(&ctx);

        prt("contentLength", contentLength);
        int totalBytesRead = 0; // сколько байт прочитано
        int totalBytesRest = contentLength; // сколько байт осталось прочитать
        uint8_t oddByte = 0; // для сохранения нечетного байта
        bool hasOddByte = false; // флаг наличия нечетного байта

        uint8_t buffer[240];
        int SEND_BUFFER_SIZE = 240; // столько байт можно отправить макс за один раз по UART (256-16)
        
        uint16_t flashBlockNumber = blockNumber * 8; 
        
        // такое кол-во байт надо загрузить в RAM и отправить команду flash
        int FLASH_BLOCK_SIZE = 32768; 
        int flashBlockSendedBytes = 0; // сколько байт блока отправили в RAM
        int flashBlockRestBytes = FLASH_BLOCK_SIZE; // сколько байт осталось до заполнения блока 32k
        unsigned long timeout = millis() + 60000; // таймаут 60 секунд

        while ((stream->connected() || stream->available()) && (totalBytesRead < contentLength)) {
            if (millis() > timeout) {
                return -904; // Ошибка: превышено время ожидания
            }

            if (stream->available()) {
                timeout = millis() + 60000; // обновляем таймаут

                // Определяем размер чанка для чтения
                int chunkSize = std::min(SEND_BUFFER_SIZE, flashBlockRestBytes);
                
                // Если у нас есть сохраненный нечетный байт, корректируем размер чанка
                if (hasOddByte) {
                    chunkSize = std::min(chunkSize, SEND_BUFFER_SIZE - 1);
                }

                // чтение из http в buffer[240]
                int bytesRead = stream->readBytes(hasOddByte ? buffer + 1 : buffer, chunkSize);

                if (bytesRead <= 0) {
                    return -905; // Ошибка чтения данных
                }

                // вывести сообщение в сериал порт если bytesRead нечетное
                //if (bytesRead % 2 != 0) {
                //    prt("bytesRead !!!!!!!!", bytesRead);
                //}

                // Если у нас был сохранен нечетный байт, добавляем его в начало буфера
                if (hasOddByte) {
                    buffer[0] = oddByte;
                    bytesRead++;
                    hasOddByte = false;
                }

                // Проверяем, нужно ли сохранить нечетный байт для следующей итерации
                // Делаем это только если это не последний чанк файла
                bool isLastChunk = (totalBytesRead + bytesRead >= contentLength);
                if (!isLastChunk && bytesRead % 2 != 0) {
                    oddByte = buffer[bytesRead - 1];
                    hasOddByte = true;
                    bytesRead--;
                }

                // ячейка переменная для записи текущего чанка
                // адрес в word. 1word = 2byte
                uint16_t flashAddress = (flashBlockSendedBytes / 2) + 0x1000;

                if (bytesRead > 0) {
                    if (!sendChunk(flashAddress, buffer, bytesRead)) {
                        return -906; // Ошибка отправки чанка
                    }

                    totalBytesRead += bytesRead;
                    totalBytesRest -= bytesRead;
                    flashBlockSendedBytes += bytesRead;
                    flashBlockRestBytes -= bytesRead;

                    if (updateCallback) {
                        updateCallback(totalBytesRead, contentLength);
                    }
                }

                if (flashBlockRestBytes <= 0) {
                    // отправить команду на запись из RAM в FLASH блока 32кб
                    prt("send block", flashBlockNumber);
                    if (!sendFlashBlock(flashBlockNumber)) {
                        return -907; // Ошибка записи флеш-блока
                    }

                    clearIncomingQueue();

                    // начинаем сборку следующего подблока 32кб
                    ++flashBlockNumber;
                    flashBlockRestBytes = FLASH_BLOCK_SIZE;
                    flashBlockSendedBytes = 0;
                }

                mbedtls_md_update(&ctx, buffer, bytesRead); // обновляем хеш md5 включая нечетный байт если он есть

            } else {
                delay(1);
            }
        }

/*
        // Если у нас остался нечетный байт на последнем чтении, отправляем его
         if (hasOddByte) {
            uint16_t flashAddress = (flashBlockSendedBytes / 2) + 0x1000;
            buffer[0] = oddByte;
            
            if (!sendChunk(flashAddress, buffer, 1)) {
                return -906; // Ошибка отправки чанка
            }
            
            totalBytesRead += 1;
            flashBlockSendedBytes += 1;
            flashBlockRestBytes -= 1;
            
            if (updateCallback) {
                updateCallback(totalBytesRead, contentLength);
            }
        }
 */
        if (flashBlockSendedBytes > 0) {
            // отправляем последний неполный блок
            prt("send block", flashBlockNumber);
            if (!sendFlashBlock(flashBlockNumber)) {
                return -907;
            }

            if (updateCallback) {
                updateCallback(totalBytesRead, contentLength);
            }
        }

        if (totalBytesRead != contentLength) {
            return -908; // Ошибка: получено неверное количество байт
        }

        mbedtls_md_finish(&ctx, calculatedhash);
        mbedtls_md_free(&ctx);


        debugln("complete");
        return 0;
    }

    int checkHashSuccess() {
        // загружать по http а не https (не хватает памяти)
        std::string md5Url = flashUrl + ".md5";
        // replace https:// на http://
        std::string::size_type pos = md5Url.find("https://");
        if (pos != std::string::npos) {
            md5Url.replace(pos, 8, "http://");
        }
        prt("md5Url", md5Url.c_str());

        std::string serverHash = loadHashFromUrl(md5Url);
        if (serverHash.empty()) {
            return -909; // Ошибка: не удалось получить хеш
        }

        if (!isValidHash(serverHash.c_str())) {
            return -910; // Ошибка: хеш не совпадает
        }

        return 0;
    }

    // загружает md5 из url get запросом и возвращает его
    // работает с https и http
    std::string loadHashFromUrl(const std::string& url) {
        HTTPClient http;
        WiFiClientSecure httpsClient;
        WiFiClient httpClient;

        if (url.substr(0, 5) == "https") {
            httpsClient.setCACert(rootCACertificate);
            httpsClient.setTimeout(60000);
            if (!http.begin(httpsClient, url.c_str())) {
                return "";
            }
        } else {
            httpClient.setTimeout(60000);
            if (!http.begin(httpClient, url.c_str())) {
                return "";
            }
        }

        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String responseHash = http.getString();
            http.end();
            return responseHash.c_str();
        }
        http.end();
        return "";
    }

    bool isValidHash(const char* expectedMD5) {
        char hashStr[33];
        for(int i = 0; i < 16; i++) {
            sprintf(&hashStr[i*2], "%02x", calculatedhash[i]);
        }
        hashStr[32] = '\0';
        prt("hash client", hashStr);
        prt("hash server", expectedMD5);

        return strcmp(hashStr, expectedMD5) == 0;
    }

    bool sendChunk(uint16_t address, const uint8_t* data, byte dataLengthBytes) {
        port->write(0x5A);
        port->write(0xA5);
        port->write((uint8_t)dataLengthBytes + 3);
        port->write(0x82);
        port->write((address >> 8) & 0xFF);
        port->write(address & 0xFF);
        port->write(data, dataLengthBytes);
        port->flush(); // flush tx & rx
        return true;
    }

    // отправить команду на запись из RAM в FLASH блока 32кб
    bool sendFlashBlock(uint16_t flashBlock) {
        uint8_t command[] = {
            0x5A, 0xA5, 
            0x0F, // packet length
            0x82, // write command
            0x00, 0xAA, // 0x00AA - write data address
            0x5A, 0x02, // D11 D10 // enable, 0x02 - write 32kb block
            0x00, 0x00, // D9  D8  // command[8/9] - flash_block - номер блока для записи 0-255 для 8Мб
            0x10, 0x00, // D7  D6  // первый адрес ячейки откуда взять данные для перемещения (блок 32кб)
            0x17, 0x70, // D5  D4  // 1770h = 6000 ms, ждать след операции записи (не перезагружаться)
            0x00, 0x00, // D3  D2  // undefined
            0x00, 0x00, // D1  D0  // undefined
        };
        command[8] = (flashBlock >> 8) & 0xFF;
        command[9] = (flashBlock) & 0xFF;

        port->write(command, sizeof(command));
        port->flush();
        delay(30); // 20ms recommended. time to write flash (or send var request and wait response)

        return true;
    }

    // очищает входящие в сериал порт данные и возвращает их количество
    int clearIncomingQueue() {
        int count = 0;
        while (port->available()) {
            char receivedChar = port->read();
            ++count;
        }
        return count;
    }

};



