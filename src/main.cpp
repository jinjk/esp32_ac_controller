// ===== PlatformIO Project: ESP32-S3 AC Controller (Modular Design) =====
#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>

// Include all module headers
#include "config.h"
#include "web_server.h"
#include "display.h"
#include "sensor.h"
#include "ir_control.h"
#include "ac_control.h"
#include "power_management.h"
#include "task_manager.h"

// Initialize SPIFFS file system
void initSPIFFS() {
  Serial.println("Initializing SPIFFS...");
  if(!SPIFFS.begin(true)){
    Serial.println("❌ An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("✅ SPIFFS mounted successfully");
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize
  
  Serial.println("=== ESP32-S3 AC Controller Starting ===");
  
  // Initialize power management first for optimal efficiency
  initPowerManagement();
  
  // Initialize SPIFFS file system first
  initSPIFFS();
  
  // Initialize WiFi
  initWiFi();
  
  // Give WiFi extra time to stabilize before time sync
  delay(2000);
  
  // Initialize time synchronization (after WiFi is stable)
  initTime();
  
  // Initialize hardware components
  initDisplay();
  initSensors();
  initIR();
  
  // Initialize rule system with persistence (after SPIFFS is mounted)
  loadRulesFromSPIFFS();
  Serial.println("✅ Rule system initialized with persistent storage");
  
  // Setup web server (SPIFFS already initialized)
  setupWebServer();
  
  // Create only essential tasks for maximum power efficiency
  // Core 0: Critical AC control task (high priority) - Gree AC is always ready!
  // Core 1: Display task (low priority)
  
  // Gree AC is always ready - no learning required!
  taskManager.startControlTask();
  Serial.println("✅ AC Control Task created - Gree AC ready");
  
  xTaskCreatePinnedToCore(displayTask, "Display Task", 4096, NULL, 1, NULL, 1);
  
  Serial.println("=== ESP32-S3 AC Controller Started Successfully! ===");
  Serial.printf("Web interface available at: http://%s\n", WiFi.localIP().toString().c_str());
  Serial.println("🎉 Gree AC control ready - No IR learning required!");
  
  Serial.printf("💾 Free heap: %d bytes, Active tasks: %d\n", ESP.getFreeHeap(), uxTaskGetNumberOfTasks());
  
  // Delete the setup and loop tasks to save memory and power
  vTaskDelete(NULL);
}

void loop() {
  // This will never be called since setup task deletes itself
  // Keep empty for Arduino framework compatibility
}
