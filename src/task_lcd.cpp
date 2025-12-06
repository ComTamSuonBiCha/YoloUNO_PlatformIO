#include "task_lcd.h"


void lcd_task(void *pvParameters) {
    AppContext_t *ctx = (AppContext_t *) pvParameters;

    // Local LCD object
    static LiquidCrystal_I2C lcd(33,16,2);
    lcd.begin();
    lcd.backlight();
    lcd.clear();

    while (1) {
        // Wait until manager signals a new state/values
        if (xSemaphoreTake(ctx->stateSemaphore, portMAX_DELAY) == pdTRUE) {
            SystemState_t state;
            float t, h;

            // Read shared state under mutex
            if (ctx->dataMutex != nullptr &&
                xSemaphoreTake(ctx->dataMutex, portMAX_DELAY) == pdTRUE) {

                state = ctx->currentState;
                t     = ctx->lastTemperature;
                h     = ctx->lastHumidity;

                xSemaphoreGive(ctx->dataMutex);
            } else {
                continue;
            }

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("T:");
            lcd.print(t, 1);
            lcd.print("C H:");
            lcd.print(h, 1);
            lcd.print("%");

            lcd.setCursor(0, 1);
            switch (state) {
                case STATE_NORMAL:
                    lcd.print("STATE: NORMAL   ");
                    break;
                case STATE_WARNING:
                    lcd.print("STATE: WARNING  ");
                    break;
                case STATE_CRITICAL:
                    lcd.print("STATE: CRITICAL ");
                    break;
            }
        }
    }
}