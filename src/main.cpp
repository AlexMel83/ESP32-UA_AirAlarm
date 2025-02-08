#include <WiFi.h>
#include <WebServer.h>
#include "config.h" // Файл с конфигурацией

WebServer server(80);
byte tries = 10;  // Попыток подключения к точке доступа

void handleRootPath();  // Forward declaration

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (--tries && WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Non Connecting to WiFi..");
  } else {
    // Иначе удалось подключиться отправляем сообщение
    // о подключении и выводим адрес IP
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  server.on("/other", []() {
    server.send(200, "text/plain", "Additional page");
  });

  server.on("/", handleRootPath);    // Свяжем функцию обработчика с путем
  server.begin();                    // Запускаем сервер
  Serial.println("Server listening");
}

void loop() {
  server.handleClient();  // Обработка входящих запросов
}

void handleRootPath() {  // Обработчик для корневого пути
  server.send(200, "text/plain", "Greetings from the site starkon.pp.ua");
}