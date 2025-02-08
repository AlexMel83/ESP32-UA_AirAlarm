#include <WiFi.h>
#include <WebServer.h>
#include "config.h" // Файл с конфигурацией

/* Настройки IP адреса */
IPAddress local_ip(192,168,0,111);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);

uint8_t LED_RED = 13;
bool LED_RED_status = LOW;
uint8_t LED_GREEN = 12;
bool LED_GREEN_status = LOW;

WebServer server(80);
byte tries = 10;  // Попыток подключения к точке доступа

void handle_NotFound();
void handle_OnConnect();
void handle_ledRedOn();
void handle_ledRedOff();
void handle_ledGreenOn();
void handle_ledGreenOff();
String SendHTML(uint8_t ledredstat,uint8_t ledgreenstat);

void setup() {
  Serial.begin(115200);
  WiFi.config(local_ip, gateway, subnet);
  WiFi.begin(ssid, password);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

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
  
  server.on("/", handle_OnConnect);
  server.on("/ledredon", handle_ledRedOn);
  server.on("/ledredoff", handle_ledRedOff);
  server.on("/ledgreenon", handle_ledGreenOn);
  server.on("/ledgreenoff", handle_ledGreenOff);
  server.onNotFound(handle_NotFound);

  server.begin();                    // Запускаем сервер
  Serial.println("Server listening");
}

void loop() {
  server.handleClient();  // Обработка входящих запросов

  if(LED_RED_status)
  {digitalWrite(LED_RED, HIGH);}
  else
  {digitalWrite(LED_RED, LOW);}
  if(LED_GREEN_status)
  {digitalWrite(LED_GREEN, HIGH);}
  else
  {digitalWrite(LED_GREEN, LOW);}
}

void handle_OnConnect() {
  LED_RED_status = LOW;
  LED_GREEN_status = LOW;
  Serial.println("GPIO12 Status: OFF | GPIO13 Status: OFF");
  server.send(200, "text/html", SendHTML(LED_RED_status,LED_GREEN_status)); 
}
void handle_ledRedOn() {
  LED_RED_status = HIGH;
  Serial.println("GPIO12 Status: ON");
  server.send(200, "text/html", SendHTML(true,LED_GREEN_status)); 
}
void handle_ledRedOff() {
  LED_RED_status = LOW;
  Serial.println("GPIO12 Status: OFF");
  server.send(200, "text/html", SendHTML(false,LED_GREEN_status)); 
}
void handle_ledGreenOn() {
  LED_GREEN_status = HIGH;
  Serial.println("GPIO13 Status: ON");
  server.send(200, "text/html", SendHTML(LED_RED_status,true)); 
}
void handle_ledGreenOff() {
  LED_GREEN_status = LOW;
  Serial.println("GPIO13 Status: OFF");
  server.send(200, "text/html", SendHTML(LED_RED_status,false)); 
}
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t ledRedStat,uint8_t ledGreenStat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<meta http-equiv=\"Content-type\" content=\"text/html; charset=utf-8\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Управление светодиодом</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Веб сервер</h1>\n";
    ptr +="<h3>Режим клиент WiFi</h3>\n";
   if(ledRedStat)
  {ptr +="<p>Состояние LED_RED: ВКЛ.</p><a class=\"button button-off\" href=\"/ledredoff\">ВЫКЛ.</a>\n";}
  else
  {ptr +="<p>Состояние LED_RED: ВЫКЛ.</p><a class=\"button button-on\" href=\"/ledredon\">ВКЛ.</a>\n";}
  if(ledGreenStat)
  {ptr +="<p>Состояние LED_GREEN: ВКЛ.</p><a class=\"button button-off\" href=\"/ledgreenoff\">ВЫКЛ.</a>\n";}
  else
  {ptr +="<p>Состояние LED_GREEN: ВЫКЛ.</p><a class=\"button button-on\" href=\"/ledgreenon\">ВКЛ.</a>\n";}
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}