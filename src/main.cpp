#include "config.h" // Файл с конфигурацией
#include <WiFi.h>
#include <WebServer.h>
#include "SPIFFS.h"
#include <ESP8266FtpServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/* Настройки IP адреса */
IPAddress local_ip(192,168,0,111);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);

//Define functions
String getContentType(String filename);
bool handleFileRead(String path);
String relay_switch();
String relay_status();

WebServer http(80);
FtpServer ftpSrv; // Создаем объект ftp сервера

byte tries = 10;  // Попыток подключения к точке доступа
const byte relay = 2; // Пин для управления реле

void setup() {
  pinMode(relay, OUTPUT);
  Serial.begin(115200);
  WiFi.config(local_ip, gateway, subnet);
  WiFi.begin(ssid, password);
  SPIFFS.begin(true); // Инициализация файловой системы
    
  while (--tries && WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Non Connecting to WiFi..");
  } else {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  http.begin();                    // Запускаем сервер
  ftpSrv.begin("relay", "relay"); // Запускаем ftp сервер
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
  http.handleClient();  // Обработка входящих запросов
    ftpSrv.handleFTP();   // Обработка ftp запросов
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