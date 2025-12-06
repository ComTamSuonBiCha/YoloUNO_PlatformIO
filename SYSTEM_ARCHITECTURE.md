# System Architecture Design

## 1. Executive Summary

This document describes the design and architecture of an ESP32-based IoT system for environmental monitoring with TinyML anomaly detection, web-based device control, and cloud integration. The system utilizes FreeRTOS for real-time task management and implements a multi-layered architecture for scalability and maintainability.

## 2. System Overview

### 2.1 Purpose
The system monitors temperature and humidity using a DHT20 sensor, processes data through a TinyML model for anomaly detection, provides visual feedback through LEDs and NeoPixels, displays information on an LCD, and offers remote control capabilities through a web interface and cloud platform (CoreIOT).

### 2.2 Key Features
- **Real-time Environmental Monitoring**: DHT20 sensor for temperature and humidity
- **TinyML Anomaly Detection**: On-device machine learning for identifying abnormal patterns
- **Multi-Mode Operation**: Access Point (AP) mode and Station (STA) mode WiFi connectivity
- **Web Interface**: Modern responsive UI for device control and monitoring
- **Cloud Integration**: CoreIOT platform for data telemetry and remote procedure calls (RPC)
- **Visual Feedback**: LED patterns, NeoPixel colors, and LCD display
- **OTA Updates**: Over-the-air firmware updates via ElegantOTA

## 3. High-Level System Architecture

```mermaid
graph TB
    subgraph "Physical Layer"
        DHT20[DHT20 Sensor<br/>Temperature & Humidity]
        LED1[LED GPIO 2]
        LED2[LED GPIO 4]
        NEO[NeoPixel RGB LED]
        LCD[LCD Display I2C]
        BOOT[Boot Button GPIO]
    end
    
    subgraph "ESP32 Platform"
        subgraph "FreeRTOS Kernel"
            SCHEDULER[Task Scheduler]
            QUEUE[Queue Manager]
            SEMA[Semaphore Manager]
        end
        
        subgraph "Application Tasks"
            SENSOR[Temp/Humi Monitor<br/>Task]
            MANAGER[Manager Task<br/>Decision Engine]
            LED_T[LED Blinky Task]
            NEO_T[NeoPixel Task]
            LCD_T[LCD Display Task]
            TINYML[TinyML Inference<br/>Task]
            WEB[Web Server Task]
            WIFI[WiFi Manager Task]
            COREIOT[CoreIOT Cloud Task]
        end
        
        subgraph "Communication Layer"
            WS[WebSocket Server]
            MQTT[MQTT Client]
            HTTP[HTTP Server]
            DNS[DNS Server<br/>Captive Portal]
        end
        
        subgraph "Storage Layer"
            LITTLEFS[LittleFS<br/>Web Assets]
        end
        
        subgraph "ML Layer"
            TFLITE[TFLite Micro<br/>Interpreter]
            MODEL[Anomaly Detection<br/>Model]
        end
    end
    
    subgraph "Network Layer"
        AP_MODE[WiFi AP Mode<br/>192.168.4.1]
        STA_MODE[WiFi STA Mode<br/>Internet Access]
    end
    
    subgraph "External Systems"
        BROWSER[Web Browser<br/>User Interface]
        CLOUD[CoreIOT Platform<br/>MQTT Broker]
    end
    
    %% Physical connections
    DHT20 -->|I2C Data| SENSOR
    SENSOR -->|SensorSample_t| QUEUE
    QUEUE -->|Distribute| MANAGER
    QUEUE -->|Distribute| TINYML
    
    MANAGER -->|LedPattern_t| LED_T
    MANAGER -->|NeoColor_t| NEO_T
    MANAGER -->|State Update| LCD_T
    MANAGER -->|TelemetryData_t| COREIOT
    
    LED_T -->|Control| LED1
    LED_T -->|Control| LED2
    NEO_T -->|Control| NEO
    LCD_T -->|I2C Display| LCD
    
    TINYML -.->|Anomaly Score| MANAGER
    TFLITE -->|Inference| MODEL
    
    %% Network connections
    WEB -->|Serve Files| LITTLEFS
    WEB -->|WebSocket| WS
    WS -.->|JSON Messages| BROWSER
    
    WIFI -->|Configure| AP_MODE
    WIFI -->|Configure| STA_MODE
    
    AP_MODE -.->|WiFi| BROWSER
    STA_MODE -.->|Internet| CLOUD
    
    COREIOT -->|Telemetry| MQTT
    MQTT -.->|Publish/Subscribe| CLOUD
    
    HTTP -->|OTA Updates| WEB
    DNS -->|Captive Portal| WEB
    
    SCHEDULER -.->|Schedule| SENSOR
    SCHEDULER -.->|Schedule| MANAGER
    SCHEDULER -.->|Schedule| LED_T
    SCHEDULER -.->|Schedule| NEO_T
    SCHEDULER -.->|Schedule| LCD_T
    SCHEDULER -.->|Schedule| TINYML
    SCHEDULER -.->|Schedule| WEB
    SCHEDULER -.->|Schedule| WIFI
    SCHEDULER -.->|Schedule| COREIOT

    style ESP32 Platform fill:#e1f5ff
    style Physical Layer fill:#fff4e6
    style Network Layer fill:#f3e5f5
    style External Systems fill:#e8f5e9
```

