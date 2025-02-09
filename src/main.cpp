#include "config.h" // Файл с конфигурацией
#include "wifi_status.h" // Файл с функциями управления светодиодом
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESP8266FtpServer.h>
#include <HTTPClient.h>

WiFiStatus wifiIndicator(12);

/* Настройки IP адреса */
IPAddress local_ip(192,168,0,111);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(8, 8, 8, 8);

//Define functions
String getContentType(String filename);
bool handleFileRead(String path);
String relay_switch();
String relay_status();
void checkAlarm();

WebServer http(80);
FtpServer ftpSrv; // Создаем объект ftp сервера

byte tries = 10;  // Попыток подключения к точке доступа
const byte relay = 2; // Пин для управления реле
const unsigned long alarmDuration = 300000; // 5 минут
unsigned long lastRequestTime = 0; // Время последнего запроса к API
const unsigned long requestInterval = 30000; // Интервал запроса к API
unsigned long alarmStartTime = 0;
bool alarmCooldown = false;
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

  //Handle http requests
  http.on("/relay_switch", []() {
    http.send(200, "text/plain", relay_switch());
  });
  http.on("/relay_status", []() {
    http.send(200, "text/plain", relay_status());
  });
  http.onNotFound([]() {
    if (!handleFileRead(http.uri()))
      http.send(404, "text/plain", "Not found");
  });
}

void loop() {
  wifiIndicator.update();
  http.handleClient();  // Обработка входящих запросов
  ftpSrv.handleFTP();   // Обработка ftp запросов
  unsigned long currentMillis = millis();
  if (currentMillis - lastRequestTime >= requestInterval) {
    lastRequestTime = currentMillis; // Обновляем время последнего запроса
    checkAlarm(); // Запрашиваем тревогу через API
  }
  bool isWiFiConnected = (WiFi.status() == WL_CONNECTED);
  bool isRelayOn = (digitalRead(relay) == HIGH);
}

String relay_switch() {
  digitalWrite(relay, !digitalRead(relay));
  return String(digitalRead(relay));
}
String relay_status() {
  return String(digitalRead(relay));  
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = http.streamFile(file, contentType);
    file.close();
    return true;  // Теперь функция возвращает bool
  }
  return false;   // Теперь функция возвращает bool
}

String getContentType(String filename) {
  if(filename.endsWith(".html")) return "text/html";
  if(filename.endsWith(".css")) return "text/css";
  if(filename.endsWith(".js")) return "application/javascript";
  if(filename.endsWith(".png")) return "image/png";
  if(filename.endsWith(".gif")) return "image/gif";
  if(filename.endsWith(".jpg")) return "image/jpeg";
  if(filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

void checkAlarm() {
  Serial.println("Checking alarm...");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERROR] WiFi not connected!");
    return;
  }

  Serial.print("Connecting to API: ");
  Serial.println(API_URL);
  Serial.println(API_KEY);

  HTTPClient http;
  http.setTimeout(5000);
  http.begin(API_URL);
  http.addHeader("accept", "application/json");
  http.addHeader("Authorization", API_KEY);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.println("Received response:");
    String payload = http.getString();
    Serial.println(payload);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    JsonArray activeAlerts = doc[0]["activeAlerts"];

    // Якщо є активні тривоги, включаємо реле
    if (activeAlerts.size() > 0) {
      digitalWrite(relay, HIGH);  // Включити реле
      Serial.println("Alarm triggered, relay ON.");
    } else {
      digitalWrite(relay, LOW);  // Вимкнути реле
      Serial.println("No active alarm, relay OFF.");
    }
  } else {
    Serial.println("[ERROR] Failed to make HTTP request!");
    Serial.print("[DEBUG] HTTPClient error code: ");
    Serial.println(httpResponseCode);
    // Додано логування для виведення детальної помилки
    String errorResponse = http.getString();
    Serial.print("[DEBUG] Server error response: ");
    Serial.println(errorResponse);
  }
  http.end();
  Serial.println("Request complete.");
}
