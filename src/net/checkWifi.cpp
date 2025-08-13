#include <WiFi.h>

// НЕ ИСПОЛЬЗУЕТСЯ
// Функция для проверки подключения к Wi-Fi

#define WIFI_ERROR_NO           0  // подключение успешно
#define WIFI_ERROR_NO_SSID      1  // SSID не найден
#define WIFI_ERROR_BAD_PASSWORD 2  // Неверный пароль
#define WIFI_ERROR_UNKNOWN      3  // Неизвестная ошибка

int checkWifi(const char* ssid, const char* password) {
  // Устанавливаем режим Wi-Fi в STA (станция)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Отключаемся от текущих сетей
  delay(100); // Небольшая задержка для стабильности

  // Начинаем попытку подключения
  WiFi.begin(ssid, password);
  Serial.print("Connect to: ");
  Serial.println(ssid);

  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 30000; // 30 секунд

  // Ждём подключения или истечения таймаута
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    delay(500);
    Serial.print("."); // Индикация процесса подключения
  }
  Serial.println(); // Перенос строки после индикаторов

  switch (WiFi.status()) {
    case WL_CONNECTED: 
      return 0;
    case WL_NO_SSID_AVAIL:
        Serial.println("NO SSID");
        return 1;
    case WL_CONNECT_FAILED:
        Serial.println("FAIL");
        return 2;
    default:
        Serial.print("unknown error");
        return 2; // По умолчанию считаем как ошибку пароля или другую проблему
  }
}
