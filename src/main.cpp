#include "config.h"
#include "wifi_status.h"
#include "routes.h"
#include "api_client.h" // Додаємо заголовочний файл для API
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

unsigned long lastRequestTime = 0; // Время последнего запроса к API
unsigned long alarmStartTime = 0;
bool alarmActive = false;

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
   http.begin();                    
   ftpSrv.begin("relay", "relay"); // Запуск FTP сервера
   Serial.println("Server listening");

  // Настройка маршрутов
  setupRoutes();
}

void loop() {
  wifiIndicator.update();
  http.handleClient();  // Обработка входящих запросов
  ftpSrv.handleFTP();   // Обработка ftp запросов
  unsigned long currentMillis = millis();
  if (currentMillis - lastRequestTime >= requestInterval) {
    lastRequestTime = currentMillis; // Обновляем время последнего запроса
    
    // Викликаємо функцію для перевірки тривоги через API
    bool alarmStatus = checkAlarmStatus();
    
    // Встановлюємо реле в залежності від статусу тривоги
    digitalWrite(relay, alarmStatus ? HIGH : LOW);
    Serial.print("Alarm status: ");
    Serial.println(alarmStatus ? "ACTIVE" : "INACTIVE");
    Serial.print("Relay status: ");
    Serial.println(alarmStatus ? "ON" : "OFF");
  }
  bool isWiFiConnected = (WiFi.status() == WL_CONNECTED);
  bool isRelayOn = (digitalRead(relay) == HIGH);
}