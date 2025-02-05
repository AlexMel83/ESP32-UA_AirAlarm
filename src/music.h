#ifndef Music_h
#define Music_h

#include <Arduino.h>

#define MELODY_LENGTH_ALERT_ON 70
#define MELODY_LENGTH_ALERT_OFF 74

// Оголошуємо масиви як extern
extern int melody_alert_on[];
extern int delay_alert_on[];
extern int melody_alert_off[];
extern int delay_alert_off[];

class Music {
public:
    Music(int pinBuzzer);
    void PlayAlertOn();
    void PlayAlertOff();

private:
    void Play(int melody[], int delays[], int length, float delay_multi);
    uint8_t pinBuzzer;  // Замінено byte на uint8_t
};

#endif
