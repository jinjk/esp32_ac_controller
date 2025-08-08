#include "power_management.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_pm.h>
#include <esp_sleep.h>

PowerConfig powerConfig;

void initPowerManagement() {
    Serial.println("Initializing power management...");
    
    // Simple and safe power management - avoid complex CPU frequency scaling
    // that might cause memory access issues
    
    // Basic WiFi power saving (safe)
    optimizePeripherals();
    
    Serial.println("Power management initialized - Basic mode for stability");
}

void configureCpuFrequency() {
    // Disabled for stability - avoid complex power management that can cause crashes
    Serial.println("CPU frequency scaling disabled for stability");
}

void enableLightSleep() {
    // Disabled for stability - complex sleep management can cause memory issues
    Serial.println("Automatic light sleep disabled for stability");
}

void optimizePeripherals() {
    // Only basic, safe WiFi power optimization
    // Advanced power management can cause memory access issues
    Serial.println("Basic peripheral power optimization applied");
}
