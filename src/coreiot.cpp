#include "coreiot.h"


#define LED1_GPIO 2
#define LED2_GPIO 4

WiFiClient espClient;
PubSubClient client(espClient);


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Check if CoreIOT configuration is available
    if (CORE_IOT_SERVER.isEmpty() || CORE_IOT_TOKEN.isEmpty() || CORE_IOT_PORT.isEmpty()) {
      Serial.println("CoreIOT configuration not set. Please configure via web interface.");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      continue;
    }

    Serial.print("Attempting MQTT connection to ");
    Serial.print(CORE_IOT_SERVER);
    Serial.print(":");
    Serial.println(CORE_IOT_PORT);
    
    // Generate unique client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Connect with token 
    if (client.connect(clientId.c_str(), CORE_IOT_TOKEN.c_str(), "")) {
      Serial.println("Connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");
    } else {
      Serial.print("Connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" - retrying in 5 seconds");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("RPC Message arrived [");
  Serial.print(topic);
  Serial.println("]");

  // Extract request ID from topic: v1/devices/me/rpc/request/{requestId}
  String topicStr = String(topic);
  int lastSlash = topicStr.lastIndexOf('/');
  String requestId = topicStr.substring(lastSlash + 1);
  
  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    
    // Send error response back to CoreIOT
    String responseTopic = "v1/devices/me/rpc/response/" + requestId;
    String response = "{\"success\":false,\"error\":\"Invalid JSON\"}";
    client.publish(responseTopic.c_str(), response.c_str());
    return;
  }

  // Extract method and params
  const char* method = doc["method"];
  JsonVariant params = doc["params"];
  
  Serial.print("Method: ");
  Serial.println(method);

  // Handle different RPC methods
  if (strcmp(method, "setLED1") == 0) {
    // Control LED1 (GPIO 1)
    // Handle both boolean and string parameters
    bool ledState = false;
    
    if (params.is<bool>()) {
      // Boolean parameter (true/false) - from CoreIOT switch widget
      ledState = params.as<bool>();
      Serial.print("Received boolean parameter: ");
      Serial.println(ledState ? "true" : "false");
    } else if (params.is<String>()) {
      // String parameter (ON/OFF) - legacy support
      String state = params.as<String>();
      ledState = state.equalsIgnoreCase("ON") || state.equalsIgnoreCase("true");
      Serial.print("Received string parameter: ");
      Serial.println(state);
    } else {
      Serial.println("Unknown parameter type for setLED1");
      ledState = false;
    }
    
    pinMode(LED1_GPIO, OUTPUT);
    digitalWrite(LED1_GPIO, ledState ? HIGH : LOW);
    
    Serial.print(ledState ? "LED1 turned ON" : "LED1 turned OFF");
    Serial.println();
    
    // Send response back to CoreIOT
    String responseTopic = "v1/devices/me/rpc/response/" + requestId;
    String response = "{\"success\":true}";
    client.publish(responseTopic.c_str(), response.c_str());
    
  } else if (strcmp(method, "setLED2") == 0) {
    // Control LED2 (GPIO 4)
    // Handle both boolean and string parameters
    bool ledState = false;
    
    if (params.is<bool>()) {
      // Boolean parameter (true/false) - from CoreIOT switch widget
      ledState = params.as<bool>();
      Serial.print("Received boolean parameter: ");
      Serial.println(ledState ? "true" : "false");
    } else if (params.is<String>()) {
      // String parameter (ON/OFF) - legacy support
      String state = params.as<String>();
      ledState = state.equalsIgnoreCase("ON") || state.equalsIgnoreCase("true");
      Serial.print("Received string parameter: ");
      Serial.println(state);
    } else {
      Serial.println("Unknown parameter type for setLED2");
      ledState = false;
    }
    
    pinMode(LED2_GPIO, OUTPUT);
    digitalWrite(LED2_GPIO, ledState ? HIGH : LOW);
    
    Serial.print(ledState ? "LED2 (GPIO 4) turned ON" : "LED2 (GPIO 4) turned OFF");
    Serial.println();
    
    // Send response back to CoreIOT
    String responseTopic = "v1/devices/me/rpc/response/" + requestId;
    String response = "{\"success\":true}";
    client.publish(responseTopic.c_str(), response.c_str());
    
  } else if (strcmp(method, "setStateLED") == 0) {
    // Legacy method name - control both LEDs
    bool ledState = false;
    
    if (params.is<bool>()) {
      ledState = params.as<bool>();
    } else if (params.is<String>()) {
      String state = params.as<String>();
      ledState = state.equalsIgnoreCase("ON") || state.equalsIgnoreCase("true");
    }
    
    pinMode(LED1_GPIO, OUTPUT);
    pinMode(LED2_GPIO, OUTPUT);
    digitalWrite(LED1_GPIO, ledState ? HIGH : LOW);
    digitalWrite(LED2_GPIO, ledState ? HIGH : LOW);
    
    Serial.print(ledState ? "Both LEDs turned ON" : "Both LEDs turned OFF");
    Serial.println();
    
    // Send response back to CoreIOT
    String responseTopic = "v1/devices/me/rpc/response/" + requestId;
    String response = "{\"success\":true}";
    client.publish(responseTopic.c_str(), response.c_str());
    
  } else {
    Serial.print("Unknown method: ");
    Serial.println(method);
    
    // Send error response
    String responseTopic = "v1/devices/me/rpc/response/" + requestId;
    String response = "{\"success\":false,\"error\":\"Unknown method\"}";
    client.publish(responseTopic.c_str(), response.c_str());
  }
}


void setup_coreiot(){
  Serial.println("🔌 CoreIOT Task: Waiting for WiFi connection...");
  
  // Wait for WiFi connection using semaphore (released by WiFi task when connected)
  while(1){
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
      break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print(".");
  }

  Serial.println("WiFi Connected!");
  
  // Verify CoreIOT configuration is available
  if (CORE_IOT_SERVER.isEmpty() || CORE_IOT_TOKEN.isEmpty() || CORE_IOT_PORT.isEmpty()) {
    Serial.println("WARNING: CoreIOT configuration not set!");
    Serial.println("   Please configure via web interface:");
    Serial.println("   - Server: " + CORE_IOT_SERVER);
    Serial.println("   - Token: " + String(CORE_IOT_TOKEN.isEmpty() ? "NOT SET" : "SET"));
    Serial.println("   - Port: " + CORE_IOT_PORT);
    Serial.println("   The task will wait for configuration...");
  } else {
    Serial.println("CoreIOT Configuration:");
    Serial.println("   Server: " + CORE_IOT_SERVER);
    Serial.println("   Port: " + CORE_IOT_PORT);
    Serial.println("   Token: " + CORE_IOT_TOKEN.substring(0, 10) + "...");
  }

  // Configure MQTT client
  client.setServer(CORE_IOT_SERVER.c_str(), CORE_IOT_PORT.toInt());
  client.setCallback(callback);
  
  // Initialize LED pins
  pinMode(LED1_GPIO, OUTPUT);
  pinMode(LED2_GPIO, OUTPUT);
  digitalWrite(LED1_GPIO, LOW);
  digitalWrite(LED2_GPIO, LOW);
  
  Serial.println("CoreIOT setup complete!");
}

void coreiot_task(void *pvParameters){
    AppContext_t *ctx = (AppContext_t *) pvParameters;
    
    setup_coreiot();

    unsigned long lastTelemetryTime = 0;
    const unsigned long TELEMETRY_INTERVAL = 10000; // 10 seconds
    TelemetryData_t latestTelemetry = {0.0f, 0.0f, 0}; // Cache latest telemetry data

    while(1){
        // Check if CoreIOT configuration is available before attempting connection
        if (CORE_IOT_SERVER.isEmpty() || CORE_IOT_TOKEN.isEmpty() || CORE_IOT_PORT.isEmpty()) {
            Serial.println("Waiting for CoreIOT configuration...");
            vTaskDelay(10000 / portTICK_PERIOD_MS);  // Wait 10 seconds before checking again
            continue;
        }

        // Maintain MQTT connection
        if (!client.connected()) {
            reconnect();
        }
        
        // Process incoming messages frequently (critical for RPC commands)
        // This must be called often to receive RPC messages in real-time
        client.loop();

        // Receive telemetry data from manager task (non-blocking)
        TelemetryData_t telemetry;
        if (ctx->telemetryQueue != nullptr && 
            xQueueReceive(ctx->telemetryQueue, &telemetry, 0) == pdTRUE) {
            // Update cached telemetry with latest data
            latestTelemetry = telemetry;
        }

        // Publish telemetry every 10 seconds (not every loop iteration)
        unsigned long currentTime = millis();
        if (currentTime - lastTelemetryTime >= TELEMETRY_INTERVAL) {
            // Only publish if we have valid data (timestamp > 0)
            if (latestTelemetry.timestamp > 0) {
                // Publish sensor data (temperature & humidity) to CoreIOT telemetry
                // Format: {"temperature": 25.5, "humidity": 60.2}
                String payload = "{\"temperature\":" + String(latestTelemetry.temperature, 1) + 
                                ",\"humidity\":" + String(latestTelemetry.humidity, 1) + "}";
                
                if (client.publish("v1/devices/me/telemetry", payload.c_str())) {
                    Serial.println("📤 Published telemetry: " + payload);
                } else {
                    Serial.println("❌ Failed to publish telemetry");
                }
            } else {
                Serial.println("⏳ Waiting for sensor data before publishing...");
            }
            
            lastTelemetryTime = currentTime;
        }

        // Small delay to prevent CPU spinning, but keep loop() called frequently
        // 10ms delay for responsive RPC handling
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}