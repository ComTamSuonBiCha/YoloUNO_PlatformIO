// // tinyml_sim_with_spike.cpp
// #include "tinyml.h"
// #include <Arduino.h>
// #include "global.h"
// // Control pins
// #define LED_GPIO   10    // onboard LED
// #define RELAY_GPIO 4     // relay 1

// // Global simulated sensor values (shared with other code if needed)
// // float glob_temperature = 25.0f;
// // float glob_humidity = 55.0f;

// // TFLite globals and arena
// namespace {
//   tflite::ErrorReporter *error_reporter = nullptr;
//   const tflite::Model *model = nullptr;
//   tflite::MicroInterpreter *interpreter = nullptr;
//   TfLiteTensor *input = nullptr;
//   TfLiteTensor *output = nullptr;
//   // Start with 32 KiB; increase if AllocateTensors() fails.
//   constexpr int kTensorArenaSize = 32 * 1024;
//   uint8_t tensor_arena[kTensorArenaSize];
// }

// // ---------------------- TinyML init ----------------------
// void setupTinyML() {
//   Serial.println("🧠 TinyML init...");
//   static tflite::MicroErrorReporter micro_error_reporter;
//   error_reporter = &micro_error_reporter;

//   model = tflite::GetModel(dht_anomaly_model_tflite);
//   if (model == nullptr) {
//     Serial.println("Model pointer is null!");
//     vTaskDelete(NULL);
//   }
//   if (model->version() != TFLITE_SCHEMA_VERSION) {
//     Serial.printf("Model schema mismatch: %d vs %d\n", model->version(), TFLITE_SCHEMA_VERSION);
//     vTaskDelete(NULL);
//   }

//   static tflite::AllOpsResolver resolver;
//   static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
//   interpreter = &static_interpreter;

//   TfLiteStatus allocate_status = interpreter->AllocateTensors();
//   if (allocate_status != kTfLiteOk) {
//     Serial.println("AllocateTensors() failed! Increase arena size.");
//     vTaskDelete(NULL);
//   }

//   input = interpreter->input(0);
//   output = interpreter->output(0);

//   pinMode(LED_GPIO, OUTPUT);
//   pinMode(RELAY_GPIO, OUTPUT);
//   digitalWrite(LED_GPIO, LOW);
//   digitalWrite(RELAY_GPIO, LOW);

//   // seed RNG a bit (if analog pin available)
//   randomSeed((unsigned long)micros());

//   Serial.println("TinyML ready.");
// }

// // ---------------------- Simulated sensor generator with spikes ----------------------
// // Behavior:
// // - By default generate temp in [20..30] and hum in [40..70].
// // - Every `spike_cycle_interval` iterations produce a temperature spike
// //   of magnitude spike_temp (for `spike_duration` iterations).
// void generateMockSensorDataWithSpikes() {
//   static int iter = 0;
//   static int spike_counter = 0;
//   // Every N iterations we'll trigger a spike period
//   const int spike_cycle_interval = 12; // e.g., every 12 loops (~60s if loop delay 5s)
//   const int spike_duration = 3;        // spike lasts for 3 loops (here ~15s)
//   const float normal_temp_min = 20.0f;
//   const float normal_temp_max = 30.0f;
//   const float normal_hum_min = 40.0f;
//   const float normal_hum_max = 70.0f;
//   const float spike_temp = 45.0f;      // temperature during anomaly

//   // increment iteration
//   iter++;

//   if (spike_counter > 0) {
//     // still in spike period
//     glob_temperature = spike_temp + (float)(random(-50, 50)) / 100.0f; // slight jitter
//     spike_counter--;
//   } else if ((iter % spike_cycle_interval) == 0) {
//     // start new spike
//     spike_counter = spike_duration - 1; // consume this iteration plus subsequent ones
//     glob_temperature = spike_temp + (float)(random(-50, 50)) / 100.0f;
//   } else {
//     // normal random values
//     glob_temperature = normal_temp_min + (float)random(0, (int)((normal_temp_max - normal_temp_min) * 100)) / 100.0f;
//   }

//   // humidity random independent
//   glob_humidity = normal_hum_min + (float)random(0, (int)((normal_hum_max - normal_hum_min) * 100)) / 100.0f;
// }

// // ---------------------- Main TinyML task ----------------------
// void tiny_ml_task(void *pvParameters) {
//   setupTinyML();

//   while (1) {
//     // generate mock data with occasional spikes
//     generateMockSensorDataWithSpikes();

//     // print the synthetic sensor values
//     Serial.printf("Temp=%.2f °C   Hum=%.2f %%\n", glob_temperature, glob_humidity);

//     // safety: check input tensor shape and type (best-effort)
//     if (input == nullptr || output == nullptr) {
//       Serial.println("Input/output tensors null");
//       vTaskDelay(pdMS_TO_TICKS(2000));
//       continue;
//     }

//     if (input->type == kTfLiteFloat32 && input->bytes >= sizeof(float) * 2) {
//       // assign floats
//       input->data.f[0] = glob_temperature;
//       input->data.f[1] = glob_humidity;
//     } else {
//       // If model is quantized or different shape, you'll need to quantize/reshape accordingly.
//       Serial.println("Unexpected input tensor type/size — adjust code for quantized model.");
//       vTaskDelay(pdMS_TO_TICKS(2000));
//       continue;
//     }

//     // invoke model
//     TfLiteStatus invoke_status = interpreter->Invoke();
//     if (invoke_status != kTfLiteOk) {
//       Serial.println("Invoke failed");
//       vTaskDelay(pdMS_TO_TICKS(2000));
//       continue;
//     }

//     // read model output (assumes float)
//     float result = 0.0f;
//     if (output->type == kTfLiteFloat32) {
//       result = output->data.f[0];
//       Serial.printf("Model output: %.3f\n", result);
//     } else {
//       Serial.println("Unexpected output type (not float).");
//       vTaskDelay(pdMS_TO_TICKS(2000));
//       continue;
//     }

//     // decision threshold (adjust if model uses different scale)
//     const float threshold = 0.5f;
//     if (result > threshold) {
//       digitalWrite(LED_GPIO, HIGH);
//       digitalWrite(RELAY_GPIO, HIGH);
//       Serial.println("Anomaly detected -> LED & Relay ON");
//     } else {
//       digitalWrite(LED_GPIO, LOW);
//       digitalWrite(RELAY_GPIO, LOW);
//       Serial.println("Normal -> LED & Relay OFF");
//     }

//     Serial.println("---------------------------------");
//     // wait before next loop
//     vTaskDelay(pdMS_TO_TICKS(5000));
//   }
// }
