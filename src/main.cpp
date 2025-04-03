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

String relay_switch() {
  bool relayState = !digitalRead(RELAY);
  digitalWrite(RELAY, relayState);
  digitalWrite(RED_LED, relayState);
  return String(relayState);
}

WebServer http(80);
FtpServer ftpSrv;

unsigned long lastRequestTime = 0;
time_t alarmStartTime = 0;
bool alarmActive = false;
time_t lastCycleStartTime = 0;
bool waitingMessagePrinted = false; // Флаг для однократного вывода сообщения

const byte relayCycleIterations = 4;
const time_t relayOnDuration = 15; // 15 секунд
const time_t relayOffDuration = 5; // 5 секунд
const time_t cycleInterval = 30 * 60; // 30 минут в секундах

void relayCycle();

void setup() {
  pinMode(RELAY, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RELAY, LOW);
  digitalWrite(RED_LED, LOW);
  Serial.begin(SERIAL_BAUD);
  wifiIndicator.begin();

  WiFi.config(local_ip, gateway, subnet, dns);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP());

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");
  while (time(nullptr) < 100000) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nNTP time synced");

  SPIFFS.begin(true);
  http.begin();
  ftpSrv.begin("relay", "relay");
  Serial.println("Server listening");

  setupRoutes();
}

void loop() {
  wifiIndicator.update();
  http.handleClient();
  ftpSrv.handleFTP();

  unsigned long currentMillis = millis();

  if (currentMillis - lastRequestTime >= requestInterval) {
    lastRequestTime = currentMillis;

    AlarmStatus alarmStatus = checkAlarmStatus();

    switch (alarmStatus) {
      case ALARM_ACTIVE:
        if (!alarmActive) {
          alarmActive = true;
          Serial.println("Alarm status: ACTIVE");
          alarmStartTime = time(nullptr);
        }
        break;

      case ALARM_INACTIVE:
        if (alarmActive) {
          digitalWrite(RELAY, LOW);
          digitalWrite(RED_LED, LOW);
          alarmActive = false;
          Serial.println("Alarm status: INACTIVE");
        }
        break;

      case API_ERROR:
        Serial.println("API Error: Could not retrieve alarm status.");
        break;
    }
  }

  if (alarmActive) {
    relayCycle();
  }
}

void relayCycle() {
  static int cycleStep = 0;
  static time_t lastChangeTime = 0;
  static bool relayState = false;
  static bool cycleRunning = false;

  time_t currentTime = time(nullptr);

  // Завершение цикла после всех шагов
  if (cycleRunning && cycleStep >= relayCycleIterations * 2) { // После шага 5
    digitalWrite(RELAY, LOW);
    digitalWrite(RED_LED, LOW);
    Serial.println("Relay cycle completed. Waiting 30 minutes...");
    cycleRunning = false;
    lastCycleStartTime = currentTime;
    cycleStep = 0;
    waitingMessagePrinted = false; // Сбрасываем флаг
    return;
  }

  // Проверка интервала тишины только если цикл завершён
  if (!cycleRunning && (currentTime - lastCycleStartTime < cycleInterval)) {
    if (!waitingMessagePrinted) {
      Serial.println("Waiting for cycle interval...");
      waitingMessagePrinted = true; // Устанавливаем флаг
    }
    return;
  }

  // Запуск нового цикла
  if (!cycleRunning && alarmActive) {
    Serial.println("Starting new relay cycle...");
    cycleRunning = true;
    cycleStep = 0;
    relayState = true;
    digitalWrite(RELAY, relayState);
    digitalWrite(RED_LED, relayState);
    lastChangeTime = currentTime;
    Serial.print("Relay ON (Step ");
    Serial.print(cycleStep);
    Serial.println(")");
    return;
  }

  // Выполнение шагов цикла (0..5)
  if (cycleRunning && cycleStep < relayCycleIterations * 2) { // До 5 включительно
    time_t interval = relayState ? relayOnDuration : relayOffDuration;
    time_t timeDiff = currentTime - lastChangeTime;

    if (timeDiff >= interval) {
      relayState = !relayState;
      digitalWrite(RELAY, relayState);
      digitalWrite(RED_LED, relayState);
      cycleStep++;

      Serial.print("Relay ");
      Serial.print(relayState ? "ON" : "OFF");
      Serial.print(" (Step ");
      Serial.print(cycleStep);
      Serial.println(")");

      // Принудительное завершение после шага 5
      if (cycleStep >= relayCycleIterations * 2) {
        digitalWrite(RELAY, LOW);
        digitalWrite(RED_LED, LOW);
        Serial.println("Relay cycle completed. Waiting 30 minutes...");
        cycleRunning = false;
        lastCycleStartTime = currentTime;
        cycleStep = 0;
        waitingMessagePrinted = false;
      }

      lastChangeTime = currentTime;
    }
  }
}