## 4. Software Architecture

### 4.1 Layered Architecture

```mermaid
graph TB
    subgraph "Presentation Layer"
        UI[Web UI<br/>HTML/CSS/JS]
        WS_API[WebSocket API]
        MQTT_API[MQTT RPC API]
    end
    
    subgraph "Application Layer"
        TASK_MGR[Task Manager]
        SENSOR_MGR[Sensor Manager]
        DEVICE_CTRL[Device Controller]
        ML_ENGINE[ML Inference Engine]
        CLOUD_MGR[Cloud Manager]
    end
    
    subgraph "Business Logic Layer"
        DECISION[Decision Engine<br/>Pattern/Color/State Mapping]
        TELEMETRY[Telemetry Processor]
        ANOMALY[Anomaly Detector]
        CONFIG[Configuration Manager]
    end
    
    subgraph "Data Access Layer"
        QUEUE_MGR[Queue Management]
        MUTEX[Mutex/Semaphore Control]
        FS[Filesystem Access]
        FLASH[Flash Storage]
    end
    
    subgraph "Hardware Abstraction Layer"
        I2C[I2C Driver]
        GPIO[GPIO Driver]
        WIFI_DRV[WiFi Driver]
        PWM[PWM Driver]
    end
    
    subgraph "Hardware Layer"
        HW[ESP32 Hardware<br/>Sensors & Actuators]
    end
    
    UI --> WS_API
    WS_API --> DEVICE_CTRL
    MQTT_API --> CLOUD_MGR
    
    DEVICE_CTRL --> GPIO
    SENSOR_MGR --> I2C
    ML_ENGINE --> ANOMALY
    
    TASK_MGR --> QUEUE_MGR
    CLOUD_MGR --> WIFI_DRV
    
    DECISION --> TELEMETRY
    ANOMALY --> ML_ENGINE
    CONFIG --> FS
    
    QUEUE_MGR --> MUTEX
    FS --> FLASH
    
    I2C --> HW
    GPIO --> HW
    WIFI_DRV --> HW
    PWM --> HW
```

### 4.2 Component Architecture

```mermaid
graph LR
    subgraph "Sensor Component"
        DHT20_DRV[DHT20 Driver]
        TEMP_TASK[Temperature/Humidity<br/>Monitor Task]
    end
    
    subgraph "Processing Component"
        MGR_TASK[Manager Task]
        TINYML_TASK[TinyML Task]
        DECISION_LOGIC[Decision Logic]
    end
    
    subgraph "Output Component"
        LED_TASK[LED Blinky Task]
        NEO_TASK[NeoPixel Task]
        LCD_TASK[LCD Task]
    end
    
    subgraph "Communication Component"
        WEB_TASK[Web Server Task]
        WIFI_TASK[WiFi Task]
        CORE_TASK[CoreIOT Task]
    end
    
    subgraph "Data Component"
        SENSOR_Q[Sensor Queue]
        LED_Q[LED Queue]
        NEO_Q[Neo Queue]
        TINYML_Q[TinyML Queue]
        TELEM_Q[Telemetry Queue]
        STATE_SEM[State Semaphore]
        DATA_MTX[Data Mutex]
    end
    
    DHT20_DRV --> TEMP_TASK
    TEMP_TASK --> SENSOR_Q
    SENSOR_Q --> MGR_TASK
    SENSOR_Q --> TINYML_TASK
    
    MGR_TASK --> DECISION_LOGIC
    DECISION_LOGIC --> LED_Q
    DECISION_LOGIC --> NEO_Q
    DECISION_LOGIC --> STATE_SEM
    DECISION_LOGIC --> TELEM_Q
    
    LED_Q --> LED_TASK
    NEO_Q --> NEO_TASK
    STATE_SEM --> LCD_TASK
    DATA_MTX --> LCD_TASK
    
    TELEM_Q --> CORE_TASK
    WIFI_TASK --> CORE_TASK
    WIFI_TASK --> WEB_TASK
    
    TINYML_TASK -.->|Anomaly Score| MGR_TASK
    WEB_TASK -.->|Control Commands| LED_TASK
    CORE_TASK -.->|RPC Commands| LED_TASK
```

## 5. Task Architecture

### 5.1 FreeRTOS Task Diagram

