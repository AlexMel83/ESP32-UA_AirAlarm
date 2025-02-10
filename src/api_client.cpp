#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "config.h"

// Функція для перевірки статусу тривоги через API
bool checkAlarmStatus() {
  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Authorization", API_KEY);
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
      String payload = http.getString();
      http.end();

      // Use StaticJsonDocument correctly
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return false;
      }

      if (!doc[0]["activeAlerts"].isNull()) {
          JsonArray activeAlerts = doc[0]["activeAlerts"];
          return activeAlerts.size() > 0;
      }
  } else {
      http.end();
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
  }

  return false;
}