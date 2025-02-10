#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "config.h"
#include <Arduino.h>
#include "api_client.h"

// Функція для перевірки статусу тривоги через API /
AlarmStatus checkAlarmStatus() {
  HTTPClient http;
  http.begin(API_URL_ALLALERTS);
  http.addHeader("Authorization", API_KEY);
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    String payload = http.getString();
    http.end();

    // Використовуємо DynamicJsonDocument, оскільки розмір відповіді може бути різним
    DynamicJsonDocument doc(4096); // Збільшено розмір буфера
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return API_ERROR;
    }

    // Перевірка наявності regionId в активних тривогах
    bool regionActive = false;
    JsonArray regions = doc.as<JsonArray>();
    for (JsonObject region : regions) {
      if (region["regionId"].as<String>() == API_REGION_ID) {
        if (region["activeAlerts"].size() > 0) {
          regionActive = true;
          break;
        }
      }
    }

    // Виводимо масив в Serial Monitor
    serializeJson(doc, Serial);
    Serial.println();

    return regionActive ? ALARM_ACTIVE : ALARM_INACTIVE;
  } else {
    http.end();
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
    return API_ERROR;
  }
}
