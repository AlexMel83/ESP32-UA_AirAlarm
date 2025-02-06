#ifndef AlertAPI_h
#define AlertAPI_h

#include <HTTPClient.h>
#include <ArduinoJson.h>

#define PIN_ALARM 2

class AlertAPI {
public:
	AlertAPI(char* url, char *key);

	int IsAlert();
 
private:
	char* apiUrl;
	char* apiKey;
	int alarmStartTime;
	bool alarmActive;
	bool alarmCooldown;
	unsigned long cooldownStartTime;
};

#endif