#include "task_wifi.h"

void startAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(String(SSID_AP), String(PASS_AP));
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void startSTA()
{
    if (WIFI_SSID.isEmpty())
    {
        Serial.println("WiFi SSID is empty. Cannot connect to WiFi.");
        return;
    }

    // Switch to AP+STA mode to keep AP running while connecting to WiFi
    WiFi.mode(WIFI_AP_STA);

    if (WIFI_PASS.isEmpty())
    {
        WiFi.begin(WIFI_SSID.c_str());
    }
    else
    {
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 50) // 5 seconds timeout
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        attempts++;
        if (attempts % 10 == 0) {
            Serial.print(".");
        }
    }
    
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.print("WiFi Connected! IP: ");
        Serial.println(WiFi.localIP());
        // Give semaphore to signal WiFi is connected (for CoreIOT task)
        xSemaphoreGive(xBinarySemaphoreInternet);
    }
    else
    {
        Serial.println();
        Serial.println("WiFi connection failed. Device will stay in AP mode.");
    }
}

bool Wifi_reconnect()
{
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        return true;
    }
    startSTA();
    return false;
}

void wifi_sta_task(void *pvParameters)
{
    // Wait a bit for configuration to be loaded
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    
    // Check if WiFi credentials are available
    if (!WIFI_SSID.isEmpty())
    {
        // Serial.println("WiFi credentials found. Connecting to WiFi...");
        // Serial.print("   SSID: ");
        // Serial.println(WIFI_SSID);
        
        startSTA();
        
        Serial.print("✅ WiFi Connected! IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("No WiFi credentials found. Device will stay in AP mode.");
    }
    
    // Task is done, delete itself
    vTaskDelete(NULL);
}