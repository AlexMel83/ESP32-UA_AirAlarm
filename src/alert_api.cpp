#include "alert_api.h"

AlertAPI::AlertAPI(const std::string& url, const std::string& key)
    : apiUrl(url), apiKey(key) {}

int AlertAPI::IsAlert() {
    HTTPClient http;
    http.begin(apiUrl.c_str());
    http.addHeader("X-API-Key", apiKey.c_str());

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        String payload = http.getString();
        http.end();

        StaticJsonDocument<128> jsonResponse;
        deserializeJson(jsonResponse, payload);

        return jsonResponse["alert"] ? 1 : 0;
    }
    http.end();
    return 2;
}