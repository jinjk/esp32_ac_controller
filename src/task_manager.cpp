#include "task_manager.h"
#include "config.h"
#include "ir_control.h"
#include "sensor.h"
#include <ArduinoJson.h>

// Global task manager instance
TaskManager taskManager;

// Constructor
TaskManager::TaskManager() {
    // Initialize task info structures
    irLearningTaskInfo.handle = nullptr;
    irLearningTaskInfo.state = TASK_STOPPED;
    irLearningTaskInfo.startTime = 0;
    irLearningTaskInfo.endTime = 0;
    irLearningTaskInfo.name = "IR Learning";
    
    calibrationTaskInfo.handle = nullptr;
    calibrationTaskInfo.state = TASK_STOPPED;
    calibrationTaskInfo.startTime = 0;
    calibrationTaskInfo.endTime = 0;
    calibrationTaskInfo.name = "Calibration";
}

// IR Learning Task Functions
bool TaskManager::startIRLearningTask() {
    if (irLearningTaskInfo.state != TASK_STOPPED) {
        return false; // Task already running or starting
    }
    
    irLearningTaskInfo.state = TASK_STARTING;
    irLearningTaskInfo.startTime = millis();
    
    BaseType_t result = xTaskCreate(
        irLearningTaskWrapper,
        "ir_learning_task",
        4096,
        this,
        1,
        &irLearningTaskInfo.handle
    );
    
    if (result == pdPASS) {
        irLearningTaskInfo.state = TASK_RUNNING;
        return true;
    } else {
        irLearningTaskInfo.state = TASK_STOPPED;
        irLearningTaskInfo.handle = nullptr;
        return false;
    }
}

bool TaskManager::stopIRLearningTask() {
    if (irLearningTaskInfo.state == TASK_STOPPED) {
        return true; // Already stopped
    }
    
    irLearningTaskInfo.state = TASK_STOPPING;
    
    if (irLearningTaskInfo.handle != nullptr) {
        vTaskDelete(irLearningTaskInfo.handle);
        irLearningTaskInfo.handle = nullptr;
    }
    
    irLearningTaskInfo.state = TASK_STOPPED;
    irLearningTaskInfo.endTime = millis();
    return true;
}

TaskState TaskManager::getIRLearningState() {
    return irLearningTaskInfo.state;
}

// Calibration Task Functions
bool TaskManager::startCalibrationTask() {
    if (calibrationTaskInfo.state != TASK_STOPPED) {
        return false; // Task already running or starting
    }
    
    calibrationTaskInfo.state = TASK_STARTING;
    calibrationTaskInfo.startTime = millis();
    
    BaseType_t result = xTaskCreate(
        calibrationTaskWrapper,
        "calibration_task",
        4096,
        this,
        1,
        &calibrationTaskInfo.handle
    );
    
    if (result == pdPASS) {
        calibrationTaskInfo.state = TASK_RUNNING;
        return true;
    } else {
        calibrationTaskInfo.state = TASK_STOPPED;
        calibrationTaskInfo.handle = nullptr;
        return false;
    }
}

bool TaskManager::stopCalibrationTask() {
    if (calibrationTaskInfo.state == TASK_STOPPED) {
        return true; // Already stopped
    }
    
    calibrationTaskInfo.state = TASK_STOPPING;
    
    if (calibrationTaskInfo.handle != nullptr) {
        vTaskDelete(calibrationTaskInfo.handle);
        calibrationTaskInfo.handle = nullptr;
    }
    
    calibrationTaskInfo.state = TASK_STOPPED;
    calibrationTaskInfo.endTime = millis();
    return true;
}

TaskState TaskManager::getCalibrationState() {
    return calibrationTaskInfo.state;
}

// General Management Functions
String TaskManager::getTaskStatus() {
    JsonDocument doc;
    
    // IR Learning task status
    JsonObject irTask = doc["ir_learning"].to<JsonObject>();
    irTask["state"] = getStateString(irLearningTaskInfo.state);
    irTask["start_time"] = irLearningTaskInfo.startTime;
    irTask["end_time"] = irLearningTaskInfo.endTime;
    irTask["name"] = irLearningTaskInfo.name;
    irTask["ready_for_control"] = isIRReadyForControl();
    irTask["learned_buttons"] = learningState.learnedButtons;
    irTask["total_buttons"] = learningState.totalButtons;
    
    // Calibration task status
    JsonObject calTask = doc["calibration"].to<JsonObject>();
    calTask["state"] = getStateString(calibrationTaskInfo.state);
    calTask["start_time"] = calibrationTaskInfo.startTime;
    calTask["end_time"] = calibrationTaskInfo.endTime;
    calTask["name"] = calibrationTaskInfo.name;
    
    String result;
    serializeJson(doc, result);
    return result;
}

void TaskManager::cleanupFinishedTasks() {
    // Clean up any finished tasks
    if (irLearningTaskInfo.state == TASK_STOPPED && irLearningTaskInfo.handle != nullptr) {
        irLearningTaskInfo.handle = nullptr;
    }
    
    if (calibrationTaskInfo.state == TASK_STOPPED && calibrationTaskInfo.handle != nullptr) {
        calibrationTaskInfo.handle = nullptr;
    }
}

bool TaskManager::isAnyTaskRunning() {
    return (irLearningTaskInfo.state == TASK_RUNNING || 
            irLearningTaskInfo.state == TASK_STARTING ||
            calibrationTaskInfo.state == TASK_RUNNING || 
            calibrationTaskInfo.state == TASK_STARTING);
}

String TaskManager::getStateString(TaskState state) {
    switch (state) {
        case TASK_STOPPED: return "stopped";
        case TASK_STARTING: return "starting";
        case TASK_RUNNING: return "running";
        case TASK_STOPPING: return "stopping";
        default: return "unknown";
    }
}

// Static wrapper functions for FreeRTOS tasks
void TaskManager::irLearningTaskWrapper(void* parameter) {
    TaskManager* manager = static_cast<TaskManager*>(parameter);
    manager->irLearningTask();
}

void TaskManager::calibrationTaskWrapper(void* parameter) {
    TaskManager* manager = static_cast<TaskManager*>(parameter);
    manager->calibrationTask();
}

// Task implementation functions
void TaskManager::irLearningTask() {
    // IR learning task implementation
    Serial.println("IR Learning task started");
    
    // Start the guided learning sequence
    startIRLearning();
    
    while (irLearningTaskInfo.state == TASK_RUNNING) {
        if (learningState.isLearning) {
            // Process IR learning operations
            processIRLearning();
            vTaskDelay(pdMS_TO_TICKS(100)); // Fast polling for IR signals
        } else {
            // Learning completed, exit the task
            Serial.println("IR Learning sequence completed");
            break;
        }
    }
    
    // Ensure learning is stopped if task was stopped externally
    if (learningState.isLearning) {
        stopIRLearning();
    }
    
    Serial.println("IR Learning task finished");
    irLearningTaskInfo.state = TASK_STOPPED;
    vTaskDelete(nullptr);
}

void TaskManager::calibrationTask() {
    // Calibration task implementation
    Serial.println("Calibration task started");
    
    while (calibrationTaskInfo.state == TASK_RUNNING) {
        // Perform sensor calibration operations
        // Note: Sensor reading functionality would be implemented here
        // when the sensor system is properly initialized
        Serial.println("Calibration task running...");
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // Check every second
    }
    
    Serial.println("Calibration task finished");
    calibrationTaskInfo.state = TASK_STOPPED;
    vTaskDelete(nullptr);
}