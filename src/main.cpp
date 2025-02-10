#include "config.h"
#include "wifi_status.h"
#include "routes.h"
#include "api_client.h"
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESP8266FtpServer.h>

WiFiStatus wifiIndicator(GREEN_LED);

//Define functions
String relay_switch() {
  digitalWrite(RELAY, !digitalRead(RELAY));
  return String(digitalRead(RELAY));
}
String relay_status() {
  return String(digitalRead(RELAY));  
}

WebServer http(80);
FtpServer ftpSrv; // Создаем объект ftp сервера
// Переменные для работы с временем
unsigned long lastRequestTime = 0; // Время последнего запроса к API
unsigned long alarmStartTime = 0; // Время начала тревоги
bool alarmActive = false; // Флаг активации тревоги

// Константы для цикла реле
const byte relayCycleIterations = 3; // Кількість ітерацій циклу реле
const unsigned long relayOnDuration = 20000;   // Тривалість ввімкнення реле (20 секунд)
const unsigned long relayOffDuration = 5000;  // Тривалість вимкнення реле (5 секунд)

// Function to control the relay cycle
void relayCycle() {
  for (int i = 0; i < relayCycleIterations; ++i) {
    // Turn relay ON
    digitalWrite(RELAY, HIGH);
    Serial.print("Relay ON for ");
    Serial.print(relayOnDuration / 1000);
    Serial.println(" seconds");
    delay(relayOnDuration);

    // Turn relay OFF
    digitalWrite(RELAY, LOW);
    Serial.print("Relay OFF for ");
    Serial.print(relayOffDuration / 1000);
    Serial.println(" seconds");
    delay(relayOffDuration);
  }
  Serial.println("Relay cycle complete.");
}


void setup() {
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
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
          alarmActive = true;
          Serial.println("Alarm status: ACTIVE");
          Serial.println("Relay status: ON");
          relayCycle(); // Start the relay cycle
        }
        break;
      case ALARM_INACTIVE:
        if (alarmActive) {
          digitalWrite(RELAY, LOW);
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