#ifndef ALERT_API_H
#define ALERT_API_H

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string>

class AlertAPI {
public:
    AlertAPI(const std::string& url, const std::string& key);
    int IsAlert();

private:
    std::string apiUrl;
    std::string apiKey;
};

#endif // ALERT_API_H