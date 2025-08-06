#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <cstdint>

// Power management functions
void initPowerManagement();
void enableLightSleep();
void configureCpuFrequency();
void optimizePeripherals();

// Power saving configuration
struct PowerConfig {
    int maxCpuFreq = 160;    // MHz - Lower frequency for power saving
    int minCpuFreq = 80;     // MHz - Minimum for stable operation
    uint32_t lightSleepThreshold = 1000; // microseconds
    bool enableAutomaticLightSleep = true;
};

extern PowerConfig powerConfig;

#endif
