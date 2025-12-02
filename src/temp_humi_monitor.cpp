#include "temp_humi_monitor.h"
#include "tinyml.h"  // For semaphore communication with TinyML task

DHT20 dht20;
LiquidCrystal_I2C lcd(33,16,2);

void temp_humi_monitor(void *pvParameters){

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

        //Update global variables for temperature and humidity
        // Note: This is temporary - Task 3 requires removing globals and using semaphores
        glob_temperature = temperature;
        glob_humidity = humidity;

        // Signal semaphore for TinyML task (semaphore-based communication)
        // This allows TinyML task to know new sensor data is available
        if (xSensorDataSemaphore != NULL)
        {
            xSemaphoreGive(xSensorDataSemaphore);
        }

        // Print the results
        
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
        
        vTaskDelay(5000);
    }
    
}