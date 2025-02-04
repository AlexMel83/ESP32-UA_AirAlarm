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
    unsigned long currentMillis = millis();

    // Перевіряємо підключення до Wi-Fi
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
        digitalWrite(PIN_LED_GREEN, LOW);   // Вимикаємо зелений LED
        digitalWrite(PIN_LED_RED, HIGH);    // Червоний LED горить постійно при втраті Wi-Fi
        return; // Виходимо, далі немає сенсу перевіряти сервер
    }

    // Перевіряємо зв’язок із сервером кожні LOGIC_DELAY мс
    if ((currentMillis - lastTime) > LOGIC_DELAY) {
        lastTime = currentMillis;
        int result = alertAPI.IsAlert();

        if (result == 2) {
            // Сервер не відповідає → мигаємо червоним LED
            if (currentMillis - lastLedTime >= 500) { // Мигаємо кожні 500 мс
                lastLedTime = currentMillis;
                digitalWrite(PIN_LED_RED, !digitalRead(PIN_LED_RED));
            }
            digitalWrite(PIN_LED_GREEN, LOW);  // Вимикаємо зелений LED
        } else {
            // Сервер відповів → оновлюємо статус тривоги
            SetAlert(result == 1);
            digitalWrite(PIN_LED_RED, LOW);   // Вимикаємо червоний LED
            digitalWrite(PIN_LED_GREEN, HIGH); // Увімкнути зелений LED
        }
    }

    // Обробка миготіння LED для сигналу тривоги
    processAlert();
}