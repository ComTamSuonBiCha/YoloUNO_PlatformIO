#include "global.h"

// DEPRECATED: These global variables are kept for backward compatibility 
// with web interface (Webserver_sendata) but should NOT be used by CoreIOT task.
// CoreIOT now uses queue-based telemetry (TelemetryData_t via telemetryQueue)
float glob_temperature = 0;
float glob_humidity = 0;

String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

String ssid = "ESP32-YOUR NETWORK HERE!!!";
String password = "12345678";
String wifi_ssid = "abcde";
String wifi_password = "123456789";
boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();