```mermaid
graph TB
    subgraph "Task Priority Levels"
        subgraph "Priority 3 - High"
            MANAGER[Manager Task<br/>Stack: 4096<br/>Period: Event-driven]
            SENSOR[Temp/Humi Monitor<br/>Stack: 2048<br/>Period: 1000ms]
            WIFI[WiFi STA Task<br/>Stack: 4096<br/>Period: Event-driven]
            COREIOT[CoreIOT Task<br/>Stack: 4096<br/>Period: 5000ms]
        end
        
        subgraph "Priority 2 - Medium"
            LED[LED Blinky Task<br/>Stack: 2048<br/>Period: Dynamic]
            NEO[NeoPixel Task<br/>Stack: 2048<br/>Period: Event-driven]
            LCD[LCD Task<br/>Stack: 4096<br/>Period: Event-driven]
            WEB[Web Server Task<br/>Stack: 8192<br/>Period: Event-driven]
            TINYML[TinyML Task<br/>Stack: 8192<br/>Period: Event-driven]
        end
    end
    
    subgraph "IPC Mechanisms"
        Q1[sensorQueue<br/>SensorSample_t<br/>Size: 10]
        Q2[ledQueue<br/>LedPattern_t<br/>Size: 5]
        Q3[neoQueue<br/>NeoColor_t<br/>Size: 5]
        Q4[tinymlQueue<br/>SensorSample_t<br/>Size: 10]
        Q5[telemetryQueue<br/>TelemetryData_t<br/>Size: 5]
        S1[stateSemaphore<br/>Binary]
        M1[dataMutex<br/>Mutex]
    end
    
    SENSOR -->|xQueueSend| Q1
    Q1 -->|xQueueReceive| MANAGER
    Q1 -->|xQueueReceive| TINYML
    
    MANAGER -->|xQueueSend| Q2
    MANAGER -->|xQueueSend| Q3
    MANAGER -->|xQueueSend| Q5
    MANAGER -->|xSemaphoreGive| S1
    
    Q2 -->|xQueueReceive| LED
    Q3 -->|xQueueReceive| NEO
    Q5 -->|xQueueReceive| COREIOT
    S1 -->|xSemaphoreTake| LCD
    
    MANAGER -.->|Lock| M1
    LCD -.->|Lock| M1
    
    TINYML -.->|Anomaly Detection| MANAGER
    
    style Priority 3 - High fill:#ffebee
    style Priority 2 - Medium fill:#e3f2fd
```

### 5.2 Task Interaction Sequence

```mermaid
sequenceDiagram
    participant DHT as DHT20 Sensor
    participant TH as Temp/Humi Task
    participant SQ as Sensor Queue
    participant MGR as Manager Task
    participant TML as TinyML Task
    participant LED as LED Task
    participant NEO as NeoPixel Task
    participant LCD as LCD Task
    participant CORE as CoreIOT Task
    
    loop Every 1000ms
        TH->>DHT: Read I2C Data
        DHT-->>TH: Temperature & Humidity
        TH->>SQ: Send SensorSample_t
        
        par Parallel Processing
            SQ->>MGR: Receive Sample
            and
            SQ->>TML: Receive Sample
        end
        
        TML->>TML: Run Inference
        TML->>TML: Calculate Anomaly Score
        
        MGR->>MGR: Map Temp → LED Pattern
        MGR->>MGR: Map Humidity → Neo Color
        MGR->>MGR: Calculate System State
        
        par Send to Actuators
            MGR->>LED: Send LedPattern_t
            and
            MGR->>NEO: Send NeoColor_t
            and
            MGR->>LCD: Signal State Change
            and
            MGR->>CORE: Send TelemetryData_t
        end
        
        LED->>LED: Update Blink Rate
        NEO->>NEO: Set RGB Color
        LCD->>LCD: Update Display
        
        alt WiFi Connected
            CORE->>CORE: Publish to MQTT
        end
    end
```

## 6. Data Flow Architecture

### 6.1 Sensor Data Flow

```mermaid
flowchart TD
    START([DHT20 Sensor Reading])
    READ[Read Temperature<br/>& Humidity via I2C]
    PACKAGE[Package into<br/>SensorSample_t]
    QUEUE_SEND[Send to sensorQueue]
    
    RECEIVE_MGR[Manager Task<br/>Receives Sample]
    RECEIVE_TML[TinyML Task<br/>Receives Sample]
    
    MAP_TEMP[Map Temperature<br/>to LED Pattern]
    MAP_HUM[Map Humidity<br/>to NeoPixel Color]
    MAP_STATE[Calculate<br/>System State]
    
    INFER[TFLite Inference]
    ANOMALY[Calculate Anomaly Score]
    CLASSIFY{Is Anomaly?}
    
    UPDATE_LED[Update LED Queue]
    UPDATE_NEO[Update Neo Queue]
    UPDATE_STATE[Update Shared State<br/>with Mutex]
    SIGNAL_LCD[Signal LCD Task<br/>via Semaphore]
    SEND_TELEM[Send to<br/>Telemetry Queue]
    
    DISPLAY[LED Blinks<br/>Neo Glows<br/>LCD Updates</br>Webserver UI]
    CLOUD[Publish to<br/>CoreIOT MQTT]
    
    LOG_ANOMALY[Log Anomaly<br/>to Serial]
    
    START --> READ
    READ --> PACKAGE
    PACKAGE --> |Sensor queue| QUEUE_SEND
    
    QUEUE_SEND -->|Manager queue| RECEIVE_MGR
    QUEUE_SEND -->|TinyML Queue| RECEIVE_TML
    
    RECEIVE_MGR --> MAP_TEMP
    RECEIVE_MGR --> MAP_HUM
    RECEIVE_MGR --> MAP_STATE
    
    RECEIVE_TML --> INFER
    INFER --> ANOMALY
    ANOMALY --> CLASSIFY
    
    MAP_TEMP --> |Led queue| UPDATE_LED
    MAP_HUM --> |Neopixel queue| UPDATE_NEO
    MAP_STATE --> UPDATE_STATE
    
    UPDATE_LED --> DISPLAY
    UPDATE_NEO --> DISPLAY
    UPDATE_STATE --> SIGNAL_LCD
    SIGNAL_LCD --> DISPLAY
    
    UPDATE_STATE --> SEND_TELEM
    SEND_TELEM --> CLOUD
    
    CLASSIFY -->|Yes| LOG_ANOMALY
    CLASSIFY -->|No| DISPLAY
    LOG_ANOMALY --> DISPLAY
    
    style START fill:#e1f5ff
    style DISPLAY fill:#c8e6c9
    style CLOUD fill:#fff9c4
    style LOG_ANOMALY fill:#ffccbc
```

