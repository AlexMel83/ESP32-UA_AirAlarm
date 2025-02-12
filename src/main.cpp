#include "config.h"
#include "wifi_status.h"
#include "routes.h"
#include "api_client.h"
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESP8266FtpServer.h>
#include <time.h>

WiFiStatus wifiIndicator(GREEN_LED);

//Define functions
String relay_switch() {
  digitalWrite(RELAY, !digitalRead(RELAY));
  return String(digitalRead(RELAY));
}

WebServer http(80);
FtpServer ftpSrv; // Создаем объект ftp сервера

// Переменные для работы с временем
unsigned long lastRequestTime = 0; // Время последнего запроса к API
time_t alarmStartTime = 0; // Время начала тревоги
bool alarmActive = false; // Флаг активации тревоги
unsigned long lastCycleStartTime = 0; // Время последнего запуска цикла реле

// Константы для цикла реле
const byte relayCycleIterations = 3; // Кількість ітерацій циклу реле
const unsigned long relayOnDuration = 20000;   // Тривалість ввімкнення реле (20 секунд)
const unsigned long relayOffDuration = 5000;  // Тривалість вимкнення реле (5 секунд)
const unsigned long cycleInterval = 30 * 60 * 1000; // Інтервал між циклами (30 хвилин)

// Function prototype
void relayCycle();

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

  // Initialize NTP
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");
  while (time(nullptr) < 100000) { // Wait until the time is valid
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("NTP time synced");

   // Запуск HTTP сервера
   SPIFFS.begin(true); // Инициализация файловой системы
   http.begin(); // Запуск HTTP сервера
   ftpSrv.begin("relay", "relay"); // Запуск FTP сервера
   Serial.println("Server listening");

  // Настройка маршрутов
  setupRoutes();

  // Инициализируем время последнего запуска цикла
  lastCycleStartTime = 0;
  alarmStartTime = 0;
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
          // Якщо тривога щойно активована
          alarmActive = true;
          Serial.println("Alarm status: ACTIVE");
          Serial.println("Relay status: ON");
          alarmStartTime = time(nullptr); // Запоминаем время начала тревоги
          Serial.print("Alarm started at: ");
          Serial.println(alarmStartTime);
          relayCycle(); // Start the relay cycle
          lastCycleStartTime = currentMillis; // Запоминаем время запуска цикла
          Serial.print("First cycle started at: ");
          Serial.println(lastCycleStartTime);
        } else {
          // Якщо тривога вже активна, перевіряємо, чи можна запустити цикл знову
          if (currentMillis - lastCycleStartTime >= cycleInterval) {
            Serial.println("Time for new cycle!");
            relayCycle(); // Запускаем цикл
            lastCycleStartTime = currentMillis; // Запоминаем время запуска цикла
            Serial.print("New cycle started at: ");
            Serial.println(lastCycleStartTime);
          } else {
            Serial.print("Relay cycle is on cooldown. Time left: ");
            Serial.println((cycleInterval - (currentMillis - lastCycleStartTime)) / 1000);
          }
        }
        break;
      case ALARM_INACTIVE:
        if (alarmActive) {
          digitalWrite(RELAY, LOW);
          alarmActive = false;
          Serial.println("Alarm status: INACTIVE");
          Serial.println("Relay status: OFF");
          alarmStartTime = 0;
          lastCycleStartTime = 0;
        }
        break;
      case API_ERROR:
        Serial.println("API Error: Could not retrieve alarm status.");
        break;
    }
  }
}

// Function to control the relay cycle
void relayCycle() {
  Serial.println("Starting relay cycle...");
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