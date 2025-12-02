#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
// #include "mainserver.h"
// #include "tinyml.h"
#include "coreiot.h"

// include task
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task_core_iot.h"
#include "task_manager.h"
void setup()
{
  Serial.begin(115200);
  check_info_File(0);

  // AppContext as a static local (not a global variable in file scope)
  static AppContext_t appCtx;

  // Create queues
  appCtx.sensorQueue = xQueueCreate(10, sizeof(SensorSample_t));
  appCtx.ledQueue    = xQueueCreate(5,  sizeof(LedPattern_t));
  appCtx.neoQueue    = xQueueCreate(5,  sizeof(NeoColor_t));

  // Create semaphores
  appCtx.stateSemaphore = xSemaphoreCreateBinary();
  appCtx.dataMutex      = xSemaphoreCreateMutex();

  // Initialize shared state
  appCtx.currentState   = STATE_NORMAL;
  appCtx.lastTemperature = 0.0f;
  appCtx.lastHumidity    = 0.0f;  
  xTaskCreate(led_blinky, "Task LED Blink", 2048, (void*)&appCtx, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, (void*)&appCtx, 2, NULL);
  xTaskCreate(manager_task, "Task Manager", 4096, (void*)&appCtx, 3, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, (void*)&appCtx, 3, NULL);
  // xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
  // xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  // xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
  // xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
}

void loop()
{
  if (check_info_File(1))
  {
    if (!Wifi_reconnect())
    {
      Webserver_stop();
    }
    else
    {
      //CORE_IOT_reconnect();
    }
  }
  Webserver_reconnect();
}