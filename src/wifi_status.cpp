#include "wifi_status.h"

WiFiStatus::WiFiStatus(byte pin) : ledPin(pin), ledState(true), lastToggleTime(0) {}

void WiFiStatus::begin() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // Спочатку діод горить
}

void WiFiStatus::update() {
  if (WiFi.status() == WL_CONNECTED) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastToggleTime >= blinkInterval) {
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      lastToggleTime = currentMillis;
    }
  } else {
    digitalWrite(ledPin, HIGH); // WiFi не підключений → діод горить постійно
  }
}
