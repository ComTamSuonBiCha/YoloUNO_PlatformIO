#include "temp_humi_monitor.h"
#include "tinyml.h"  // For semaphore communication with TinyML task

DHT20 dht20;
LiquidCrystal_I2C lcd(33,16,2);

void temp_humi_monitor(void *pvParameters){
    AppContext_t *ctx = (AppContext_t *) pvParameters;

    // Local sensor object (not global)
    static DHT20 dht20;
    Wire.begin(11, 12);
    Serial.begin(115200);
    dht20.begin();

    while (1){
        /* code */
        
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();

        

        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity =  -1;
            //return;
        }
        SensorSample_t sample;
        sample.temperature = temperature;
        sample.humidity    = humidity;

        // Send to manager (blocking if queue full)
        if (ctx->sensorQueue != nullptr) {
            xQueueSend(ctx->sensorQueue, &sample, portMAX_DELAY);
        }

        // Print the results
        
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
        
        // Send data to web interface via WebSocket
        String sensorData = "{\"page\":\"sensor\",\"temperature\":" + String(temperature, 1) + ",\"humidity\":" + String(humidity, 1) + "}";
        Webserver_sendata(sensorData);
        
        vTaskDelay(5000);
    }
    
}