#include "alert_logic.h"
#include <Arduino.h>

AlertLogic::AlertLogic()
	: alertAPI(API_URL, API_KEY){

}

void AlertLogic::Init(){
	pinMode(PIN_ALARM, OUTPUT); // Настроить пин как выход
  digitalWrite(PIN_ALARM, LOW); // По умолчанию выключено

	pinMode(this->leds[0].pin, OUTPUT);
	pinMode(this->leds[1].pin, OUTPUT);
	digitalWrite(PIN_LED_GREEN, 1);
	digitalWrite(PIN_LED_RED, 1);

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	Serial.println("Connecting WiFi");

	while(WiFi.status() != WL_CONNECTED)
		delay(1000);		

	Serial.println("WiFi connected");
	Serial.println(WiFi.localIP());

	this->whNTP = new WhNTP();
	this->whNTP->Update();

	this->SetAlert(false, true);
	digitalWrite(PIN_LED_GREEN, 0);
	digitalWrite(PIN_LED_RED, 0);
}

void AlertLogic::SetAlert(bool alarm, bool firstTime){
	if(this->alertStatus != alarm || firstTime){
		#ifdef DEBUG
		char buffer[128];
		sprintf(buffer, "processAlarm(is_alarm = %d, firstTime = %d)", alarm, firstTime);
		Serial.println(buffer);
		#endif

		this->prevAlertStatus = this->alertStatus;
		this->alertStatus = alarm;
		this->lastLedTime = millis();
		this->leds[int(this->alertStatus)].status = true;
		digitalWrite(this->leds[int(this->alertStatus)].pin, HIGH);
		digitalWrite(this->leds[int(!this->alertStatus)].pin, LOW);

		if(!firstTime)      
			if(alarm){
				this->music.PlayAlertOn();
			}else{
				this->music.PlayAlertOff();
			}
	}
}

void AlertLogic::processAlert(){
	unsigned long max_delay = leds[int(this->alertStatus)].blinkTimeDealyOff;
	if(leds[int(this->alertStatus)].status)
		max_delay = this->leds[int(this->alertStatus)].blinkTimeDealyOn;

	if(millis() - this->lastLedTime >= max_delay){
		this->lastLedTime = millis();
		this->leds[int(this->alertStatus)].status = !leds[int(this->alertStatus)].status;
		digitalWrite(this->leds[int(this->alertStatus)].pin, int(this->leds[int(this->alertStatus)].status));
		#ifdef DEBUG
		char buffer[128];
		sprintf(buffer, "processAlarm: max_delay reset, status(%d) pin(%d) write(%d)", this->leds[int(this->alertStatus)].status, this->leds[int(this->alertStatus)].pin, int(this->leds[int(this->alertStatus)].status));
		Serial.println(buffer);
		#endif
	}	
}

void AlertLogic::Tick(){
	unsigned long currentMillis = millis();

	if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
        digitalWrite(PIN_LED_GREEN, LOW);   // Вимикаємо зелений LED
        digitalWrite(PIN_LED_RED, HIGH);    // Червоний LED горить постійно при втраті Wi-Fi
        return; // Виходимо, далі немає сенсу перевіряти сервер
    }
	
	if(this->whNTP->Update()){
		digitalWrite(PIN_LED_GREEN, 0);
		digitalWrite(PIN_LED_RED, 0);
	}

	if(this->whNTP->IsOperational()){
		if((millis() - this->lastTime) > LOGIC_DELAY) {
			#ifdef DEBUG
			Serial.println("Logic time");
			#endif
			this->lastTime = millis();

			if(WiFi.status() == WL_CONNECTED){
				int result = this->alertAPI.IsAlert();
			  
				if(result < 2)
					this->SetAlert(bool(result));
			}
		}

		this->processAlert();
	}

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
}