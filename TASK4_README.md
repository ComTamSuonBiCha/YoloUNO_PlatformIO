# Task 4: Web Server in Access Point Mode

## Overview
This implementation provides a modern, responsive web interface for controlling two LED devices through a web server running in Access Point (AP) mode on the ESP32.

## Features

### 1. **Modern Web Interface**
- Clean, responsive design with gradient colors and smooth animations
- Sidebar navigation with 4 sections:
  - 🏠 Home: Real-time temperature and humidity gauges
  - ⚡ Device Control: LED1 and LED2 control panel
  - ℹ️ Info: Device information
  - ⚙️ Settings: WiFi and IoT configuration

### 2. **Device Control (Task 4 Requirements)**
The interface includes controls for two devices:

#### **LED 1 (GPIO 2)**
- Individual ON/OFF toggle button
- Real-time status indicator with color-coded dot
- Status text display (Bật/Tắt)

#### **LED 2 (GPIO 4)**
- Individual ON/OFF toggle button
- Real-time status indicator with color-coded dot
- Status text display (Bật/Tắt)

#### **Group Controls**
- "Bật tất cả" button: Turns both LEDs ON
- "Tắt tất cả" button: Turns both LEDs OFF

### 3. **Communication**
- WebSocket-based real-time communication
- JSON message format for device control
- Automatic reconnection on connection loss
- Server-side confirmation of commands

## Technical Implementation

### Access Point Configuration
- **SSID**: `ESP32 LOCAL` (defined in platformio.ini)
- **Password**: `12345678` (defined in platformio.ini)
- **IP Address**: `192.168.4.1` (default ESP32 AP IP)

### File Structure
```
YoloUNO_PlatformIO/
├── data/
│   ├── index.html      # Main web interface
│   ├── script.js       # Device control logic
│   └── styles.css      # Modern UI styling
├── src/
│   ├── main.cpp        # Entry point with AP initialization
│   ├── task_webserver.cpp  # Web server task implementation
│   ├── task_handler.cpp    # WebSocket message handler
│   └── task_wifi.cpp   # WiFi AP setup
└── include/
    ├── task_webserver.h
    ├── task_handler.h
    └── task_wifi.h
```

### WebSocket Message Format

#### Device Control (Client → Server)
```json
{
  "page": "device",
  "device": "LED1",
  "gpio": 2,
  "status": "ON"
}
```

#### Status Confirmation (Server → Client)
```json
{
  "page": "device",
  "device": "LED1",
  "status": "ON"
}
```

### Code Components

#### 1. **main.cpp**
- Initializes WiFi in AP mode using `startAP()`
- Creates FreeRTOS task for web server
- Task priority: 2, Stack size: 8192 bytes

#### 2. **task_webserver.cpp**
- Mounts LittleFS filesystem for serving web files
- Configures AsyncWebServer on port 80
- Sets up WebSocket on `/ws` endpoint
- Serves HTML, CSS, and JavaScript files
- Includes ElegantOTA for firmware updates

#### 3. **task_handler.cpp**
- Parses incoming JSON messages
- Controls GPIO pins based on commands
- Validates message format
- Sends confirmation back to client

#### 4. **script.js (Frontend)**
Key Functions:
- `toggleLED1()`: Toggle LED1 state
- `toggleLED2()`: Toggle LED2 state
- `turnAllOn()`: Turn both LEDs on
- `turnAllOff()`: Turn both LEDs off
- `updateLED1UI()`: Update LED1 visual state
- `updateLED2UI()`: Update LED2 visual state

## Usage Instructions

### 1. **Connect to Access Point**
1. Upload the code to your ESP32
2. Open Serial Monitor to see the AP IP address
3. On your computer/phone, connect to WiFi:
   - **Network**: `ESP32 LOCAL`
   - **Password**: `12345678`

### 2. **Access Web Interface**
1. Open a web browser
2. Navigate to: `http://192.168.4.1`
3. The web interface will load automatically

### 3. **Control Devices**
1. Click on "⚡ Thiết bị" in the sidebar
2. Use the toggle buttons to control individual LEDs
3. Use "Bật tất cả" or "Tắt tất cả" for group control
4. Status indicators show real-time device states

### 4. **Configure Settings (Optional)**
1. Click on "⚙️ Cài đặt" in the sidebar
2. Enter WiFi credentials and IoT server details
3. Click "Lưu cấu hình" to save settings

## GPIO Pin Configuration
- **LED 1**: GPIO 2 (Built-in LED on most ESP32 boards)
- **LED 2**: GPIO 4 (Can be changed in script.js and connected externally)

## Design Features

### Visual Indicators
- **Green dot**: Device is ON
- **Red dot**: Device is OFF
- **Button color**: Gray (OFF) / Blue gradient (ON)
- **Hover effects**: Smooth transitions and shadows

### Responsive Design
- Works on desktop, tablet, and mobile devices
- Adaptive layout for different screen sizes
- Touch-friendly buttons and controls

### User Experience
- Instant visual feedback on actions
- Clear labeling in Vietnamese
- Intuitive icon-based navigation
- Smooth animations and transitions

## Testing

### Test Scenarios
1. **Single Device Control**
   - Toggle LED1 ON/OFF individually
   - Toggle LED2 ON/OFF individually
   - Verify status indicators update correctly

2. **Group Control**
   - Use "Bật tất cả" and verify both LEDs turn ON
   - Use "Tắt tất cả" and verify both LEDs turn OFF

3. **Connection Stability**
   - Disconnect and reconnect WiFi
   - Verify WebSocket reconnects automatically
   - Test commands after reconnection

4. **Multiple Clients**
   - Connect from multiple devices
   - Verify all clients see synchronized states

## Troubleshooting

### Cannot Connect to AP
- Verify AP is broadcasting (check Serial Monitor)
- Ensure password is correct: `12345678`
- Try forgetting the network and reconnecting

### Web Page Not Loading
- Confirm you're connected to the correct AP
- Verify IP address is `192.168.4.1`
- Check if LittleFS mounted successfully (Serial Monitor)

### LEDs Not Responding
- Check GPIO pin connections
- Verify JSON message format in browser console
- Check Serial Monitor for error messages

### WebSocket Connection Issues
- Refresh the web page
- Check browser console for WebSocket errors
- Verify WebSocket endpoint: `ws://192.168.4.1/ws`

## Future Enhancements
- Add more device controls (relays, motors, etc.)
- Implement authentication/security
- Add device scheduling features
- Store device states in EEPROM/SPIFFS
- Add data logging and history
- Support for PWM control (dimming)

## Compliance with Task 4 Requirements

✅ **Web Server in Access Point Mode**: Implemented with ESP32 AP  
✅ **Redesigned Interface**: Modern, responsive web UI  
✅ **Two Device Controls**: LED1 and LED2 with dedicated controls  
✅ **At Least Two Buttons**: Multiple buttons per device plus group controls  
✅ **Properly Labeled Actions**: Clear Vietnamese labels and icons  
✅ **Better Usability**: Intuitive design with visual feedback  

## Credits
- ESP32 platform: Espressif Systems
- Libraries: ESPAsyncWebServer, ArduinoJson, ElegantOTA
- UI Framework: Custom CSS with Font Awesome icons
- Gauges: JustGage library
