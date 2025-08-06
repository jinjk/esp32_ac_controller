#include "task_manager.h"
#include "config.h"
#include "ir_control.h"
#include "sensor.h"
#include "ac_control.h"
#include <ArduinoJson.h>

// Global task manager instance
TaskManager taskManager;

// Constructor
TaskManager::TaskManager() {
    // Initialize task info structures
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
    if (calibrationTaskInfo.state == TASK_STOPPED && calibrationTaskInfo.handle != nullptr) {
        calibrationTaskInfo.handle = nullptr;
    }
    
    if (controlTaskInfo.state == TASK_STOPPED && controlTaskInfo.handle != nullptr) {
        controlTaskInfo.handle = nullptr;
    }
}

bool TaskManager::isAnyTaskRunning() {
    return (calibrationTaskInfo.state == TASK_RUNNING || 
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
void TaskManager::calibrationTaskWrapper(void* parameter) {
    TaskManager* manager = static_cast<TaskManager*>(parameter);
    manager->calibrationTask();
}

void TaskManager::controlTaskWrapper(void* parameter) {
    TaskManager* manager = static_cast<TaskManager*>(parameter);
    manager->controlTask();
}

// Task implementation functions
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
    ::controlTask(nullptr);
    
    // If the control task returns, mark as stopped
    controlTaskInfo.state = TASK_STOPPED;
    Serial.println("AC Control task finished");
    vTaskDelete(nullptr);
}