### 6.2 Control Flow (User Commands)

```mermaid
flowchart TD
    START_WEB([User Action on<br/>Web Interface])
    START_CLOUD([RPC Command from<br/>CoreIOT Platform])
    
    WS_MSG[WebSocket Message<br/>JSON Format]
    MQTT_MSG[MQTT RPC Message<br/>JSON Format]
    
    PARSE_WEB[Parse JSON<br/>Extract device, gpio, status]
    PARSE_CLOUD[Parse JSON<br/>Extract method, params]
    
    VALIDATE{Valid Command?}
    
    GPIO_CTRL[Set GPIO State<br/>digitalWrite]
    
    CONFIRM_WEB[Send Confirmation<br/>via WebSocket]
    CONFIRM_CLOUD[Publish Response<br/>to RPC topic]
    
    ERROR[Send Error Message]
    
    END([Device State Updated])
    
    START_WEB --> WS_MSG
    START_CLOUD --> MQTT_MSG
    
    WS_MSG --> PARSE_WEB
    MQTT_MSG --> PARSE_CLOUD
    
    PARSE_WEB --> VALIDATE
    PARSE_CLOUD --> VALIDATE
    
    VALIDATE -->|Yes| GPIO_CTRL
    VALIDATE -->|No| ERROR
    
    GPIO_CTRL --> CONFIRM_WEB
    GPIO_CTRL --> CONFIRM_CLOUD
    
    CONFIRM_WEB --> END
    CONFIRM_CLOUD --> END
    ERROR --> END
    
    style START_WEB fill:#e1f5ff
    style START_CLOUD fill:#f3e5f5
    style ERROR fill:#ffcdd2
    style END fill:#c8e6c9
```

## 7. Network Architecture

### 7.1 WiFi Operation Modes

```mermaid
graph TB
    subgraph "ESP32 WiFi Modes"
        AP[Access Point Mode<br/>SSID: ESP32 LOCAL<br/>IP: 192.168.4.1<br/>Password: 12345678]
        STA[Station Mode<br/>Connects to External WiFi<br/>Dynamic IP via DHCP]
    end
    
    subgraph "AP Mode Services"
        DNS_SERVER[DNS Server<br/>Captive Portal<br/>Port 53]
        WEB_SERVER[AsyncWebServer<br/>Port 80]
        WS_SERVER[WebSocket<br/>Endpoint: /ws]
        OTA_SERVER[ElegantOTA<br/>Firmware Updates]
    end
    
    subgraph "STA Mode Services"
        MQTT_CLIENT[MQTT Client<br/>CoreIOT Connection]
        NTP[NTP Client<br/>Time Sync]
    end
    
    subgraph "Clients"
        PHONE[Mobile Phone<br/>Web Browser]
        LAPTOP[Laptop<br/>Web Browser]
        IOT_PLATFORM[CoreIOT Platform<br/>MQTT Broker]
    end
    
    AP --> DNS_SERVER
    AP --> WEB_SERVER
    WEB_SERVER --> WS_SERVER
    WEB_SERVER --> OTA_SERVER
    
    STA --> MQTT_CLIENT
    STA --> NTP
    
    PHONE -.->|WiFi| AP
    LAPTOP -.->|WiFi| AP
    
    PHONE --> DNS_SERVER
    PHONE --> WEB_SERVER
    LAPTOP --> WEB_SERVER
    
    STA -.->|Internet| IOT_PLATFORM
    MQTT_CLIENT --> IOT_PLATFORM
    
    style AP fill:#ffebee
    style STA fill:#e3f2fd
    style Clients fill:#f1f8e9
```

### 7.2 Communication Protocols

