#pragma once

// ---------------- ЭКРАНЫ ----------------
#define DWIN_PAGE_SPLASH_SCREEN     0   // экран заставки
#define DWIN_PAGE_HOME              1   // главный экран
#define DWIN_PAGE_ERROR             15  // экран с выводом кода ошибки
#define DWIN_PAGE_FIRMWARE_UPDATE   20  // экран обновление ПО

// Экраны завершения
#define DWIN_PAGE_COMPLETE_HOT      13  // окончание готовки (т-ра > 80)
#define DWIN_PAGE_COMPLETE_COLD     14  // окончание готовки (т-ра < 80)
#define DWIN_PAGE_STOPPED_HOT       19  // экран при ручной остановке (т-ра > 80)
#define DWIN_PAGE_STOPPED_COLD      30  // экран при ручной остановке (т-ра < 80)
#define DWIN_PAGE_COMPLETE_SUVID    36  // экран завершения сувид-рецепта (просто "Готово")
#define DWIN_PAGE_COMPLETE          36  // универсальный экран завершения(просто "Готово")

#define DWIN_PAGE_RECIPE_BOOK_1     2   // книга рецептов стр 1
#define DWIN_PAGE_RECIPE_BOOK_2     3   // книга рецептов стр 2
#define DWIN_PAGE_RECIPE_BOOK_3     4   // книга рецептов стр 3

#define DWIN_PAGE_RECIPE_PREVIEW    5   // экран просмотра рецепта (из книги рецептов)
#define DWIN_PAGE_RECIPE_SELF       6   // экран настройки собственного рецепта (свой режим)

#define DWIN_PAGE_SUVID             34  // экран просмотра сувид-рецепта
#define DWIN_PAGE_SUVID_TIME        33  // экран ввода сувид-времени
#define DWIN_PAGE_SUVID_TEMP        32  // экран ввода сувид-температуры

#define DWIN_PAGE_DISTILL_INACTIVE  7   // экран дистилляция остановлена
#define DWIN_PAGE_DISTILL_ACTIVE    8   // экран дистилляция запущена

#define DWIN_PAGE_CONNECTION_INFO   10  // соединение: главный (выключено)
#define DWIN_PAGE_CONNECTION_WAIT   11  // соединение: ожидание подключения
#define DWIN_PAGE_CONNECTION_ACTIVE 12  // соединение: подключено

// Экраны для переключения из "программы" рецепта
// При доб-ии доабвить в Display.h updateProgramPage()
// Экраны рецептов
#define DWIN_PAGE_PROGRAM_10        17  // Deprecated: универсальный для готовки (нагрев)
#define DWIN_PAGE_PROGRAM_11        16  // универсальный для готовки по шагам

// экраны дистилляции
#define DWIN_PAGE_PROGRAM_21        7   // дистилляция на паузе
#define DWIN_PAGE_PROGRAM_22        8   // дистилляция в работе

#define DWIN_PAGE_WATER_CHECK       31   // экран проверки наличия воды для готовки (общий для обоих режимов)
// -------------- /END ЭКРАНЫ --------------

// Типы шагов
//#define DWIN_STEP_TYPE_OFF           0   // нагрев выключен
//#define DWIN_STEP_TYPE_WARMUP        1   // нагрев
//#define DWIN_STEP_TYPE_COOLING       2   // охлаждение
//#define DWIN_STEP_TYPE_HOLD          3   // готовка
//#define DWIN_STEP_TYPE_POWER         4   // постоянная мощность

//#define DWIN_TIME_TYPE_LEFT          0   // время осталось
//#define DWIN_TIME_TYPE_PASSED        1   // время прошло


