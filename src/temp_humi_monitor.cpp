#include <Wire.h>
#include "temp_humi_monitor.h"
#include <DHT20.h>

DHT20 dht20;

// Choose free pins on this board:
#define I2C_SDA  2
#define I2C_SCL  10
// Add 4.7 kΩ pull-ups from SDA→3V3 and SCL→3V3 if your DHT20 breakout doesn’t already include them
// DHT20 VCC → P5-6 (VDD33), GND → P5-7 (GND)
// DHT20 SDA → GPIO2, SCL → GPIO10
void temp_humi_monitor(void *pvParameters){

  // Start I²C on GPIO2 (SDA) and GPIO10 (SCL)
  Wire.begin(I2C_SDA, I2C_SCL);

  Serial.begin(115200);
  dht20.begin();

  for(;;){
    dht20.read();
    float temperature = dht20.getTemperature();
    float humidity    = dht20.getHumidity();

    // If read fails, send NaN so your UI can show "N/A"
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT20!");
      temperature = NAN;
      humidity    = NAN;
    }

    glob_temperature = temperature;
    glob_humidity    = humidity;

    Serial.print("Humidity: ");   Serial.print(humidity);
    Serial.print("%  Temperature: "); Serial.print(temperature);
    Serial.println("°C");

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