```mermaid
graph LR
    subgraph "Internal Communication"
        I2C[I2C Protocol<br/>DHT20, LCD<br/>400kHz]
        GPIO_PWM[GPIO/PWM<br/>LEDs, NeoPixel]
    end
    
    subgraph "Network Communication"
        WEBSOCKET[WebSocket<br/>Real-time Bidirectional<br/>JSON Messages]
        HTTP[HTTP/HTTPS<br/>Web Page Serving<br/>OTA Updates]
        MQTT[MQTT v3.1.1<br/>Telemetry & RPC<br/>QoS 0/1]
        DNS_PROTO[DNS<br/>Captive Portal<br/>Port 53]
    end
    
    subgraph "Data Formats"
        JSON[JSON<br/>Configuration & Commands]
        BINARY[Binary<br/>TFLite Model]
        HTML[HTML/CSS/JS<br/>Web Assets]
    end
    
    I2C --> JSON
    WEBSOCKET --> JSON
    HTTP --> HTML
    MQTT --> JSON
    GPIO_PWM --> BINARY
    
    style Internal Communication fill:#e1f5ff
    style Network Communication fill:#f3e5f5
    style Data Formats fill:#fff9c4
```

## 8. TinyML Architecture

### 8.1 Machine Learning Pipeline

```mermaid
flowchart LR
    subgraph "Training Phase (Offline)"
        DATASET[HCMC Weather Dataset<br/>CSV Format]
        PREPROCESS[Data Preprocessing<br/>Normalization<br/>Train/Test Split]
        TRAIN[Model Training<br/>Keras/TensorFlow<br/>Autoencoder Architecture]
        CONVERT[TFLite Conversion<br/>Quantization<br/>Optimization]
        MODEL_FILE[dht_anomaly_model.tflite<br/>Embedded in Firmware]
    end
    
    subgraph "Inference Phase (On-Device)"
        SENSOR_DATA[Real-time Sensor Data<br/>Temperature & Humidity]
        NORMALIZE[Input Normalization<br/>Scale to 0, 1]
        TFLITE[TFLite Micro Interpreter<br/>8KB Tensor Arena]
        INFERENCE[Forward Pass<br/>Reconstruction]
        CALCULATE[Calculate MSE<br/>Anomaly Score]
        THRESHOLD{Score > Threshold?}
        NORMAL[Normal State<br/>Continue Monitoring]
        ANOMALY[Anomaly Detected<br/>Log & Alert]
    end
    
    DATASET --> PREPROCESS
    PREPROCESS --> TRAIN
    TRAIN --> CONVERT
    CONVERT --> MODEL_FILE
    
    MODEL_FILE -.->|Embedded| TFLITE
    
    SENSOR_DATA --> NORMALIZE
    NORMALIZE --> TFLITE
    TFLITE --> INFERENCE
    INFERENCE --> CALCULATE
    CALCULATE --> THRESHOLD
    
    THRESHOLD -->|No| NORMAL
    THRESHOLD -->|Yes| ANOMALY
    
    style Training Phase (Offline) fill:#e3f2fd
    style Inference Phase (On-Device) fill:#f1f8e9
    style ANOMALY fill:#ffccbc
```

### 8.2 TinyML Model Architecture

```mermaid
graph TB
    INPUT[Input Layer<br/>2 features<br/>Temperature, Humidity]
    
    ENCODE1[Encoder Layer 1<br/>Dense + ReLU<br/>8 neurons]
    ENCODE2[Encoder Layer 2<br/>Dense + ReLU<br/>4 neurons]
    ENCODE3[Encoder Layer 3<br/>Dense + ReLU<br/>2 neurons - Bottleneck]
    
    DECODE1[Decoder Layer 1<br/>Dense + ReLU<br/>4 neurons]
    DECODE2[Decoder Layer 2<br/>Dense + ReLU<br/>8 neurons]
    
    OUTPUT[Output Layer<br/>Dense + Sigmoid<br/>2 neurons<br/>Reconstructed Input]
    
    MSE[Calculate MSE<br/>Anomaly Score]
    
    INPUT --> ENCODE1
    ENCODE1 --> ENCODE2
    ENCODE2 --> ENCODE3
    ENCODE3 --> DECODE1
    DECODE1 --> DECODE2
    DECODE2 --> OUTPUT
    
    OUTPUT --> MSE
    INPUT -.->|Compare| MSE
    
    style INPUT fill:#e1f5ff
    style ENCODE3 fill:#ffccbc
    style OUTPUT fill:#c8e6c9
    style MSE fill:#fff9c4
```

## 9. State Management

### 9.1 System State Machine

