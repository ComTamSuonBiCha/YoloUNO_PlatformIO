#include "tinyml.h"

// Global variables for TinyML (internal to this module)
namespace
{
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    constexpr int kTensorArenaSize = 8 * 1024; // 8KB tensor arena - adjust if needed
    uint8_t tensor_arena[kTensorArenaSize];
    
    // Accuracy evaluation metrics
    AccuracyMetrics_t accuracy_metrics = {0};
    
    // Note: Detailed TP/TN tracking is done in updateAccuracyMetrics
    // using static variables for proper accumulation
    
    // Data collection mode flag
    bool data_collection_enabled = false;
    unsigned long sample_count = 0;
} // namespace

/**
 * @brief Initialize TensorFlow Lite Micro on ESP32
 * 
 * This function sets up the TensorFlow Lite interpreter with the pre-trained
 * DHT anomaly detection model. The model takes temperature and humidity as
 * inputs and outputs an anomaly score.
 */
void setupTinyML()
{
    Serial.println("\n=== TensorFlow Lite Micro Initialization ===");
    
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // Load the model from the embedded array
    model = tflite::GetModel(dht_anomaly_model_tflite);
    
    // Verify model schema version
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        Serial.print("ERROR: Model schema version ");
        Serial.print(model->version());
        Serial.print(" does not match supported version ");
        Serial.println(TFLITE_SCHEMA_VERSION);
        error_reporter->Report("Model provided is schema version %d, not equal to supported version %d.",
                               model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    // Create resolver for all operations
    static tflite::AllOpsResolver resolver;
    
    // Create interpreter with tensor arena
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    // Allocate memory for tensors
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        Serial.println("ERROR: Failed to allocate tensors!");
        error_reporter->Report("AllocateTensors() failed");
        return;
    }

    // Get input and output tensor pointers
    input = interpreter->input(0);
    output = interpreter->output(0);

    // Print model information
    Serial.println("Model loaded successfully!");
    Serial.print("Input tensor size: ");
    Serial.println(input->bytes);
    Serial.print("Output tensor size: ");
    Serial.println(output->bytes);
    Serial.print("Tensor arena used: ");
    Serial.print(interpreter->arena_used_bytes());
    Serial.print(" / ");
    Serial.println(kTensorArenaSize);
    Serial.println("=== TinyML Setup Complete ===\n");
}



/**
 * @brief Run inference on sensor data
 * 
 * @param temperature Temperature value in Celsius
 * @param humidity Humidity value in percentage
 * @return InferenceResult_t Structure containing inference results
 */
InferenceResult_t runInference(float temperature, float humidity)
{
    InferenceResult_t result = {0};
    unsigned long start_time = micros();

    if (interpreter == nullptr || input == nullptr || output == nullptr)
    {
        Serial.println("ERROR: TinyML not initialized!");
        result.prediction_valid = false;
        return result;
    }

    // Prepare input data
    input->data.f[0] = temperature;
    input->data.f[1] = humidity;

    // Run inference
    TfLiteStatus invoke_status = interpreter->Invoke();
    
    unsigned long end_time = micros();
    result.inference_time = end_time - start_time;

    if (invoke_status != kTfLiteOk)
    {
        Serial.println("ERROR: Inference failed!");
        error_reporter->Report("Invoke failed");
        result.prediction_valid = false;
        return result;
    }

    // Get output (anomaly score: 0 = normal, 1 = anomaly)
    result.anomaly_score = output->data.f[0];
    
    // Calculate confidence (distance from threshold)
    float distance_from_threshold = fabs(result.anomaly_score - ANOMALY_THRESHOLD);
    result.confidence = distance_from_threshold * 2.0f; // Normalize to 0-1
    if (result.confidence > 1.0f) result.confidence = 1.0f;
    
    result.prediction_valid = true;
    
    return result;
}

/**
 * @brief Enable or disable data collection mode
 * 
 * In data collection mode, the system will save sensor readings with labels
 * for training dataset creation.
 * 
 * @param enable true to enable, false to disable
 */
void enableDataCollectionMode(bool enable)
{
    data_collection_enabled = enable;
    if (enable)
    {
        Serial.println("Data collection mode ENABLED");
        sample_count = 0;
    }
    else
    {
        Serial.println("Data collection mode DISABLED");
        Serial.print("Total samples collected: ");
        Serial.println(sample_count);
    }
}

