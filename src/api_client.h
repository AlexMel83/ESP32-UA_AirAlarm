#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Enum для представлення статусу тривоги та помилки
enum AlarmStatus {
  ALARM_ACTIVE,
  ALARM_INACTIVE,
  API_ERROR
};

// Функція для перевірки тривоги
AlarmStatus checkAlarmStatus();

// Оголошення зовнішньої змінної StaticJsonDocument
extern StaticJsonDocument<4096> lastRegionNames;

#endif
