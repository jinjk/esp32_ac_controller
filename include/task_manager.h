#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Task states
enum TaskState {
    TASK_STOPPED,
    TASK_STARTING,
    TASK_RUNNING,
    TASK_STOPPING
};

// Task information structure
struct TaskInfo {
    TaskHandle_t handle;
    TaskState state;
    unsigned long startTime;
    unsigned long endTime;
    String name;
};

// Task Manager class
class TaskManager {
private:
    TaskInfo calibrationTaskInfo;
    TaskInfo controlTaskInfo;
    
public:
    TaskManager();
    
    // Calibration Task Management  
    bool startCalibrationTask();
    bool stopCalibrationTask();
    TaskState getCalibrationState();
    
    // AC Control Task Management
    bool startControlTask();
    bool stopControlTask();
    TaskState getControlState();
    bool isControlTaskRunning();
    
    // General task management
    String getTaskStatus();
    void cleanupFinishedTasks();
    bool isAnyTaskRunning();
    
    // Task control functions
    static void calibrationTaskWrapper(void* parameter);
    static void controlTaskWrapper(void* parameter);
    
private:
    // Helper methods
    String getStateString(TaskState state);
    void calibrationTask();
    void controlTask();
};

// Global task manager instance
extern TaskManager taskManager;

#endif
