
#ifndef __TASK_WEBSERVER_H__
#define __TASK_WEBSERVER_H__

#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include <DNSServer.h>
#include <task_handler.h>

extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern DNSServer dnsServer;

void Webserver_stop();
void Webserver_reconnect();
void Webserver_sendata(String data);
void webserver_task(void *parameter);

#endif