// ===== PlatformIO Project: ESP32-S3 AC Controller (Modular Design) =====
#include <Arduino.h>
#include <WiFi.h>

// Include all module headers
#include "config.h"
#include "web_server.h"
#include "display.h"
#include "sensor.h"
#include "ir_control.h"
#include "ac_control.h"
#include "power_management.h"
#include "task_manager.h"

void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize
  
  Serial.println("=== ESP32-S3 AC Controller Starting ===");
  
  // Initialize power management first for optimal efficiency
  initPowerManagement();
  
  // Initialize WiFi first
  initWiFi();
  
  // Initialize time synchronization
  initTime();
  
  // Initialize hardware components
  initDisplay();
  initSensors();
  initIR();
  
  // Initialize rule system
  initDefaultRules();
  Serial.println("✅ Rule system initialized with default rules");
  
  // Setup web server
  setupWebServer();
  
  // Create only essential tasks for maximum power efficiency
  // Core 0: Critical AC control task (high priority) - Gree AC is always ready!
  // Core 1: Display task (low priority)
  // Other tasks (IR Learning, Calibration) are created on-demand via WebUI
  
  // Gree AC is always ready - no learning required!
  taskManager.startControlTask();
  Serial.println("✅ AC Control Task created - Gree AC ready");
  
  xTaskCreatePinnedToCore(displayTask, "Display Task", 4096, NULL, 1, NULL, 1);
  
  Serial.println("=== ESP32-S3 AC Controller Started Successfully! ===");
  Serial.printf("Web interface available at: http://%s\n", WiFi.localIP().toString().c_str());
  Serial.println("📱 Use the WebUI to start tasks like Calibration on-demand");
  Serial.println("🎉 Gree AC control ready - No IR learning required!");
  
  Serial.printf("💾 Free heap: %d bytes, Active tasks: %d\n", ESP.getFreeHeap(), uxTaskGetNumberOfTasks());
  
  // Delete the setup and loop tasks to save memory and power
  vTaskDelete(NULL);
}

void loop() {
  // This will never be called since setup task deletes itself
  // Keep empty for Arduino framework compatibility
}
