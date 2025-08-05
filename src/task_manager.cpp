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
    
    controlTaskInfo.handle = nullptr;
    controlTaskInfo.state = TASK_STOPPED;
    controlTaskInfo.startTime = 0;
    controlTaskInfo.endTime = 0;
    controlTaskInfo.name = "AC Control";
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

// AC Control Task Functions
bool TaskManager::startControlTask() {
    if (controlTaskInfo.state != TASK_STOPPED) {
        return false; // Task already running or starting
    }
    
    if (!isIRReadyForControl()) {
        Serial.println("Cannot start control task: IR system not ready");
        return false;
    }
    
    controlTaskInfo.state = TASK_STARTING;
    controlTaskInfo.startTime = millis();
    
    BaseType_t result = xTaskCreatePinnedToCore(
        controlTaskWrapper,
        "AC Control Task",
        8192,
        this,
        2,
        &controlTaskInfo.handle,
        0  // Pin to core 0
    );
    
    if (result == pdPASS) {
        controlTaskInfo.state = TASK_RUNNING;
        Serial.println("✅ AC Control Task started successfully");
        return true;
    } else {
        controlTaskInfo.state = TASK_STOPPED;
        controlTaskInfo.handle = nullptr;
        Serial.println("❌ Failed to start AC Control Task");
        return false;
    }
}

bool TaskManager::stopControlTask() {
    if (controlTaskInfo.state == TASK_STOPPED) {
        return true; // Already stopped
    }
    
    controlTaskInfo.state = TASK_STOPPING;
    
    if (controlTaskInfo.handle != nullptr) {
        vTaskDelete(controlTaskInfo.handle);
        controlTaskInfo.handle = nullptr;
    }
    
    controlTaskInfo.state = TASK_STOPPED;
    controlTaskInfo.endTime = millis();
    Serial.println("🛑 AC Control Task stopped");
    return true;
}

TaskState TaskManager::getControlState() {
    return controlTaskInfo.state;
}

bool TaskManager::isControlTaskRunning() {
    return controlTaskInfo.state == TASK_RUNNING;
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
    
    // Control task status
    JsonObject controlTask = doc["control"].to<JsonObject>();
    controlTask["state"] = getStateString(controlTaskInfo.state);
    controlTask["start_time"] = controlTaskInfo.startTime;
    controlTask["end_time"] = controlTaskInfo.endTime;
    controlTask["name"] = controlTaskInfo.name;
    controlTask["ir_ready"] = isIRReadyForControl();
    
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

void TaskManager::controlTaskWrapper(void* parameter) {
    TaskManager* manager = static_cast<TaskManager*>(parameter);
    manager->controlTask();
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
            
            // Check if we should start the control task now that IR is ready
            if (isIRReadyForControl() && !isControlTaskRunning()) {
                Serial.println("🚀 IR learning complete - starting AC Control Task automatically");
                startControlTask();
            }
            
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

void TaskManager::controlTask() {
    // Control task implementation - calls the existing controlTask function
    Serial.println("AC Control task started via Task Manager");
    
    // Call the existing controlTask function from ac_control.cpp
    // We need to import the function declaration
    extern void controlTask(void* param);
    controlTask(nullptr);
    
    // If the control task returns, mark as stopped
    controlTaskInfo.state = TASK_STOPPED;
    Serial.println("AC Control task finished");
    vTaskDelete(nullptr);
}