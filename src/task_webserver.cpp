#include "task_webserver.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;

bool webserver_isrunning = false;
const byte DNS_PORT = 53;

void Webserver_sendata(String data)
{
    if (ws.count() > 0)
    {
        ws.textAll(data); // Gửi đến tất cả client đang kết nối
        Serial.println("📤 Đã gửi dữ liệu qua WebSocket: " + data);
    }
    else
    {
        Serial.println("⚠️ Không có client WebSocket nào đang kết nối!");
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;

        if (info->opcode == WS_TEXT)
        {
            String message;
            message += String((char *)data).substring(0, len);
            // parseJson(message, true);
            handleWebSocketMessage(message);
        }
    }
}

void connnectWSV()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    
    // Main routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "application/javascript"); });
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles.css", "text/css"); });
    
    // Captive portal routes - redirect all unknown requests to index.html
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); }); // Android
    server.on("/fwlink", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); }); // Microsoft
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); }); // iOS/macOS
    
    // Catch-all handler for captive portal
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });
    
    server.begin();
    ElegantOTA.begin(&server);
    webserver_isrunning = true;
    
    Serial.println("✅ Captive Portal enabled - all requests redirect to web interface");
}

void Webserver_stop()
{
    ws.closeAll();
    server.end();
    webserver_isrunning = false;
}

void Webserver_reconnect()
{
    if (!webserver_isrunning)
    {
        connnectWSV();
    }
    ElegantOTA.loop();
}

void webserver_task(void *parameter)
{
    // Initialize LittleFS for serving web files
    if (!LittleFS.begin())
    {
        Serial.println("❌ LittleFS Mount Failed");
        vTaskDelete(NULL);
        return;
    }
    Serial.println("✅ LittleFS Mounted Successfully");

    // Start DNS server for captive portal
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    Serial.println("✅ DNS Server started for Captive Portal");

    // Start web server
    connnectWSV();
    Serial.println("🌐 Web Server Started in AP Mode with Captive Portal");
    Serial.print("📡 Connect to AP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("📡 Browser will auto-open when connected!");

    // Keep the task running and handle OTA + DNS
    while (true)
    {
        dnsServer.processNextRequest(); // Handle DNS requests for captive portal
        ElegantOTA.loop();
        ws.cleanupClients(); // Clean up disconnected WebSocket clients
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
