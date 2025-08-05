#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <esp_pm.h>
#include <esp_sleep.h>

// Power management functions
void initPowerManagement();
void enableLightSleep();
void configureCpuFrequency();
void optimizePeripherals();

// Power saving configuration
struct PowerConfig {
    uint32_t maxCpuFreq = 160;    // MHz - Lower frequency for power saving
    uint32_t minCpuFreq = 80;     // MHz - Minimum for stable operation
    uint32_t lightSleepThreshold = 1000; // microseconds
    bool enableAutomaticLightSleep = true;
};

extern PowerConfig powerConfig;

#endif