// -------------- ПЕРЕМЕННЫЕ --------------
#define DWIN_VAR_BUTTON             0x5000 // [R] нажатия всех клавиш в этой переменной
#define DWIN_VAR_ERROR_CODE         0x5001 // [W] переменная: код ошибки
#define DWIN_VAR_RECIPE_NUMBER      0x5002 // [R] номер выбранного рецепта на дисплее
#define DWIN_VAR_VERSION_DISPLAY    0x5004 // [R] версия дисплея  (обновляется)
#define DWIN_VAR_VERSION_FIRMWARE   0x5005 // [W] версия прошивки (обновляется)
#define DWIN_VAR_VERSION_HARDWARE   0x5006 // [W] версия платы (фикс)
#define DWIN_VAR_BRIGHTNESS         0x5007 // [R] яркость дисплея
#define DWIN_VAR_STOPPED_STEP       0x5008 // [W] шаг остановки
#define DWIN_VAR_STOPPED_MINUTE     0x5009 // [W] минута остановки с начала шага
#define DWIN_VAR_STATUS_ICONS       0x5020 // [W] иконки в статус бар (5x next words)

// DWIN VAR текущая температура с датчика и оставшееся время
#define DWIN_VAR_CURRENT_TIME_REST      0x5050 // [W] время по программе (deprecated)
#define DWIN_VAR_CURRENT_TIME_HHMM      0x5056 // [W] время по программе в формате HH:MM (прошло/осталось)
#define DWIN_VAR_CURRENT_TEMPERATURE    0x5051 // [W] температура с датчика
#define DWIN_VAR_STEP_NUMBER            0x5052 // [W] номер шага
#define DWIN_VAR_STEP_COUNT             0x5053 // [W] кол-во шагов
#define DWIN_VAR_STEP_TYPE              0x5054 // [W] тип шага, код иконки(нагрев/готовка/охлад)
#define DWIN_VAR_TIME_TYPE              0x5055 // [W] тип времени, код иконки(прошло/осталось)

// DWIN VAR темп/время/мощность заданные пользователем
#define DWIN_VAR_CUSTOM_TEMPERATURE     0x5060 // [R] выбор пользователя: температура
#define DWIN_VAR_CUSTOM_TIME            0x5061 // [R] выбор пользователя: время
#define DWIN_VAR_CUSTOM_POWER           0x5062 // [R] выбор пользователя: мощность

#define DWIN_VAR_CUSTOM_SUVID_TEMPERATURE       0x5070 // [R] выбор пользователя: температура (сувид)
#define DWIN_VAR_CUSTOM_SUVID_TIME_HOURS        0x5071 // [R] выбор пользователя: часы (сувид)
#define DWIN_VAR_CUSTOM_SUVID_TIME_MINUTES      0x5072 // [R] выбор пользователя: минуты (сувид)
#define DWIN_VAR_CUSTOM_SUVID_TIME              0x5073 // [R] выбор пользователя: время (вывод в формате 00:00, значение передается в HEX 0xHHMM)

#define DWIN_VAR_FIRMWARE_PROGRESS      0x50FF // [W] прогресс обновления ПО %
// ----------- /END ПЕРЕМЕННЫЕ -----------

// ---------------- ИКОНКИ ----------------
#define DWIN_ICON_EMPTY                 0 // пусто
#define DWIN_ICON_BLE                   1 // ble доступен для подключения
#define DWIN_ICON_BLE_CONNECTED         2 // ble подключен клиент
#define DWIN_ICON_WIFI                  3 // wifi вкл
#define DWIN_ICON_CLOUD                 4 // cloud online
#define DWIN_ICON_WIFI_4                3 // wifi сигнал 4 деления (полная иконка)
#define DWIN_ICON_WIFI_3                5 // wifi сигнал 3 деления
#define DWIN_ICON_WIFI_2                6 // wifi сигнал 2 деления
#define DWIN_ICON_WIFI_1                7 // wifi сигнал 1 деление

// ------------- /END ИКОНКИ -------------


// ---------------- КНОПКИ ----------------
#define DWIN_BTN_START                  0x10 // запуск готовки
#define DWIN_BTN_STOP                   0x11 // стоп готовки.

