#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
// #include "mainserver.h"
#include "tinyml.h"  // Enable TinyML task for Task 5
#include "coreiot.h"

// include task
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task_core_iot.h"
#include "task_manager.h"
#include "task_lcd.h"
void setup()
{
  Serial.begin(115200);
  check_info_File(0);

  // Start WiFi in Access Point mode for Task 4
  Serial.println("🚀 Starting WiFi in Access Point mode...");
  startAP();
  Serial.print("📡 AP IP Address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("📱 Connect to the AP and navigate to the IP address above");

  // AppContext as a static local (not a global variable in file scope)
  static AppContext_t appCtx;

  // Create queues
  appCtx.sensorQueue = xQueueCreate(10, sizeof(SensorSample_t));
  appCtx.ledQueue    = xQueueCreate(5,  sizeof(LedPattern_t));
  appCtx.neoQueue    = xQueueCreate(5,  sizeof(NeoColor_t));
  appCtx.tinymlQueue = xQueueCreate(10, sizeof(SensorSample_t)); // Queue for TinyML task

  // Create semaphores
  appCtx.stateSemaphore = xSemaphoreCreateBinary();
  appCtx.dataMutex      = xSemaphoreCreateMutex();

  // Initialize shared state
  appCtx.currentState   = STATE_NORMAL;
  appCtx.lastTemperature = 0.0f;
  appCtx.lastHumidity    = 0.0f;  
  
  // Task 1, 2, 3 tasks
  xTaskCreate(led_blinky, "Task LED Blink", 2048, (void*)&appCtx, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, (void*)&appCtx, 2, NULL);
  xTaskCreate(manager_task, "Task Manager", 4096, (void*)&appCtx, 3, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, (void*)&appCtx, 3, NULL);
  xTaskCreate(lcd_task, "Task LCD", 4096, (void*)&appCtx, 2, NULL);
  
  // Task 4: Web Server in Access Point Mode
  xTaskCreate(webserver_task, "Task Web Server", 8192, NULL, 2, NULL);
  
  // WiFi Station Task - Connects to WiFi if credentials are available
  xTaskCreate(wifi_sta_task, "Task WiFi STA", 4096, NULL, 3, NULL);
  
  // Task 5: TinyML Task - Enable for anomaly detection
  xTaskCreate(tiny_ml_task, "Task TinyML", 8192, (void*)&appCtx, 2, NULL);
  
  // Task 6: CoreIOT Cloud Server - Data Publishing and RPC Control
  xTaskCreate(coreiot_task, "CoreIOT Task", 4096, NULL, 2, NULL);
  
  Serial.println("✅ All tasks created successfully");
  // xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
  // xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
}

void loop()
{
  // Empty loop - everything handled by FreeRTOS tasks
}