```mermaid
stateDiagram-v2
    [*] --> INIT: Power On
    
    INIT --> NORMAL: Initialization Complete
    
    NORMAL --> WARNING: Temperature ≥ 30°C OR<br/>Humidity ≥ 70%
    NORMAL --> CRITICAL: Temperature ≥ 35°C OR<br/>Humidity ≥ 80%
    
    WARNING --> NORMAL: Temperature < 30°C AND<br/>Humidity < 70%
    WARNING --> CRITICAL: Temperature ≥ 35°C OR<br/>Humidity ≥ 80%
    
    CRITICAL --> WARNING: Temperature < 35°C AND<br/>Humidity < 80%
    CRITICAL --> NORMAL: Temperature < 30°C AND<br/>Humidity < 70%
    
    state NORMAL {
        [*] --> LED_SLOW
        [*] --> NEO_BLUE: Humidity < 40%
        [*] --> NEO_GREEN: 40% ≤ Humidity < 70%
        
        LED_SLOW: LED Blink: Slow (1000ms)
        NEO_BLUE: NeoPixel: Blue
        NEO_GREEN: NeoPixel: Green
    }
    
    state WARNING {
        [*] --> LED_MEDIUM
        [*] --> NEO_GREEN: 40% ≤ Humidity < 70%
        [*] --> NEO_RED: Humidity ≥ 70%
        
        LED_MEDIUM: LED Blink: Medium (500ms)
        NEO_GREEN: NeoPixel: Green
        NEO_RED: NeoPixel: Red
    }
    
    state CRITICAL {
        [*] --> LED_FAST
        [*] --> NEO_RED
        
        LED_FAST: LED Blink: Fast (200ms)
        NEO_RED: NeoPixel: Red
    }
    
    note right of NORMAL
        LCD Display:
        "NORMAL"
        Temp: XX.X°C
        Humi: XX.X%
    end note
    
    note right of WARNING
        LCD Display:
        "WARNING"
        Temp: XX.X°C
        Humi: XX.X%
    end note
    
    note right of CRITICAL
        LCD Display:
        "CRITICAL!"
        Temp: XX.X°C
        Humi: XX.X%
    end note
```

### 9.2 WiFi State Machine

```mermaid
stateDiagram-v2
    [*] --> AP_MODE: System Boot
    
    AP_MODE --> AP_RUNNING: Start AP<br/>SSID: ESP32 LOCAL
    
    AP_RUNNING --> STA_CONNECTING: WiFi Credentials<br/>Configured via Web UI
    
    STA_CONNECTING --> STA_CONNECTED: Connection Success
    STA_CONNECTING --> AP_RUNNING: Connection Failed<br/>Retry Later
    
    STA_CONNECTED --> DUAL_MODE: AP Still Active
    
    DUAL_MODE --> AP_RUNNING: WiFi Disconnected
    DUAL_MODE --> MQTT_CONNECTING: CoreIOT Config Available
    
    MQTT_CONNECTING --> MQTT_CONNECTED: MQTT Connected
    MQTT_CONNECTING --> DUAL_MODE: Connection Failed
    
    MQTT_CONNECTED --> DUAL_MODE: Disconnected
    
    state AP_RUNNING {
        [*] --> DNS_ACTIVE
        [*] --> WEB_ACTIVE
        [*] --> WS_ACTIVE
        
        DNS_ACTIVE: DNS Server Running<br/>Captive Portal
        WEB_ACTIVE: Web Server Running<br/>Port 80
        WS_ACTIVE: WebSocket Active<br/>/ws endpoint
    }
    
    state STA_CONNECTED {
        [*] --> IP_ASSIGNED
        [*] --> INTERNET_ACCESS
        
        IP_ASSIGNED: DHCP IP Obtained
        INTERNET_ACCESS: Internet Connectivity
    }
    
    state MQTT_CONNECTED {
        [*] --> TELEMETRY_ACTIVE
        [*] --> RPC_SUBSCRIBED
        
        TELEMETRY_ACTIVE: Publishing Sensor Data
        RPC_SUBSCRIBED: Listening for Commands
    }
```

## 10. Security Architecture

### 10.1 Security Layers

```mermaid
graph TB
    subgraph "Network Security"
        AP_PWD[AP Mode Password<br/>WPA2-PSK: 12345678]
        STA_PWD[Station Mode<br/>User-configured Credentials]
    end
    
    subgraph "Authentication"
        MQTT_TOKEN[MQTT Token Authentication<br/>Device Access Token]
        WS_ORIGIN[WebSocket Origin Check<br/>Optional CORS]
    end
    
    subgraph "Data Security"
        JSON_VALID[JSON Schema Validation<br/>Input Sanitization]
        FLASH_ENCRYPT[Flash Encryption<br/>Optional]
    end
    
    subgraph "Access Control"
        GPIO_CTRL[GPIO Access Control<br/>Authorized Commands Only]
        OTA_AUTH[OTA Authentication<br/>Optional Password]
    end
    
    subgraph "Secure Storage"
        LITTLEFS_SECURE[LittleFS Storage<br/>Web Assets]
        CONFIG_PERSIST[Persistent Configuration<br/>WiFi & CoreIOT Settings]
    end
    
    AP_PWD --> JSON_VALID
    STA_PWD --> MQTT_TOKEN
    MQTT_TOKEN --> GPIO_CTRL
    WS_ORIGIN --> GPIO_CTRL
    JSON_VALID --> LITTLEFS_SECURE
    OTA_AUTH --> FLASH_ENCRYPT
    
    style Network Security fill:#ffebee
    style Authentication fill:#e3f2fd
    style Data Security fill:#f3e5f5
    style Access Control fill:#fff9c4
    style Secure Storage fill:#f1f8e9
```

