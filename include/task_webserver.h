#ifndef __TASK_WEBSERVER_H__
#define __TASK_WEBSERVER_H__

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include "task_handler.h" // đảm bảo handleWebSocketMessage hoặc tương tự được khai báo

extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern bool webserver_isrunning;

void Webserver_stop();
void connectWSV();
void Webserver_reconnect();
void Webserver_sendata(const String &data); // nhận theo ref để tránh copy
// void loadRelays();
// void saveRelays();

#endif // __TASK_WEBSERVER_H__
