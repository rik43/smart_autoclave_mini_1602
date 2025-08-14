#pragma once

#define FIRMWARE_VERSION            24 // версия прошивки (видима извне)
#define HARDWARE_VERSION_DEFAULT    10 // версия платы (только чтобы сохранить в preferences)

#define PIN_RELAY 32

// MAX6675 pinout (SPI)
#define CLK_PIN   14  // Пин SCK
#define DATA_PIN  12  // Пин SO
#define CS_PIN    15  // Пин CS


#define FACTORY_TEMPERATURE_CALIBRATION     -2.0f  // заводская калибровка фиксируется и хранится в NVS
#define TEMPERATURE_CALIBRATION_MIN         -5.0f  // максимальное допустимое отриц отклонение калибровки
#define TEMPERATURE_CALIBRATION_MAX         3.0f   // максимальное допустимое положительное отклонение калибровки

#define FACTORY_HEATER_MAX_POWER            0.75f  // макс мощность нагрева (для инициализации NVS)

// пределы допустимых значений темп-ры (при выходе за диапаон - выкл нагрев и вывести ошибку)
#define MAX_ALLOWED_TEMPERATURE     130.0f
#define MIN_ALLOWED_TEMPERATURE     5.0f

#define MAX_SEQUENTAL_ERRORS_TO_FAIL     6 // максимальное кол-во ошибок для выключения программы

#define DESCRIPTOR_TYPE     "autoclave"
#define DESCRIPTOR_MODEL    "A1"

// LEDs  + is ON / - is OFF
// Resistors 150 Ohm
#define LED_COUNT    2

#define LED_1_PIN    18
#define LED_2_PIN    25

#define HOT_WARNING_TEMPERATURE             80.0f // град, темп-ра предупреждения о горячем и необходимости опускания клапана

#define ERROR_CODE_NOT_VALID_TEMPERATURE    1

// коды отображения сообщений на дисплее
#define DISPLAY_COMPLETE 1 // complete screen


#define _LCD_TYPE               1      // 1 - по шине I2C, 2 - десятиконтактное (для LCD_1602_RUS_ALL)
#define LCD_ADDRESS             0x27
#define LCD_WIDTH               16
#define LCD_HEIGHT              2

#define ENCODER_S1              4
#define ENCODER_S2              19
#define ENCODER_KEY             23

