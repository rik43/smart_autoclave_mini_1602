#include <Arduino.h>
#include "display/buffer.h"

uint8_t buffer_get_uint8(const uint8_t* buffer) {
  return buffer[0];
}

// старший байт слова
uint8_t buffer_get_uint8_high(const uint8_t* buffer) {
  return buffer[0];
}

// младший байт слова
uint8_t buffer_get_uint8_low(const uint8_t* buffer) {
  return buffer[1];
}

// старший байт слова
int8_t buffer_get_int8_high(const uint8_t* buffer) {
  return static_cast<int8_t>(buffer[0]);
}

// младший байт слова
int8_t buffer_get_int8_low(const uint8_t* buffer) {
  return static_cast<int8_t>(buffer[1]);
}

uint16_t buffer_get_uint16(const uint8_t* buffer) {
  return (static_cast<uint16_t>(buffer[0]) << 8) | buffer[1];
}

int16_t buffer_get_int16(const uint8_t* buffer) {
  return (static_cast<int16_t>(buffer[0]) << 8) | buffer[1];
}

uint32_t buffer_get_uint32(const uint8_t* buffer) {
  return (static_cast<uint32_t>(buffer[0]) << 24) |
          (static_cast<uint32_t>(buffer[1]) << 16) |
          (static_cast<uint32_t>(buffer[2]) << 8) |
          buffer[3];
}

int32_t buffer_get_int32(const uint8_t* buffer) {
  return (static_cast<uint32_t>(buffer[0]) << 24) |
          (static_cast<uint32_t>(buffer[1]) << 16) |
          (static_cast<uint32_t>(buffer[2]) << 8) |
          buffer[3];
}

float buffer_get_float32(const uint8_t* buffer) {
  uint32_t rawValue = buffer_get_uint32(buffer);
  //printBuffer(reinterpret_cast<const uint8_t*>(&rawValue), 4);

  float result;
  memcpy(&result, &rawValue, sizeof(float));

  return result;
}

