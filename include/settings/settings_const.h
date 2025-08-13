// Выделено примерно 20кб под NVS

// текущая версия настроек, если в NVS памяти записана меньше - нужна миграция данных, до текущей версии
#define SETTINGS_VERSION 4

// Названия ключей в хранилище 
// (по возможности использовать короткие, макс возможная 15 симв)
// 1-2х байтовые значения не экономят память по сравнению с 4-8х байтовыми
// т.к. значения все равно выравниваются до 8 байт нулями

// версия настроек (для миграции настроек)
#define SETTING_KEY_VERSION                 "version"

// версия платы (при первом запуске сохранить HARDWARE_VERSION_DEFAULT)
#define SETTING_KEY_HARDWARE_VERSION        "hwVersion"

// Bluetooth
#define SETTING_KEY_BLE_TOKEN               "btToken"

// Wifi
#define SETTING_KEY_WIFI_SSID               "wifiSsid"
#define SETTING_KEY_WIFI_PASSWORD           "wifiPass"

// Состояние обновления дисплея. 0 - завершено, 1 - не завершено
#define SETTING_KEY_DISPLAY_FIRMWARE_UPDATE_STATE "dsp_upd"

// Server
#define SETTING_KEY_SERVER_TOKEN            "token"
#define SETTING_KEY_WEBSOCKET_HOST          "ws_host"
#define SETTING_KEY_WEBSOCKET_PROTOCOL      "ws_protocol"
#define SETTING_KEY_WEBSOCKET_PORT          "ws_port"

// Калибровка температуры (int32_t x 0.1 град)
#define SETTING_KEY_CALIBRATION_FACTORY     "t1_clbr_fct"       // темп-ра заводская калибровка
#define SETTING_KEY_CALIBRATION_USER        "t1_clbr"           // темп-ра пользовательская калибровка
#define SETTING_KEY_HEATER_MAX_POWER        "h1_max_pow"        // макс мощность нагрева

// Яркость экрана (int32_t 0-100)
#define SETTING_KEY_SCREEN_BRIGHTNESS       "scrBright"

// Параметры устройства "железа"
#define SETTING_KEY_DEVICE_POWER            "dev_power"   // мощность ТЭНа
#define SETTING_KEY_DEVICE_VOLUME           "dev_volume"  // объем бака

