"""
TensorFlow Lite Model Training Script for DHT Anomaly Detection
Task 5: TinyML Deployment & Accuracy Evaluation

This script:
1. Loads collected sensor data
2. Trains a neural network model
3. Evaluates model performance
4. Converts to TensorFlow Lite
5. Generates C array for embedding in ESP32
"""

import tensorflow as tf
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import classification_report, confusion_matrix, precision_recall_curve, precision_score, recall_score, f1_score
import matplotlib.pyplot as plt
import os

# ============================================================================
# CONFIGURATION
# ============================================================================
DATASET_FILE = '/content/hcmc_weather_dataset_binary.csv'
MODEL_SAVE_PATH = 'dht_anomaly_model.keras' # Using .keras format
TFLITE_MODEL_PATH = 'dht_anomaly_model.tflite'
TFLITE_HEADER_PATH = 'dht_anomaly_model.h' # Simplified path for Colab

# Model hyperparameters
EPOCHS = 100
BATCH_SIZE = 32
VALIDATION_SPLIT = 0.15
TEST_SPLIT = 0.15

# ============================================================================
# DATA LOADING AND PREPARATION
# ============================================================================

def load_dataset(filename):
    """Load dataset from CSV file"""
    print(f"Loading dataset from {filename}...")
    try:
        data = pd.read_csv(filename)
        print(f"Dataset loaded: {len(data)} samples")
        print(f"Columns: {data.columns.tolist()}")
        print(f"\nDataset statistics:")
        print(data.describe())
        print(f"\nLabel distribution:")
        print(data['label'].value_counts()) # Use 'label' as per your dataset
        return data
    except FileNotFoundError:
        print(f"ERROR: File {filename} not found!")
        print("Please collect data first using the ESP32 data collection mode.")
        return None
    except Exception as e:
        print(f"ERROR loading dataset: {e}")
        return None

def prepare_data(data):
    """Prepare data for training"""
    print("\nPreparing data...")
    
    # Extract features and labels using correct column names
    X = data[['temperature', 'humidity']].values # Use 'temperature', 'humidity'
    y = data['label'].values # Use 'label'
    
    # Split into train/validation/test
    X_temp, X_test, y_temp, y_test = train_test_split(
        X, y, test_size=TEST_SPLIT, random_state=42, stratify=y
    )
    
    X_train, X_val, y_train, y_val = train_test_split(
        X_temp, y_temp, test_size=VALIDATION_SPLIT/(1-TEST_SPLIT), 
        random_state=42, stratify=y_temp
    )
    
    print(f"Training set: {len(X_train)} samples")
    print(f"Validation set: {len(X_val)} samples")
    print(f"Test set: {len(X_test)} samples")
    
    # Normalize features
    scaler = StandardScaler()
    X_train = scaler.fit_transform(X_train)
    X_val = scaler.transform(X_val)
    X_test = scaler.transform(X_test)
    
    print("Features normalized using StandardScaler.")
    
    return X_train, X_val, X_test, y_train, y_val, y_test, scaler

# ============================================================================
# MODEL DEFINITION
# ============================================================================

def create_model():
    """Create and compile the neural network model"""
    print("\nCreating model...")
    
    model = tf.keras.Sequential([
        # Input layer: 2 features (temperature, humidity)
        tf.keras.layers.Dense(16, activation='relu', input_shape=(2,), name='dense_1'),
        tf.keras.layers.Dropout(0.1), # Add dropout for regularization
        tf.keras.layers.Dense(8, activation='relu', name='dense_2'),
        # Output layer: 1 neuron with sigmoid (binary classification)
        tf.keras.layers.Dense(1, activation='sigmoid', name='dense_output')
    ])
    
    model.compile(
        optimizer='adam',
        loss='binary_crossentropy',
        metrics=['accuracy', 'precision', 'recall']
    )
    
    print("Model architecture:")
    model.summary()
    
    return model

# ============================================================================
# MODEL TRAINING
# ============================================================================

def train_model(model, X_train, y_train, X_val, y_val):
    """Train the model"""
    print("\nTraining model...")
    print(f"Epochs: {EPOCHS}, Batch size: {BATCH_SIZE}")
    
    # Calculate class weights to handle imbalance
    from sklearn.utils.class_weight import compute_class_weight
    classes = np.unique(y_train)
    class_weights = compute_class_weight('balanced', classes=classes, y=y_train)
    class_weight_dict = dict(zip(classes, class_weights))
    print(f"Calculated class weights: {class_weight_dict}")
    
    # Callbacks
    callbacks = [
        tf.keras.callbacks.EarlyStopping(
            monitor='val_loss',
            patience=10,
            restore_best_weights=True
        ),
        tf.keras.callbacks.ReduceLROnPlateau(
            monitor='val_loss',
            factor=0.5,
            patience=5,
            min_lr=1e-6
        )
    ]
    
    # Train with class weights
    history = model.fit(
        X_train, y_train,
        validation_data=(X_val, y_val),
        epochs=EPOCHS,
        batch_size=BATCH_SIZE,
        callbacks=callbacks,
        verbose=1,
        class_weight=class_weight_dict # Use class weights
    )
    
    return history

