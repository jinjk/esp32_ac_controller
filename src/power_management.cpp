#include "power_management.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_pm.h>
#include <esp_sleep.h>

PowerConfig powerConfig;

void initPowerManagement() {
    Serial.println("Initializing power management...");
    
    // Configure CPU frequency scaling for power efficiency
    configureCpuFrequency();
    
    // Enable automatic light sleep when possible
    if (powerConfig.enableAutomaticLightSleep) {
        enableLightSleep();
    }
    
    // Optimize peripheral power usage
    optimizePeripherals();
    
    Serial.printf("Power management initialized - CPU: %d-%d MHz\n", 
                  powerConfig.minCpuFreq, powerConfig.maxCpuFreq);
}

void configureCpuFrequency() {
    // Configure CPU frequency scaling
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = powerConfig.maxCpuFreq,
        .min_freq_mhz = powerConfig.minCpuFreq,
        .light_sleep_enable = powerConfig.enableAutomaticLightSleep
    };
    
    esp_err_t ret = esp_pm_configure(&pm_config);
    if (ret == ESP_OK) {
        Serial.println("CPU frequency scaling configured successfully");
    } else {
        Serial.printf("Failed to configure CPU frequency scaling: %s\n", esp_err_to_name(ret));
    }
}

void enableLightSleep() {
    // Configure automatic light sleep
    esp_sleep_enable_timer_wakeup(1000000); // Wake up every 1 second minimum
    
    // Allow light sleep when system is idle
    esp_pm_lock_type_t lock_type = ESP_PM_CPU_FREQ_MAX;
    esp_pm_lock_handle_t pm_lock;
    esp_pm_lock_create(lock_type, 0, "main", &pm_lock);
    
    Serial.println("Automatic light sleep enabled");
}

void optimizePeripherals() {
    // Reduce WiFi power consumption
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM); // Enable WiFi power saving
    
    // Configure peripheral clock gating for unused peripherals
    // This is automatically handled by ESP-IDF for most peripherals
    
    Serial.println("Peripheral power optimization applied");
}
