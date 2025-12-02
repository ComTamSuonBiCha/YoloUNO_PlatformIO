#include <task_handler.h>

void handleWebSocketMessage(String message)
{
    Serial.println(message);
    StaticJsonDocument<256> doc;

    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
        Serial.println("❌ Lỗi parse JSON!");
        return;
    }

    if (doc["page"] == "device")
    {
        // Handle LED control
        if (!doc.containsKey("device") || !doc.containsKey("gpio") || !doc.containsKey("status"))
        {
            Serial.println("⚠️ JSON thiếu thông tin device, gpio hoặc status");
            return;
        }

        String device = doc["device"].as<String>();
        int gpio = doc["gpio"];
        String status = doc["status"].as<String>();

        Serial.printf("⚙️ Điều khiển %s (GPIO %d) → %s\n", device.c_str(), gpio, status.c_str());
        
        pinMode(gpio, OUTPUT);
        if (status.equalsIgnoreCase("ON"))
        {
            digitalWrite(gpio, HIGH);
            Serial.printf("🔆 %s (GPIO %d) ON\n", device.c_str(), gpio);
        }
        else if (status.equalsIgnoreCase("OFF"))
        {
            digitalWrite(gpio, LOW);
            Serial.printf("💤 %s (GPIO %d) OFF\n", device.c_str(), gpio);
        }

        // Send confirmation back to client
        String response = "{\"page\":\"device\",\"device\":\"" + device + "\",\"status\":\"" + status + "\"}";
        ws.textAll(response);
    }
    else if (doc["page"] == "setting")
    {
        JsonObject value = doc["value"];
        String WIFI_SSID = value["ssid"].as<String>();
        String WIFI_PASS = value["password"].as<String>();
        String CORE_IOT_TOKEN = value["token"].as<String>();
        String CORE_IOT_SERVER = value["server"].as<String>();
        String CORE_IOT_PORT = value["port"].as<String>();

        Serial.println("📥 Nhận cấu hình từ WebSocket:");
        Serial.println("SSID: " + WIFI_SSID);
        Serial.println("PASS: " + WIFI_PASS);
        Serial.println("TOKEN: " + CORE_IOT_TOKEN);
        Serial.println("SERVER: " + CORE_IOT_SERVER);
        Serial.println("PORT: " + CORE_IOT_PORT);

        // 👉 Gọi hàm lưu cấu hình
        Save_info_File(WIFI_SSID, WIFI_PASS, CORE_IOT_TOKEN, CORE_IOT_SERVER, CORE_IOT_PORT);

        // Phản hồi lại client
        String msg = "{\"status\":\"ok\",\"page\":\"setting_saved\"}";
        ws.textAll(msg);
    }
}