/**
 * @brief Collect a sensor data sample for dataset creation
 * 
 * This function saves sensor readings with labels to a file for later use
 * in model training.
 * 
 * @param temp Temperature value
 * @param hum Humidity value
 * @param label Label: 0 = normal, 1 = anomaly
 * @param filename Output filename (default: /data/collected_data.csv)
 */
void collectSensorDataSample(float temp, float hum, int label, const char* filename)
{
    // Note: File system operations would need to be implemented
    // This is a placeholder structure for the data collection
    Serial.print("Collecting sample #");
    Serial.print(++sample_count);
    Serial.print(": Temp=");
    Serial.print(temp);
    Serial.print(", Hum=");
    Serial.print(hum);
    Serial.print(", Label=");
    Serial.println(label);
}

/**
 * @brief Reset accuracy evaluation metrics
 */
void resetAccuracyMetrics()
{
    accuracy_metrics.total_samples = 0;
    accuracy_metrics.correct_predictions = 0;
    accuracy_metrics.false_positives = 0;
    accuracy_metrics.false_negatives = 0;
    accuracy_metrics.accuracy = 0.0f;
    accuracy_metrics.precision = 0.0f;
    accuracy_metrics.recall = 0.0f;
    accuracy_metrics.f1_score = 0.0f;
    
    // Reset static counters in updateAccuracyMetrics
    // Note: This requires the static variables to be accessible - 
    // for full reset, we may need to modify the function
    
    Serial.println("Accuracy metrics reset.");
}

/**
 * @brief Update accuracy metrics with a prediction result
 * 
 * @param predicted_anomaly True if model predicted anomaly
 * @param actual_anomaly True if actual condition is anomaly (ground truth)
 */
void updateAccuracyMetrics(bool predicted_anomaly, bool actual_anomaly)
{
    accuracy_metrics.total_samples++;
    
    // Track detailed metrics
    static unsigned int true_positives = 0;
    static unsigned int true_negatives = 0;
    
    if (predicted_anomaly && actual_anomaly)
    {
        // True Positive: Predicted anomaly, actual anomaly
        true_positives++;
        accuracy_metrics.correct_predictions++;
    }
    else if (!predicted_anomaly && !actual_anomaly)
    {
        // True Negative: Predicted normal, actual normal
        true_negatives++;
        accuracy_metrics.correct_predictions++;
    }
    else if (predicted_anomaly && !actual_anomaly)
    {
        // False Positive: Predicted anomaly, actual normal
        accuracy_metrics.false_positives++;
    }
    else // !predicted_anomaly && actual_anomaly
    {
        // False Negative: Predicted normal, actual anomaly
        accuracy_metrics.false_negatives++;
    }
    
    // Calculate metrics
    if (accuracy_metrics.total_samples > 0)
    {
        // Accuracy: (TP + TN) / Total
        accuracy_metrics.accuracy = (float)accuracy_metrics.correct_predictions / 
                                   (float)accuracy_metrics.total_samples;
        
        // Precision: TP / (TP + FP)
        unsigned int predicted_positives = true_positives + accuracy_metrics.false_positives;
        if (predicted_positives > 0)
        {
            accuracy_metrics.precision = (float)true_positives / (float)predicted_positives;
        }
        
        // Recall: TP / (TP + FN)
        unsigned int actual_positives = true_positives + accuracy_metrics.false_negatives;
        if (actual_positives > 0)
        {
            accuracy_metrics.recall = (float)true_positives / (float)actual_positives;
        }
        
        // F1 Score: 2 * (Precision * Recall) / (Precision + Recall)
        if (accuracy_metrics.precision + accuracy_metrics.recall > 0)
        {
            accuracy_metrics.f1_score = 2.0f * (accuracy_metrics.precision * accuracy_metrics.recall) /
                                       (accuracy_metrics.precision + accuracy_metrics.recall);
        }
    }
}

/**
 * @brief Print accuracy metrics to Serial
 */
void printAccuracyMetrics()
{
    Serial.println("\n=== TinyML Accuracy Metrics ===");
    Serial.print("Total samples: ");
    Serial.println(accuracy_metrics.total_samples);
    Serial.print("Correct predictions: ");
    Serial.println(accuracy_metrics.correct_predictions);
    Serial.print("False positives: ");
    Serial.println(accuracy_metrics.false_positives);
    Serial.print("False negatives: ");
    Serial.println(accuracy_metrics.false_negatives);
    Serial.print("Accuracy: ");
    Serial.print(accuracy_metrics.accuracy * 100.0f);
    Serial.println("%");
    Serial.print("Precision: ");
    Serial.print(accuracy_metrics.precision * 100.0f);
    Serial.println("%");
    Serial.print("Recall: ");
    Serial.print(accuracy_metrics.recall * 100.0f);
    Serial.println("%");
    Serial.print("F1 Score: ");
    Serial.print(accuracy_metrics.f1_score * 100.0f);
    Serial.println("%");
    Serial.println("==============================\n");
}

