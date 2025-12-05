#include "task_manager.h"

static LedPattern_t mapTempToPattern(float t) {
    if (t < 25.0f) {
        return LED_PATTERN_SLOW;
    } else if (t < 30.0f) {
        return LED_PATTERN_MEDIUM;
    } else {
        return LED_PATTERN_FAST;
    }
}

static NeoColor_t mapHumToColor(float h) {
    NeoColor_t c;
    if (h < 40.0f) {
        // Dry -> Blue
        c = {0, 0, 255};
    } else if (h < 70.0f) {
        // Normal -> Green
        c = {0, 255, 0};
    } else {
        // Humid -> Red
        c = {255, 0, 0};
    }
    return c;
}

static SystemState_t mapToState(float t, float h) {
    // Example thresholds for LCD state
    if (t < 30.0f && h < 70.0f) {
        return STATE_NORMAL;
    } else if (t < 35.0f && h < 80.0f) {
        return STATE_WARNING;
    } else {
        return STATE_CRITICAL;
    }
}

void manager_task(void *pvParameters) {
    AppContext_t *ctx = (AppContext_t *) pvParameters;
    SensorSample_t sample;

    while (1) {
        // Wait for new sensor sample
        if (xQueueReceive(ctx->sensorQueue, &sample, portMAX_DELAY) == pdTRUE) {
            float t = sample.temperature;
            float h = sample.humidity;

            // Task 1: temperature -> LED pattern
            LedPattern_t pattern = mapTempToPattern(t);
            if (ctx->ledQueue != nullptr) {
                xQueueSend(ctx->ledQueue, &pattern, 0); // no block if full
            }

            // Task 2: humidity -> NeoPixel color
            NeoColor_t color = mapHumToColor(h);
            if (ctx->neoQueue != nullptr) {
                xQueueSend(ctx->neoQueue, &color, 0);
            }

            // Task 3: state for LCD
            SystemState_t newState = mapToState(t, h);

            // Update shared state under mutex
            if (ctx->dataMutex != nullptr &&
                xSemaphoreTake(ctx->dataMutex, portMAX_DELAY) == pdTRUE) {

                ctx->currentState   = newState;
                ctx->lastTemperature = t;
                ctx->lastHumidity    = h;

                xSemaphoreGive(ctx->dataMutex);
            }
            
            glob_temperature = t;
            glob_humidity = h;

            // Notify LCD that state/values changed
            if (ctx->stateSemaphore != nullptr) {
                xSemaphoreGive(ctx->stateSemaphore);
            }
        }
    }
}