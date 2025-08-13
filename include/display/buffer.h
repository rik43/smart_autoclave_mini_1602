#pragma once

#ifndef BUFFER_H
#define BUFFER_H

uint8_t buffer_get_uint8(const uint8_t* buffer);

// старший байт слова
uint8_t buffer_get_uint8_high(const uint8_t* buffer);

// младший байт слова
uint8_t buffer_get_uint8_low(const uint8_t* buffer);

// старший байт слова
int8_t buffer_get_int8_high(const uint8_t* buffer);

// младший байт слова
int8_t buffer_get_int8_low(const uint8_t* buffer);

uint16_t buffer_get_uint16(const uint8_t* buffer);

int16_t buffer_get_int16(const uint8_t* buffer);

uint32_t buffer_get_uint32(const uint8_t* buffer);

int32_t buffer_get_int32(const uint8_t* buffer);

float buffer_get_float32(const uint8_t* buffer);

#endif // BUFFER_H
