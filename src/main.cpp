#include "global.h"

#include "led_blinky.h"
#include "temp_humi_monitor.h"
// #include "tinyml.h"
#include "coreiot.h"
#include "relay_toggle_task.h"
#include "task_check_info.h"
#include "task_core_iot.h"
#include "task_webserver.h"
// void setup() {
//   Serial.begin(115200);

//   // delay(1000);

//   // if (!LittleFS.begin(true)) {
//   //   Serial.println("❌ LittleFS mount failed!");
//   // } else {
//   //   Serial.println("✅ LittleFS mounted!");
//   // }
//   // startAP(); // Phát WiFi ESP32 setup
//   check_info_File(0);
//   xTaskCreate(led_blinky, "Task LED Blink" ,2048  ,NULL  ,2 , NULL);
//   // xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor" ,2048  ,NULL  ,2 , NULL);
//   // xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
//   // xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
//   // xTaskCreate(tiny_ml_task, "Tiny ML Task", 16384, NULL, 2, NULL);
//   xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
//   xTaskCreate(relay_toggle_task, "Relay Toggle", 4096, NULL, 2, NULL);
// }

void setup() {
  Serial.begin(115200);
  delay(1000);



  if (!LittleFS.begin(true)) {
    Serial.println("❌ LittleFS mount failed!");
  } else {
    Serial.println("✅ LittleFS mounted!");
    // create a default file on first boot to avoid VFS error
    if (!LittleFS.exists("/info.dat")) {
      File f = LittleFS.open("/info.dat", "w");
      if (f) { f.print("{}"); f.close(); }
    }
  }

  startAP();
  xTaskCreate(led_blinky, "Task LED Blink" ,2048 ,NULL ,2 , NULL);
  xTaskCreate(coreiot_task, "CoreIOT Task" ,4096 ,NULL ,2 , NULL);
  // xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  // xTaskCreate(relay_toggle_task, "Relay Toggle", 4096, NULL, 2, NULL);
}


void loop() {
  if (check_info_File(1))
  {
    if (!Wifi_reconnect())
    {
      Webserver_stop();
    }
    else
    {
      CORE_IOT_reconnect();
    }
  }
  Webserver_reconnect();
}