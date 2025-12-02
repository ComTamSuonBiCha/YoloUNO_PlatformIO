#ifndef __TINY_ML__
#define __TINY_ML__

#include <Arduino.h>

#include "dht_anomaly_model.h"
#include "global.h"

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Semaphore for sensor data communication (replacing global variables)
extern SemaphoreHandle_t xSensorDataSemaphore;

// Structure to hold sensor data
typedef struct {
    float temperature;
    float humidity;
    bool data_valid;
} SensorData_t;

// Structure to hold inference results and accuracy metrics
typedef struct {
    float anomaly_score;          // Model output (0 = normal, 1 = anomaly)
    float confidence;             // Confidence level of prediction
    unsigned long inference_time; // Inference time in microseconds
    bool prediction_valid;        // Whether prediction is valid
} InferenceResult_t;

// TinyML accuracy evaluation structure
typedef struct {
    unsigned int total_samples;
    unsigned int correct_predictions;
    unsigned int false_positives;
    unsigned int false_negatives;
    float accuracy;
    float precision;
    float recall;
    float f1_score;
} AccuracyMetrics_t;

// Function declarations
void setupTinyML();
void tiny_ml_task(void *pvParameters);

// Data collection functions for dataset creation
void enableDataCollectionMode(bool enable);
void collectSensorDataSample(float temp, float hum, int label, const char* filename = "/data/collected_data.csv");

// Accuracy evaluation functions
void resetAccuracyMetrics();
void updateAccuracyMetrics(bool predicted_anomaly, bool actual_anomaly);
void printAccuracyMetrics();
AccuracyMetrics_t getAccuracyMetrics();

// Helper functions
bool getSensorDataViaSemaphore(SensorData_t* sensor_data, TickType_t timeout_ms = 1000);
InferenceResult_t runInference(float temperature, float humidity);

// Threshold for anomaly detection (adjust based on your model)
#define ANOMALY_THRESHOLD 0.5f

#endif