## 11. Data Models

### 11.1 Core Data Structures

```mermaid
classDiagram
    class SensorSample_t {
        +float temperature
        +float humidity
        +unsigned long timestamp
    }
    
    class LedPattern_t {
        <<enumeration>>
        LED_PATTERN_SLOW
        LED_PATTERN_MEDIUM
        LED_PATTERN_FAST
    }
    
    class NeoColor_t {
        +uint8_t r
        +uint8_t g
        +uint8_t b
    }
    
    class SystemState_t {
        <<enumeration>>
        STATE_NORMAL
        STATE_WARNING
        STATE_CRITICAL
    }
    
    class TelemetryData_t {
        +float temperature
        +float humidity
        +unsigned long timestamp
    }
    
    class AppContext_t {
        +QueueHandle_t sensorQueue
        +QueueHandle_t ledQueue
        +QueueHandle_t neoQueue
        +QueueHandle_t tinymlQueue
        +QueueHandle_t telemetryQueue
        +SemaphoreHandle_t stateSemaphore
        +SemaphoreHandle_t dataMutex
        +SystemState_t currentState
        +float lastTemperature
        +float lastHumidity
    }
    
    AppContext_t --> SensorSample_t : contains
    AppContext_t --> LedPattern_t : uses
    AppContext_t --> NeoColor_t : uses
    AppContext_t --> SystemState_t : manages
    AppContext_t --> TelemetryData_t : processes
```

### 11.2 Message Formats

```mermaid
graph TB
    subgraph "WebSocket Messages"
        WS_DEVICE[Device Control<br/>page: device<br/>device: LED1/LED2<br/>gpio: 2/4<br/>status: ON/OFF]
        
        WS_CONFIG[Configuration<br/>page: config<br/>ssid: string<br/>password: string<br/>coreiot_token: string<br/>coreiot_server: string<br/>coreiot_port: string]
        
        WS_STATUS[Status Update<br/>page: device<br/>device: LED1/LED2<br/>status: ON/OFF]
    end
    
    subgraph "MQTT Messages"
        MQTT_TELEM[Telemetry<br/>v1/devices/me/telemetry<br/>temperature: float<br/>humidity: float<br/>timestamp: long]
        
        MQTT_RPC_REQ[RPC Request<br/>v1/devices/me/rpc/request/+<br/>method: string<br/>params: object]
        
        MQTT_RPC_RES[RPC Response<br/>v1/devices/me/rpc/response/{id}<br/>success: boolean<br/>result/error: string]
    end
    
    style WebSocket Messages fill:#e1f5ff
    style MQTT Messages fill:#f3e5f5
```

## 12. Deployment Architecture

### 12.1 System Deployment

```mermaid
graph TB
    subgraph "Development Environment"
        VSCODE[VS Code + PlatformIO]
        PYTHON[Python Environment<br/>TensorFlow/Keras]
        GIT[Git Repository]
    end
    
    subgraph "Build Process"
        COMPILE[PlatformIO Build<br/>ESP32 Toolchain]
        MODEL_CONV[Model Conversion<br/>TFLite Converter]
        FS_BUILD[LittleFS Build<br/>Web Assets]
    end
    
    subgraph "Deployment"
        USB_FLASH[USB Serial Flash<br/>Initial Upload]
        OTA_UPDATE[OTA Update<br/>ElegantOTA]
    end
    
    subgraph "ESP32 Device"
        FIRMWARE[Firmware<br/>main.bin]
        ML_MODEL[TinyML Model<br/>Embedded in Firmware]
        FILESYSTEM[LittleFS<br/>index.html, script.js, styles.css]
    end
    
    subgraph "Cloud Services"
        COREIOT_CLOUD[CoreIOT Platform<br/>MQTT Broker<br/>Dashboard & Analytics]
    end
    
    VSCODE --> COMPILE
    PYTHON --> MODEL_CONV
    
    COMPILE --> FIRMWARE
    MODEL_CONV --> ML_MODEL
    FS_BUILD --> FILESYSTEM
    
    USB_FLASH --> FIRMWARE
    USB_FLASH --> FILESYSTEM
    OTA_UPDATE --> FIRMWARE
    
    FIRMWARE -.->|Telemetry| COREIOT_CLOUD
    COREIOT_CLOUD -.->|RPC| FIRMWARE
    
    GIT -.->|Version Control| VSCODE
    
    style Development Environment fill:#e3f2fd
    style Build Process fill:#fff9c4
    style Deployment fill:#f3e5f5
    style ESP32 Device fill:#c8e6c9
    style Cloud Services fill:#f1f8e9
```

## 13. Performance Considerations

### 13.1 Resource Allocation

