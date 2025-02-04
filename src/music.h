#ifndef MUSIC_H
#define MUSIC_H
#include <stdint.h> 

extern int melody_alert_on[];
extern int delay_alert_on[];
extern int melody_alert_off[];
extern int delay_alert_off[];

#define MELODY_LENGTH_ALERT_ON 70
#define MELODY_LENGTH_ALERT_OFF 74

class Music {
public:
    Music(int pinBuzzer);
    void PlayAlertOn();
    void PlayAlertOff();

private:
    void Play(int melody[], int delays[], int length, float delay_multi);
    uint8_t pinBuzzer;  // Використовуємо uint8_t замість byte
};

#endif // MUSIC_H