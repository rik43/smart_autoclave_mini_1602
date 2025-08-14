#pragma once
#include <stdint.h>

enum class InputEventDevice : uint8_t { Encoder, EncoderButton, System };

enum class EncoderAction : uint8_t { Turn };
enum class EncoderButtonAction  : uint8_t { Click, LongPress };
enum class SystemAction  : uint8_t { Tick, Timeout };
// enum class AnalogAction  : uint8_t { Change };
// enum class TouchAction   : uint8_t { Down, Up, Tap, Move, LongPress, Swipe };

struct EncoderEvent { bool isRight; bool isFast; };
struct EncoderButtonEvent  { EncoderButtonAction action; };
struct SystemEvent  { SystemAction action; };
// struct AnalogEvent  { uint16_t value; uint16_t prev; };
// struct TouchEvent   { uint16_t x; uint16_t y; TouchAction action; };

struct InputEvent {
  InputEventDevice device;

  union {
    EncoderEvent encoder;
    EncoderButtonEvent  button;
    SystemEvent  system;
  } data;
};