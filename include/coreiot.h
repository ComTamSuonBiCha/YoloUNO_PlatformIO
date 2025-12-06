#ifndef __COREIOT_H__
#define __COREIOT_H__

#include <Arduino.h>
#include <WiFi.h>
#include "global.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

// CoreIOT task now receives AppContext to access telemetry queue
void coreiot_task(void *pvParameters);

#endif