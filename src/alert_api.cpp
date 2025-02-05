#include "alert_api.h"
#include <ArduinoJson.h>

AlertAPI::AlertAPI(char* url, char *key) {
    this->apiUrl = url;
    this->apiKey = key;
}

int AlertAPI::IsAlert() {
    HTTPClient http;
    http.begin(this->apiUrl);
    http.addHeader("accept", "application/json");
    http.addHeader("Authorization", this->apiKey);

    int httpResponseCode = http.GET();

    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);  // Логируем код ответа сервера

    if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println("Server Response:");
        Serial.println(payload);  // Логируем полный JSON-ответ сервера

        http.end();

        // Разбор JSON
        StaticJsonDocument<1024> jsonResponse;
        DeserializationError error = deserializeJson(jsonResponse, payload);

        if (error) {
            Serial.print("JSON parsing error: ");
            Serial.println(error.c_str());
            return 2; // Ошибка парсинга
        }

        // Проверка наличия массива activeAlerts
        if (!jsonResponse[0]["activeAlerts"].isNull()) {
            JsonArray activeAlerts = jsonResponse[0]["activeAlerts"];

            Serial.print("Active Alerts Count: ");
            Serial.println(activeAlerts.size());  // Количество тревог

            if (activeAlerts.size() > 0) {
                Serial.println("ALERT ACTIVE!");
                digitalWrite(PIN_ALARM, HIGH); // Включаем питание на GPIO2
                return 1;
            } else {
                Serial.println("NO ALERT");
                digitalWrite(PIN_ALARM, LOW); // Выключаем питание на GPIO2
                return 0;
            }
        } else {
            Serial.println("No 'activeAlerts' field in JSON!");
            return 2;
        }
    }

    Serial.println("No response from the server");
    digitalWrite(PIN_ALARM, LOW); // Выключаем питание на GPIO2
    http.end();
    return 2;
}
