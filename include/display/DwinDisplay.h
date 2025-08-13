#pragma once

#include <vector>
#include "buffer.h"

class DwinDisplay {
private:
  HardwareSerial* port;
  bool isPrintDebug = true;
  uint32_t lastCharTime = 0;

  // callback на получение значения слова (при изменении и считывании)
  //typedef void (*VarCallback)(uint16_t address, uint16_t value, uint8_t* buffer, uint8_t bufferLength);
  using VarCallback = std::function<void(uint16_t, uint16_t, uint8_t*, uint8_t)>;

  enum PacketReadState {
    HEADER_BYTE_1, // ждем первый байт заголовка
    HEADER_BYTE_2, // ждем второй байт заголовка
    HEADER_DATA_LENGTH,  // ждем байт длины данных
    //COMMAND_BYTE, 
    DATA_CONTENT, // чтение данных в буфер
  } readState = HEADER_BYTE_1;

  const uint8_t maxBufferSize = 32;  // Максимальный размер сохраняемой части пакета в байтах
  uint8_t buffer[32];
  uint8_t packetDataSize = 0; // размер данных из заголовка
  uint8_t packetDataReaded = 0; // кол-во уже прочитанных данных
  uint8_t bufferDataSize = 0; // размер прочитанных данных в буфере (до maxBufferSize)

  VarCallback onVarCallback;  // Указатель на функцию коллбэка - приёма данных

  void resetBuffer() {
    for (uint8_t i = 0; i < maxBufferSize; i++) {
      buffer[i] = 0;
    }
    packetDataReaded = 0;
    bufferDataSize = 0;
  }

  void printBufferAll() {
    printBuffer(buffer, bufferDataSize);
  }

  void printBuffer(const uint8_t* buf, uint8_t size) {
    char hexBuffer[3];
    for (uint8_t i = 0; i < size; i++) {
      sprintf(hexBuffer, "%02X", buf[i]);
      Serial.print(hexBuffer);
      Serial.print('.');
    }
    Serial.println();
  }

public:
  DwinDisplay(HardwareSerial& serialPort) : port(&serialPort) {}

  //void setVarCallback(VarCallback callback) {
  void setVarCallback(const VarCallback& callback) {
    onVarCallback = callback;
  }

  void loop() {
    while (port->available()) {
      char receivedChar = port->read();

      switch (readState) {
      case HEADER_BYTE_1:
        if (receivedChar == 0x5A) {
          readState = HEADER_BYTE_2;
        }
        break;
      case HEADER_BYTE_2:
        if (receivedChar == 0xA5) {
          readState = HEADER_DATA_LENGTH; // заголовок пакета верный
        }
        else {
          readState = HEADER_BYTE_1; // второй байт не верный - ждем нового заголовка сначала
        }
        break;
      case HEADER_DATA_LENGTH:
        packetDataSize = receivedChar;
        resetBuffer();
        readState = DATA_CONTENT;
        break;
      case DATA_CONTENT:
        // сохраняем пока буффер не заполнится
        /*if (packetDataReaded == 0) {
          command = static_cast<uint8_t>(receivedChar);
        }*/
        if (packetDataReaded < maxBufferSize) {
          buffer[packetDataReaded] = static_cast<uint8_t>(receivedChar);
          ++bufferDataSize;
        }
        else {
          // skip left bytes
        }
        ++packetDataReaded;
        if (packetDataReaded == packetDataSize) {
          readState = HEADER_BYTE_1; // ждем следующий пакет
          // обработать данные в пакете
          handlePacketReceived();
        }
        break;
      }
    }

    // если данных долго нет - сброс недопринятой команды
    if (lastCharTime && (millis() - lastCharTime > 500)) {
      readState = HEADER_BYTE_1;
      resetBuffer();
      lastCharTime = 0;
    }
    else {
      lastCharTime = millis();
    }
  }

  void sendReset() {
    uint32_t cmd = 0x55AA5AA5;
    setVar(0x0004, cmd);
  }

  // timeMs время звука (макс 2048мс = 2сек)
  void beep(uint16_t timeMs = 512) {
    uint16_t cycles = timeMs / 8;
    setVar(0x00A0, cycles); 
  }

  void longBeep() {
    beep(1024); 
  }

  // пакет полностью получен - обработать buffer
  void handlePacketReceived() {
    if (bufferDataSize == 0) return;

    printBufferAll();

    if (buffer[0] == 0x83) { // Read data
      handleVarReceived(buffer + 1, bufferDataSize - 1); // skip 0x83 command byte
    }

    resetBuffer();
  }

  // обработать пришедшее значение переменной
  void handleVarReceived(uint8_t* buffer, uint8_t bufferSize) {
    // Request  5A A5 04  83 1000 01
    // Response 5A A5 06  83 1000 01  0002 
    uint16_t address = buffer_get_uint16(buffer); // адрес первой ячейки
    uint8_t wordsCount = buffer[2];

    // после 3х байт (адрес и размер) дальше идут данные, значения ячеек. (может быть несколько, если запрашивалось)   
    buffer += 3;
    bufferSize -= 3;

    // Передаем каждое слово в пользовательскую функцию обработчик - переменных
    // Также передаем буфер, чтобы можно было считывать переменные другой длины
    for (int i = 0; i < wordsCount; i++) {
      uint16_t value = buffer_get_uint16(buffer); // очередное значение из буфера
      callVarCallback(address, value, buffer, bufferSize);
      // каждые следующие 2 байта - новое слово со следующим адресом
      buffer += 2;
      bufferSize -= 2;
      address++;
      if (bufferSize <= 1) { // если передано больше данных чем наш буфер
        break;
      }
    }
  }

