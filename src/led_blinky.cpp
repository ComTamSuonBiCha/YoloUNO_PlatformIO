#include "led_blinky.h"

void led_blinky_task(void *pvParameters) {
    AppContext_t *ctx = (AppContext_t *) pvParameters;
    pinMode(LED_GPIO, OUTPUT);

    LedPattern_t currentPattern = LED_PATTERN_SLOW;
    TickType_t blinkDelay = pdMS_TO_TICKS(1000);

    while (1) {
        // Check if new pattern available (non-blocking)
        LedPattern_t newPattern;
        if (xQueueReceive(ctx->ledQueue, &newPattern, 0) == pdTRUE) {
            currentPattern = newPattern;

            // Map pattern -> delay
            switch (currentPattern) {
                case LED_PATTERN_SLOW:
                    blinkDelay = pdMS_TO_TICKS(1000);
                    break;
                case LED_PATTERN_MEDIUM:
                    blinkDelay = pdMS_TO_TICKS(500);
                    break;
                case LED_PATTERN_FAST:
                    blinkDelay = pdMS_TO_TICKS(100);
                    break;
            }
        }

        // Blink with current pattern
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(blinkDelay);
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(blinkDelay);
    }
}