/**
 * @brief Get current accuracy metrics
 * 
 * @return AccuracyMetrics_t Structure containing all accuracy metrics
 */
AccuracyMetrics_t getAccuracyMetrics()
{
    return accuracy_metrics;
}

/**
 * @brief TinyML Task - Main RTOS task for running inference
 * 
 * This task:
 * 1. Initializes TensorFlow Lite Micro
 * 2. Continuously reads sensor data via queue
 * 3. Runs inference on the data
 * 4. Evaluates and logs results
 * 5. Optionally collects data for training dataset
 */
void tiny_ml_task(void *pvParameters)
{
    AppContext_t *ctx = (AppContext_t *) pvParameters;
    Serial.println("TinyML Task Started");

    // Initialize TensorFlow Lite Micro
    setupTinyML();
    
    // Reset accuracy metrics
    resetAccuracyMetrics();

    enableDataCollectionMode(true);
    
    // Initialize sensor data structure
    SensorSample_t sensor_data;
    InferenceResult_t inference_result;
    
    // Counter for periodic accuracy reporting
    unsigned int inference_count = 0;

    while (1)
    {
        // Get sensor data from dedicated TinyML queue (with timeout)
        if (xQueueReceive(ctx->tinymlQueue, &sensor_data, pdMS_TO_TICKS(10000)) == pdTRUE)
        {
            // Print inference timestamp
            Serial.print("[TinyML] Inference Timestamp: ");
            Serial.print(sensor_data.timestamp);
            Serial.println(" ms");

            // Validate sensor data
            if (!isnan(sensor_data.temperature) && !isnan(sensor_data.humidity)) {
                // Run inference
                inference_result = runInference(sensor_data.temperature, sensor_data.humidity);
                
                if (inference_result.prediction_valid)
                {
                    // Determine if anomaly detected
                    bool is_anomaly = (inference_result.anomaly_score >= ANOMALY_THRESHOLD);
                    
                    // Print inference results
                    Serial.print("[TinyML] Temp: ");
                    Serial.print(sensor_data.temperature, 1);
                    Serial.print("°C, Hum: ");
                    Serial.print(sensor_data.humidity, 1);
                    Serial.print("%, Anomaly Score: ");
                    Serial.print(inference_result.anomaly_score, 4);
                    Serial.print(", Prediction: ");
                    Serial.print(is_anomaly ? "ANOMALY" : "NORMAL");
                    Serial.print(", Confidence: ");
                    Serial.print(inference_result.confidence * 100.0f, 1);
                    Serial.print("%, Inference Time: ");
                    Serial.print(inference_result.inference_time);
                    Serial.println(" μs");
                    
                    // Data collection mode
                    // if (data_collection_enabled)
                    // {

                    //     collectSensorDataSample(sensor_data.temperature, 
                    //                            sensor_data.humidity, 
                    //                            is_anomaly ? 1 : 0);
                    // }
                    
    // Update accuracy metrics
    // For real accuracy evaluation need ground truth labels
    // For demonstration, assume all predictions are correct
    // In real evaluation, we need to provide ground truth labels
    // Example: updateAccuracyMetrics(is_anomaly, actual_ground_truth_label);
    
    // Uncomment below when have ground truth labels for evaluation:
    // bool ground_truth = determineGroundTruth(sensor_data.temperature, sensor_data.humidity);
    // updateAccuracyMetrics(is_anomaly, ground_truth);
                    
                    inference_count++;
                    
                    // Print accuracy metrics every 50 inferences
                    if (inference_count % 50 == 0)
                    {
                        printAccuracyMetrics();
                    }
                }
                else
                {
                    Serial.println("[TinyML] ERROR: Inference failed!");
                }
            }
            else
            {
                Serial.println("[TinyML] WARNING: Invalid sensor data (NaN values)");
            }
        }
        else
        {
            Serial.println("[TinyML] WARNING: Failed to receive sensor data from queue (timeout)");
        }
        
        // No additional delay needed - queue receive provides timing
    }
}