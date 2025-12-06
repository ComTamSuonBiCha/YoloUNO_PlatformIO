#include "neo_blinky.h"


void neo_blinky(void *pvParameters){

    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    AppContext_t *ctx = (AppContext_t *) pvParameters;

    strip.begin();
    strip.show(); // all off

    NeoColor_t color = {0, 0, 0};

    while (1) {
        // Wait for new color from manager
        if (xQueueReceive(ctx->neoQueue, &color, portMAX_DELAY) == pdTRUE) {
            for (int i = 0; i < NEOPIXEL_COUNT; ++i) {
                strip.setPixelColor(i, strip.Color(color.r, color.g, color.b));
            }
            strip.show();
        }
    }
}