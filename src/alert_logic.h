#include "config.h"

#ifndef ALERT_LOGIC_H
#define ALERT_LOGIC_H

#include <WiFi.h>
#include "wh_ntp.h"
#include "alert_api.h"
#include "music.h"
#include <array>

#define PIN_BUZZER 18
#define PIN_LED_RED 13
#define PIN_LED_GREEN 12

#define LOGIC_DELAY 30000

struct Led {
    unsigned long blinkTimeOn;
    unsigned long blinkTimeOff;
    bool status;
    uint8_t pin;

    Led(unsigned long on, unsigned long off, bool st, uint8_t p)
        : blinkTimeOn(on), blinkTimeOff(off), status(st), pin(p) {}
};

class AlertLogic {
public:
    AlertLogic();
    void Init();
    void Tick();

private:
    void SetAlert(bool alert, bool firstTime = false);
    void processAlert();

    WhNTP* whNTP = nullptr;
    Music music;
    AlertAPI alertAPI;
    std::array<Led, 2> leds;
    unsigned long lastTime = 0;
    unsigned long lastLedTime = 0;
    bool alertStatus = false;
};

#endif // ALERT_LOGIC_H