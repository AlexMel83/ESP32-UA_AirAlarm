#include "alert_logic.h"

AlertLogic::AlertLogic()
    : music(PIN_BUZZER), alertAPI(API_URL, API_KEY),
      leds(std::array<Led, 2>{{  // Явно створюємо std::array
          Led(50, 3000, false, PIN_LED_GREEN),
          Led(300, 300, false, PIN_LED_RED)
      }}) {}

void AlertLogic::Init() {
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) delay(1000);

    whNTP = new WhNTP();
    whNTP->Update();
    SetAlert(false, true);
}

void AlertLogic::SetAlert(bool alarm, bool firstTime) {
    if (alertStatus != alarm || firstTime) {
        alertStatus = alarm;
        lastLedTime = millis();
        leds[alertStatus].status = true;
        digitalWrite(leds[alertStatus].pin, HIGH);
        digitalWrite(leds[!alertStatus].pin, LOW);

        if (!firstTime) {
            alarm ? music.PlayAlertOn() : music.PlayAlertOff();
        }
    }
}

void AlertLogic::processAlert() {
    auto& led = leds[alertStatus];
    unsigned long delayTime = led.status ? led.blinkTimeOn : led.blinkTimeOff;
    if (millis() - lastLedTime >= delayTime) {
        lastLedTime = millis();
        led.status = !led.status;
        digitalWrite(led.pin, led.status);
    }
}

void AlertLogic::Tick() {
    if (whNTP->Update()) {
        digitalWrite(PIN_LED_GREEN, LOW);
        digitalWrite(PIN_LED_RED, LOW);
    }

    if (whNTP->IsOperational() && (millis() - lastTime) > LOGIC_DELAY) {
        lastTime = millis();
        if (WiFi.status() == WL_CONNECTED) {
            int result = alertAPI.IsAlert();
            if (result < 2) SetAlert(result);
        }
    }
    processAlert();
}