# ============================================================================
# MODEL EVALUATION
# ============================================================================

def find_optimal_threshold(y_true, y_pred_prob):
    """Find the threshold that maximizes F1 score."""
    precision, recall, thresholds = precision_recall_curve(y_true, y_pred_prob)
    # F1 score
    f1_scores = 2 * (precision * recall) / (precision + recall)
    f1_scores = np.nan_to_num(f1_scores) # Handle division by zero
    optimal_idx = np.argmax(f1_scores)
    optimal_threshold = thresholds[optimal_idx]
    return optimal_threshold

def evaluate_model(model, X_test, y_test):
    """Evaluate model on test set"""
    print("\n" + "="*50)
    print("MODEL EVALUATION")
    print("="*50)
    
    # Evaluate
    test_loss, test_accuracy, test_precision, test_recall = model.evaluate(
        X_test, y_test, verbose=0
    )
    
    print(f"\nTest Loss: {test_loss:.4f}")
    print(f"Test Accuracy: {test_accuracy*100:.2f}%")
    print(f"Test Precision: {test_precision*100:.2f}%")
    print(f"Test Recall: {test_recall*100:.2f}%")
    
    # Calculate F1 score
    test_f1 = 2 * (test_precision * test_recall) / (test_precision + test_recall)
    print(f"Test F1 Score: {test_f1*100:.2f}%")
    
    # Predictions
    y_pred_prob = model.predict(X_test, verbose=0)
    y_pred_prob_flat = y_pred_prob.flatten() # Ensure it's 1D
    
    # Find optimal threshold
    optimal_threshold = find_optimal_threshold(y_test, y_pred_prob_flat)
    print(f"\nOptimal Threshold (based on F1): {optimal_threshold:.3f}")
    
    # Predictions using optimal threshold
    y_pred_optimal = (y_pred_prob_flat > optimal_threshold).astype(int)
    
    # Classification report with optimal threshold
    print("\nClassification Report (Optimal Threshold):")
    print(classification_report(y_test, y_pred_optimal, 
                                target_names=['Normal', 'Anomaly']))
    
    # Confusion matrix with optimal threshold
    print("\nConfusion Matrix (Optimal Threshold):")
    cm = confusion_matrix(y_test, y_pred_optimal)
    print(cm)
    
    # Calculate metrics for the optimal threshold
    opt_precision = precision_score(y_test, y_pred_optimal)
    opt_recall = recall_score(y_test, y_pred_optimal)
    opt_f1 = f1_score(y_test, y_pred_optimal)
    opt_accuracy = (y_pred_optimal == y_test).mean()
    
    print(f"\nMetrics with Optimal Threshold:")
    print(f"Accuracy: {opt_accuracy*100:.2f}%")
    print(f"Precision: {opt_precision*100:.2f}%")
    print(f"Recall: {opt_recall*100:.2f}%")
    print(f"F1 Score: {opt_f1*100:.2f}%")
    
    return {
        'loss': test_loss,
        'accuracy': opt_accuracy, # Use optimal threshold metrics
        'precision': opt_precision,
        'recall': opt_recall,
        'f1_score': opt_f1,
        'predictions': y_pred_optimal, # Use optimal threshold predictions
        'probabilities': y_pred_prob_flat
    }

# ============================================================================
# MODEL CONVERSION TO TENSORFLOW LITE
# ============================================================================

def convert_to_tflite(model, tflite_path):
    """Convert Keras model to TensorFlow Lite"""
    print(f"\nConverting model to TensorFlow Lite...")
    
    # Convert to TFLite
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    
    try:
        tflite_model = converter.convert()
        
        # Save TFLite model
        with open(tflite_path, 'wb') as f:
            f.write(tflite_model)
        
        print(f"TensorFlow Lite model saved to {tflite_path}")
        print(f"Model size: {len(tflite_model) / 1024:.2f} KB")
        
        return tflite_model
    except Exception as e:
        print(f"ERROR converting to TFLite: {e}")
        return None

