#include "routes.h"
#include "SPIFFS.h"

// Оголошення зовнішньої змінної http
extern WebServer http;

//Define functions
String getContentType(String filename);
bool handleFileRead(String path);
String relay_switch();
String relay_status();

void setupRoutes() {
  //Handle http requests
  http.on("/relay_switch", []() {
    http.send(200, "text/plain", relay_switch());
  });
  http.on("/relay_status", []() {
    http.send(200, "text/plain", relay_status());
  });
  http.onNotFound([]() {
    if (!handleFileRead(http.uri()))
      http.send(404, "text/plain", "Not found");
  });
}

String getContentType(String filename) {
  if(filename.endsWith(".html")) return "text/html";
  if(filename.endsWith(".css")) return "text/css";
  if(filename.endsWith(".js")) return "application/javascript";
  if(filename.endsWith(".png")) return "image/png";
  if(filename.endsWith(".gif")) return "image/gif";
  if(filename.endsWith(".jpg")) return "image/jpeg";
  if(filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = http.streamFile(file, contentType);
    file.close();
    return true;  // Теперь функция возвращает bool
  }
  return false;   // Теперь функция возвращает bool
}