| Component | Memory (RAM) | CPU Usage | Priority | Notes |
|-----------|-------------|-----------|----------|-------|
| Manager Task | 4 KB Stack | ~5% | High (3) | Decision engine |
| Sensor Monitor | 2 KB Stack | ~3% | High (3) | Periodic reading |
| TinyML Task | 8 KB Stack + 8 KB Tensor Arena | ~15% | Medium (2) | Inference overhead |
| Web Server | 8 KB Stack | ~10% | Medium (2) | HTTP/WebSocket |
| CoreIOT Task | 4 KB Stack | ~5% | High (3) | MQTT communication |
| LED Tasks | 2 KB Stack each | ~1% | Medium (2) | GPIO control |
| LCD Task | 4 KB Stack | ~2% | Medium (2) | I2C display |
| WiFi Task | 4 KB Stack | ~8% | High (3) | Network management |

### 13.2 Timing Requirements

```mermaid
gantt
    title Task Execution Timeline (1000ms Window)
    dateFormat X
    axisFormat %L ms
    
    section High Priority
    Sensor Read       :0, 50
    Manager Process   :50, 100
    CoreIOT Publish   :100, 150
    WiFi Management   :150, 200
    
    section Medium Priority
    TinyML Inference  :200, 350
    Web Server Handle :350, 400
    LED Update        :400, 420
    NeoPixel Update   :420, 440
    LCD Update        :440, 480
    
    section Idle
    System Idle       :480, 1000
```

## 14. Error Handling & Recovery

```mermaid
flowchart TD
    ERROR_DETECT{Error Detected}
    
    SENSOR_FAIL[Sensor Read Failure]
    WIFI_FAIL[WiFi Connection Lost]
    MQTT_FAIL[MQTT Disconnection]
    QUEUE_FULL[Queue Overflow]
    OOM[Out of Memory]
    MODEL_FAIL[TFLite Inference Failure]
    
    RETRY[Retry Operation<br/>Max 3 attempts]
    RESET_WIFI[Reset WiFi Stack]
    RECONNECT_MQTT[Reconnect MQTT]
    DROP_DATA[Drop Oldest Data]
    REBOOT[System Reboot]
    LOG_ERROR[Log to Serial<br/>Continue without ML]
    
    SUCCESS{Success?}
    CONTINUE[Continue Operation]
    ALERT[Send Alert via Serial]
    
    ERROR_DETECT -->|Sensor| SENSOR_FAIL
    ERROR_DETECT -->|Network| WIFI_FAIL
    ERROR_DETECT -->|Cloud| MQTT_FAIL
    ERROR_DETECT -->|Queue| QUEUE_FULL
    ERROR_DETECT -->|Memory| OOM
    ERROR_DETECT -->|ML| MODEL_FAIL
    
    SENSOR_FAIL --> RETRY
    WIFI_FAIL --> RESET_WIFI
    MQTT_FAIL --> RECONNECT_MQTT
    QUEUE_FULL --> DROP_DATA
    OOM --> REBOOT
    MODEL_FAIL --> LOG_ERROR
    
    RETRY --> SUCCESS
    RESET_WIFI --> SUCCESS
    RECONNECT_MQTT --> SUCCESS
    DROP_DATA --> CONTINUE
    LOG_ERROR --> CONTINUE
    
    SUCCESS -->|Yes| CONTINUE
    SUCCESS -->|No| ALERT
    ALERT --> CONTINUE
    
    style ERROR_DETECT fill:#ffebee
    style REBOOT fill:#ef5350
    style CONTINUE fill:#66bb6a
```

## 15. Future Enhancements

### 15.1 Potential Improvements

1. **Enhanced Security**
   - HTTPS support for web interface
   - Certificate-based MQTT authentication
   - Secure boot and flash encryption

2. **Advanced ML Features**
   - Multiple anomaly detection models
   - Online learning capabilities
   - Predictive maintenance alerts

3. **Extended Connectivity**
   - Bluetooth Low Energy (BLE) support
   - LoRaWAN integration for long-range communication
   - Multi-cloud platform support

4. **User Experience**
   - Mobile application (iOS/Android)
   - Push notifications
   - Historical data visualization
   - User authentication system

5. **Data Management**
   - Local data logging to SD card
   - Time-series database integration
   - Data compression and buffering

6. **Scalability**
   - Multi-device mesh networking
   - Device clustering and coordination
   - Load balancing for multiple clients

## 16. Conclusion

This ESP32-based IoT system demonstrates a comprehensive approach to environmental monitoring with edge AI capabilities. The architecture emphasizes:

- **Modularity**: Clear separation of concerns with FreeRTOS tasks
- **Real-time Performance**: Priority-based task scheduling
- **Scalability**: Queue-based communication for easy expansion
- **Intelligence**: On-device TinyML for anomaly detection
- **Connectivity**: Multi-mode WiFi with web and cloud integration
- **Reliability**: Error handling and recovery mechanisms

The system serves as a robust foundation for IoT applications requiring sensor monitoring, edge computing, and remote control capabilities.