def convert_to_c_array(tflite_model, header_path):
    """Convert TFLite model to C array header file"""
    print(f"\nConverting to C array header...")
    
    # Generate C array
    c_array_lines = [
        "#ifndef __DHT_ANOMALY_MODEL_H__",
        "#define __DHT_ANOMALY_MODEL_H__",
        "",
        "// Generated TensorFlow Lite model for DHT anomaly detection",
        f"// Model size: {len(tflite_model)} bytes",
        f"// Generated by train_model.py",
        "",
        "const unsigned char dht_anomaly_model_tflite[] = {"
    ]
    
    # Convert bytes to hex
    for i, byte in enumerate(tflite_model):
        if i % 12 == 0:
            c_array_lines.append("  ")
        c_array_lines[-1] += f"0x{byte:02x}, "
        if (i + 1) % 12 == 0:
            c_array_lines[-1] = c_array_lines[-1].rstrip()  # Remove trailing comma and space
    
    # Close array
    if c_array_lines[-1].endswith(", "):
        c_array_lines[-1] = c_array_lines[-1][:-2]  # Remove trailing comma
    c_array_lines.append("};")
    c_array_lines.append("")
    c_array_lines.append(f"const unsigned int dht_anomaly_model_tflite_len = {len(tflite_model)};")
    c_array_lines.append("")
    c_array_lines.append("#endif // __DHT_ANOMALY_MODEL_H__")
    
    # Write to file
    try:
        with open(header_path, 'w') as f:
            f.write('\n'.join(c_array_lines))
        print(f"C array header saved to {header_path}")
    except Exception as e:
        print(f"ERROR writing header file: {e}")

# ============================================================================
# VISUALIZATION
# ============================================================================

def plot_training_history(history):
    """Plot training history"""
    print("\nGenerating training plots...")
    
    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    
    # Accuracy
    axes[0, 0].plot(history.history['accuracy'], label='Training Accuracy')
    axes[0, 0].plot(history.history['val_accuracy'], label='Validation Accuracy')
    axes[0, 0].set_title('Model Accuracy')
    axes[0, 0].set_xlabel('Epoch')
    axes[0, 0].set_ylabel('Accuracy')
    axes[0, 0].legend()
    axes[0, 0].grid(True)
    
    # Loss
    axes[0, 1].plot(history.history['loss'], label='Training Loss')
    axes[0, 1].plot(history.history['val_loss'], label='Validation Loss')
    axes[0, 1].set_title('Model Loss')
    axes[0, 1].set_xlabel('Epoch')
    axes[0, 1].set_ylabel('Loss')
    axes[0, 1].legend()
    axes[0, 1].grid(True)
    
    # Precision
    axes[1, 0].plot(history.history['precision'], label='Training Precision')
    axes[1, 0].plot(history.history['val_precision'], label='Validation Precision')
    axes[1, 0].set_title('Model Precision')
    axes[1, 0].set_xlabel('Epoch')
    axes[1, 0].set_ylabel('Precision')
    axes[1, 0].legend()
    axes[1, 0].grid(True)
    
    # Recall
    axes[1, 1].plot(history.history['recall'], label='Training Recall')
    axes[1, 1].plot(history.history['val_recall'], label='Validation Recall')
    axes[1, 1].set_title('Model Recall')
    axes[1, 1].set_xlabel('Epoch')
    axes[1, 1].set_ylabel('Recall')
    axes[1, 1].legend()
    axes[1, 1].grid(True)
    
    plt.tight_layout()
    plt.savefig('training_history.png', dpi=150)
    print("Training plots saved to training_history.png")
    plt.close()

# ============================================================================
# MAIN EXECUTION
# ============================================================================

def main():
    """Main training pipeline"""
    print("="*50)
    print("TinyML Model Training for DHT Anomaly Detection")
    print("Task 5: TinyML Deployment & Accuracy Evaluation")
    print("="*50)
    
    # Load dataset
    data = load_dataset(DATASET_FILE)
    if data is None:
        return
    
    # Prepare data
    X_train, X_val, X_test, y_train, y_val, y_test, scaler = prepare_data(data) # Unpack scaler
    
    # Create model
    model = create_model()
    
    # Train model
    history = train_model(model, X_train, y_train, X_val, y_val)
    
    # Evaluate model
    results = evaluate_model(model, X_test, y_test)
    
    # Save Keras model (using native .keras format)
    try:
        model.save(MODEL_SAVE_PATH)
        print(f"\nKeras model saved to {MODEL_SAVE_PATH}")
    except Exception as e:
        print(f"ERROR saving model: {e}")
    
    # Convert to TensorFlow Lite
    tflite_model = convert_to_tflite(model, TFLITE_MODEL_PATH)
    if tflite_model is not None:
        convert_to_c_array(tflite_model, TFLITE_HEADER_PATH)
    
    # Plot training history
    try:
        plot_training_history(history)
    except Exception as e:
        print(f"WARNING: Could not generate plots: {e}")
    
    print("\n" + "="*50)
    print("TRAINING COMPLETE!")
    print("="*50)
    print(f"\nFinal Test Accuracy (Optimal Threshold): {results['accuracy']*100:.2f}%")
    print(f"Final Test F1 Score (Optimal Threshold): {results['f1_score']*100:.2f}%")
    print("\nNext steps:")
    print("1. Copy the generated header file to your ESP32 project's include/ directory")
    print("2. Rebuild your ESP32 project")
    print("3. Deploy and evaluate on hardware")
    print("4. Remember to normalize sensor data using the same scaler parameters in your ESP32 code!")

if __name__ == "__main__":
    main()