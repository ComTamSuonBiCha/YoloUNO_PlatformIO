#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

extern float glob_temperature;
extern float glob_humidity;

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;

// ---- Sensor sample from DHT20 ----
typedef struct {
    float temperature;
    float humidity;
} SensorSample_t;

// ---- LED patterns (Task 1) ----
typedef enum {
    LED_PATTERN_SLOW = 0,
    LED_PATTERN_MEDIUM,
    LED_PATTERN_FAST
} LedPattern_t;

// ---- NeoPixel color (Task 2) ----
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} NeoColor_t;

// ---- System state for LCD (Task 3) ----
typedef enum {
    STATE_NORMAL = 0,
    STATE_WARNING,
    STATE_CRITICAL
} SystemState_t;

// ---- Application context (no global vars) ----
typedef struct {
    QueueHandle_t sensorQueue;   // sensor -> manager
    QueueHandle_t ledQueue;      // manager -> LED
    QueueHandle_t neoQueue;      // manager -> NeoPixel

    SemaphoreHandle_t stateSemaphore; // manager -> LCD (state changes)
    SemaphoreHandle_t dataMutex;      // protect state + last values

    // Shared state for LCD
    SystemState_t currentState;
    float lastTemperature;
    float lastHumidity;
} AppContext_t;
#endif