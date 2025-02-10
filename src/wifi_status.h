#ifndef WIFI_STATUS_H
#define WIFI_STATUS_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiStatus {
  private:
    const byte ledPin;
    bool ledState;
    unsigned long lastToggleTime;
    const unsigned long blinkInterval = 1000; // Інтервал миготіння (1 сек)

  public:
    WiFiStatus(byte pin);
    void begin();
    void update();
};

#endif