#define DWIN_BTN_START_POWER            0x12 // запуск дистиляции (по мощности)
#define DWIN_BTN_OWN_RECIPE             0x13 // переход на "свой рецепт"
#define DWIN_BTN_RECIPE_BACK            0x15 // назад из рецепта
#define DWIN_BTN_HOME                   0x16 // переход на экран домой

#define DWIN_BTN_PRESTART_SELF          0x17 // перейти на экран подтверждения запуска при готовке (свой режим)
#define DWIN_BTN_PRESTART_PROGRAM       0x18 // перейти на экран подтверждения запуска при готовке (из книги рецептов)
#define DWIN_BTN_PRESTART_POWER         0x19 // перейти на экран подтверждения запуска при дистилляции (по мощности)
#define DWIN_BTN_WATER_SUBMIT           0x1A // подтвердить наличие воды при готовке (реальный запуск)
#define DWIN_BTN_WATER_CANCEL           0x1B // отмена подтверждения наличия воды при готовке (вернуться назад)
#define DWIN_BTN_PRESTART_SUVID         0x1C // перейти на экран подтверждения запуска при сувид-рецепте

// Экран "соединение"
#define DWIN_BTN_PAIRING_SCREEN         0x3F // вход на экран соединения
#define DWIN_BTN_PAIRING_START          0x30 // нажал подключиться
#define DWIN_BTN_PAIRING_STOP           0x31 // остановил подключение
#define DWIN_BTN_PAIRING_FORGET         0x32 // нажал отключиться

// Слайдеры +/-
#define DWIN_BTN_TIME_SLIDER_DEC        0x61
#define DWIN_BTN_TIME_SLIDER_INC        0x62
#define DWIN_BTN_TEMPERATURE_SLIDER_DEC 0x63
#define DWIN_BTN_TEMPERATURE_SLIDER_INC 0x64
#define DWIN_BTN_POWER_SLIDER_DEC       0x65
#define DWIN_BTN_POWER_SLIDER_INC       0x66

// Слайдеры +/- (сувид)
#define DWIN_BTN_SUVID_HOURS_SLIDER_DEC         0x67
#define DWIN_BTN_SUVID_HOURS_SLIDER_INC         0x68
#define DWIN_BTN_SUVID_MINUTES_SLIDER_DEC       0x69
#define DWIN_BTN_SUVID_MINUTES_SLIDER_INC       0x6A
#define DWIN_BTN_SUVID_TEMPERATURE_SLIDER_DEC   0x6B
#define DWIN_BTN_SUVID_TEMPERATURE_SLIDER_INC   0x6C

// Кнопки Другие
#define DWIN_BTN_STOP_REJECTION                 0x70 // кнопка НЕТ (при отклонении остановки прогр)
#define DWIN_BTN_COMPLETE_OK                    0x71 // кнопка ОК (при окончании сувид-рецепта)
// -------------- /END КНОПКИ --------------


// ---------------- ЛИМИТЫ СЛАЙДЕРОВ ----------------
// пределы установки времени
#define MAX_CUSTOM_TIME             120
#define MIN_CUSTOM_TIME             0

// пределы установки температуры
#define MAX_CUSTOM_TEMPERATURE      122
#define MIN_CUSTOM_TEMPERATURE      30

// пределы и шаг ввода мощности
#define MAX_CUSTOM_POWER            100
#define MIN_CUSTOM_POWER            0
#define STEP_CUSTOM_POWER           1

// пределы установки времени (сувид)
#define MAX_CUSTOM_TIME_SUVID_HOURS   72
#define MIN_CUSTOM_TIME_SUVID_HOURS   0
#define MAX_CUSTOM_TIME_SUVID_MINUTES 59
#define MIN_CUSTOM_TIME_SUVID_MINUTES 0

// пределы установки температуры (сувид)
#define MAX_CUSTOM_TEMPERATURE_SUVID 90
#define MIN_CUSTOM_TEMPERATURE_SUVID 25

// -------------- /END ЛИМИТЫ СЛАЙДЕРОВ --------------



