#include "alert_api.h"
#include <ArduinoJson.h>

AlertAPI::AlertAPI(char* url, char *key) {
    this->apiUrl = url;
    this->apiKey = key;
    this->alarmStartTime = 0;
    this->alarmActive = false;
    this->alarmCooldown = false;
    this->cooldownStartTime = 0;
    pinMode(23, OUTPUT);
    pinMode(PIN_ALARM, OUTPUT); // Настроим пин тревоги как выход
    digitalWrite(23, LOW);
    digitalWrite(PIN_ALARM, LOW); // По умолчанию выключен
}

int AlertAPI::IsAlert() {
    unsigned long currentTime = millis();
    
    if (this->alarmCooldown) {
        if (currentTime - this->cooldownStartTime >= 1800000) {
            this->alarmCooldown = false;
            digitalWrite(23, LOW);
            Serial.println("COOLDOWN FINISHED!");
        } else {
            Serial.println("COOLDOWN ACTIVE, IGNORING ALERTS");
            return 0;
        }
    }

    if (this->alarmActive && (currentTime - this->alarmStartTime >= 60000)) {
        Serial.println("ALERT DEACTIVATED AFTER 1 MINUTE!");
        digitalWrite(PIN_ALARM, LOW);
        this->alarmActive = false;
        this->alarmCooldown = true;
        this->cooldownStartTime = currentTime;
        digitalWrite(23, HIGH);
    }

    HTTPClient http;
    http.begin(this->apiUrl);
    http.addHeader("accept", "application/json");
    http.addHeader("Authorization", this->apiKey);

    int httpResponseCode = http.GET();

    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println("Server Response:");
        Serial.println(payload);
        
        http.end();

        StaticJsonDocument<1024> jsonResponse;
        DeserializationError error = deserializeJson(jsonResponse, payload);

        if (error) {
            Serial.print("JSON parsing error: ");
            Serial.println(error.c_str());
            return 2;
        }

        if (!jsonResponse[0]["activeAlerts"].isNull()) {
            JsonArray activeAlerts = jsonResponse[0]["activeAlerts"];

            Serial.print("Active Alerts Count: ");
            Serial.println(activeAlerts.size());

            if (activeAlerts.size() > 0 && !this->alarmActive && !this->alarmCooldown) {
                Serial.println("ALERT ACTIVE!");
                digitalWrite(PIN_ALARM, HIGH);
                this->alarmStartTime = currentTime;
                this->alarmActive = true;
                digitalWrite(23, LOW);
                return 1;
            }
        } else {
            Serial.println("No 'activeAlerts' field in JSON!");
            return 2;
        }
    }

    Serial.println("NO ALERT");
    return 0;
}