  void callVarCallback(uint16_t address, uint16_t value, uint8_t* buffer, uint8_t bufferLength) {
    if (onVarCallback) {
      onVarCallback(address, value, buffer, bufferLength);
    }
  }

  void sendData(const uint8_t* data, size_t length) {
    port->write(0x5A);
    port->write(0xA5);
    port->write((uint8_t)length);
    port->write(data, length);

    //printSendBuffer(data, length);
  }

  //void printSendBuffer(const uint8_t* data, size_t length) {
  //  char hexBuffer[3];
  //  Serial.print("SEND: ");
  //  for (uint8_t i = 0; i < length; i++) {
  //    sprintf(hexBuffer, "%02X", data[i]);
  //    Serial.print(hexBuffer);
  //    Serial.print('.');
  //  }
  //  Serial.println();
  //}

  void sendReadVar(uint16_t address, uint8_t wordsLength = 1) {
    const uint8_t data[] = {
      0x83, // Command: Read
      static_cast<uint8_t>((address >> 8) & 0xFF),
      static_cast<uint8_t>(address & 0xFF),
      wordsLength // кол-во слов (по 2 байта) для чтения (можно читать несколько ячеек)
    };
    sendData(data, sizeof(data));
  }

  // запишет uint8_t в младший байт
  void setVar(uint16_t address, uint8_t value) {
    const uint8_t data[] = {
      0x82, // command: Write
      // address:
      static_cast<uint8_t>((address >> 8) & 0xFF),
      static_cast<uint8_t>(address & 0xFF),
      // value:
      0,
      value,
    };
    sendData(data, sizeof(data));
  }

  // записать слово из двух байт
  //void setVar(uint16_t address, uint8_t hight, uint8_t low) {
  //  const uint8_t data[] = {
  //    0x82, // command: Write
  //    // address:
  //    static_cast<uint8_t>((address >> 8) & 0xFF),
  //    static_cast<uint8_t>(address & 0xFF),
  //    // value:
  //    hight,
  //    low,
  //  };
  //  sendData(data, sizeof(data));
  //}

  void setVar(uint16_t address, uint32_t value) {
    const uint8_t data[] = {
      0x82, // command: Write
      // address:
      static_cast<uint8_t>((address >> 8) & 0xFF),
      static_cast<uint8_t>(address & 0xFF),
      // value:
      static_cast<uint8_t>((value >> 24) & 0xFF),
      static_cast<uint8_t>((value >> 16) & 0xFF),
      static_cast<uint8_t>((value >> 8) & 0xFF),
      static_cast<uint8_t>(value & 0xFF),
    };
    sendData(data, sizeof(data));
  }

  // Рекурсивная вспомогательная функция для добавления значений в вектор
  // Обработка одного аргумента (базовый случай рекурсии)
  template<typename T>
  void appendValues(std::vector<uint8_t>& data, T value) {
    data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(value & 0xFF));
  }

  // Обработка двух и более аргументов
  template<typename T, typename... Args>
  void appendValues(std::vector<uint8_t>& data, T first, Args... args) {
    appendValues(data, first); // Добавляем первый аргумент
    appendValues(data, args...); // Рекурсивно обрабатываем оставшиеся аргументы
  }

  void appendValues(std::vector<uint8_t>& data) {
    // Ничего не делаем, рекурсия останавливается
  }

  // Версия функции setVar для переменного количества аргументов uint16
  // если переданы не uint16 то они все равно будут рассматриваться как word
  // из каждого будет взято 2 младших байта
  template<typename... Args>
  void setVar(uint16_t address, Args... args) {
    std::vector<uint8_t> data = {
        0x82, // command: Write
        static_cast<uint8_t>((address >> 8) & 0xFF),
        static_cast<uint8_t>(address & 0xFF),
    };

    appendValues(data, args...); // Добавляем значения в вектор

    sendData(data.data(), data.size()); // Отправляем данные
  }

  void setPage(uint16_t page) {
    Serial.printf("PAGE %u\n", page);
    setVar(0x84, 0x5A01, page);
  }
};



/*  void setVar(uint16_t address, uint16_t value) {
    const uint8_t data[] = {
      0x82, // command: Write
      // address:
      static_cast<uint8_t>((address >> 8) & 0xFF),
      static_cast<uint8_t>(address & 0xFF),
      // value:
      static_cast<uint8_t>((value >> 8) & 0xFF),
      static_cast<uint8_t>(value & 0xFF),
    };
    sendData(data, sizeof(data));
  }*/

  /*  // send 2 words
    void setVar(uint16_t address, uint16_t value1, uint16_t value2) {
      const uint8_t data[] = {
        0x82, // command: Write
        // address:
        static_cast<uint8_t>((address >> 8) & 0xFF),
        static_cast<uint8_t>(address & 0xFF),
        // value:
        static_cast<uint8_t>((value1 >> 8) & 0xFF),
        static_cast<uint8_t>(value1 & 0xFF),
        static_cast<uint8_t>((value2 >> 8) & 0xFF),
        static_cast<uint8_t>(value2 & 0xFF),
      };
      sendData(data, sizeof(data));
    }*/
