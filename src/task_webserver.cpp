#include "task_webserver.h"
#include <ArduinoJson.h>
#include <Preferences.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
bool webserver_isrunning = false;


// Gửi dữ liệu qua websocket tới tất cả client
void Webserver_sendata(const String &data)
{
    if (ws.count() > 0)
    {
        ws.textAll(data);
        Serial.println("Đã gửi dữ liệu qua WebSocket: " + data);
    }
    else
    {
        Serial.println("Không có client WebSocket nào đang kết nối!");
    }
}

void onEvent(AsyncWebSocket *serverPtr, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
        {
            AwsFrameInfo *info = (AwsFrameInfo *)arg;
            if (info->opcode == WS_TEXT)
            {
                // build string safely
                String message;
                message.reserve(len + 1);
                message = String((char *)data).substring(0, len);
                // xử lý message (đảm bảo hàm handleWebSocketMessage có trong task_handler.h)
                handleWebSocketMessage(message);
            }
            else if (info->opcode == WS_BINARY)
            {
                // nếu cần xử lý binary frames, thêm code ở đây
                Serial.printf("Received binary frame, len=%d\n", (int)len);
            }
            break;
        }
        case WS_EVT_PONG:
            // optional
            break;
        case WS_EVT_ERROR:
            Serial.println("WebSocket error event");
            break;
    }
}

void connectWSV()
{
    if (webserver_isrunning) return;

    // Lưu ý: LittleFS.begin() phải đã được gọi ở setup() trước
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "application/javascript"); });
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles.css", "text/css"); });

    // optional: fallback not found
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });

    server.begin();
    ElegantOTA.begin(&server); // đăng ký OTA với server
    webserver_isrunning = true;
    Serial.println("Webserver started");
}

void Webserver_stop()
{
    Serial.println("Stopping webserver...");
    ws.closeAll();
    server.end();
    webserver_isrunning = false;
}

void Webserver_reconnect()
{
    // đảm bảo LittleFS đã khởi tạo trước khi connect
    if (!webserver_isrunning)
    {
        connectWSV();
    }
    // **NOTES**: ElegantOTA.loop() nên được gọi định kỳ (ví dụ trong loop() hoặc task), 
    // gọi ở đây chỉ khi reconnect thôi sẽ không đủ nếu bạn muốn OTA hoạt động luôn.
    ElegantOTA.loop();
}
