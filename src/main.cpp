#include "config.h"
#include "wifi_status.h"
#include "routes.h"
#include "api_client.h"
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <ESP8266FtpServer.h>

WiFiStatus wifiIndicator(12);

//Define functions
String relay_switch() {
  digitalWrite(relay, !digitalRead(relay));
  return String(digitalRead(relay));
}
String relay_status() {
  return String(digitalRead(relay));  
}

WebServer http(80);
FtpServer ftpSrv; // Создаем объект ftp сервера
// Переменные для работы с временем
unsigned long lastRequestTime = 0; // Время последнего запроса к API
unsigned long alarmStartTime = 0; // Время начала тревоги
bool alarmActive = false; // Флаг активации тревоги

void setup() {
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  Serial.begin(115200);
  wifiIndicator.begin(); // Ініціалізація діода статусу WiFi

  WiFi.config(local_ip, gateway, subnet, dns);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
  // Даем время на подключение
  unsigned long startMillis = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

   // Запуск HTTP сервера
   SPIFFS.begin(true); // Инициализация файловой системы
   http.begin(); // Запуск HTTP сервера
   ftpSrv.begin("relay", "relay"); // Запуск FTP сервера
   Serial.println("Server listening");

  // Настройка маршрутов
  setupRoutes();
}

void loop() {
  wifiIndicator.update(); // Оновлення індикатора статусу WiFi
	// bool isWiFiConnected = (WiFi.status() == WL_CONNECTED);
  // bool isRelayOn = (digitalRead(relay) == HIGH);
  http.handleClient();  // Обработка входящих запросов
  ftpSrv.handleFTP();   // Обработка ftp запросов
  unsigned long currentMillis = millis(); // Оновлюємо значення currentMillis
  if (currentMillis - lastRequestTime >= requestInterval) {
    lastRequestTime = currentMillis; // Обновляем время последнего запроса
    
    // Викликаємо функцію для перевірки тривоги через API
    AlarmStatus alarmStatus = checkAlarmStatus();

    // Перевіряємо статус тривоги
    switch (alarmStatus) {
      case ALARM_ACTIVE:
        if (!alarmActive) {
          digitalWrite(relay, HIGH);
          alarmActive = true;
          Serial.println("Alarm status: ACTIVE");
          Serial.println("Relay status: ON");
        }
        break;
      case ALARM_INACTIVE:
        if (alarmActive) {
          digitalWrite(relay, LOW);
          alarmActive = false;
          Serial.println("Alarm status: INACTIVE");
          Serial.println("Relay status: OFF");
        }
        break;
      case API_ERROR:
        Serial.println("API Error: Could not retrieve alarm status.");
        break;
    }
  }
}
