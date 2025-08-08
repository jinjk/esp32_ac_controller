#ifndef IR_CONTROL_H
#define IR_CONTROL_H

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>
#include "config.h"

// Gree AC Control Interface
class GreeACController {
private:
    IRGreeAC ac;
    IRsend irsend;
    bool isInitialized;
    bool _isOn;

public:
    GreeACController();
    void init();
    
    // Basic AC Controls
    void powerOn();
    void powerOff();
    bool isPowerOn();
    
    // Temperature Control
    void setTemperature(uint8_t temp);
    uint8_t getTemperature();
    
    // Fan Control
    void setFanSpeed(uint8_t speed); // 0=Auto, 1=Low, 2=Med, 3=High
    uint8_t getFanSpeed();
    
    // Mode Control
    void setMode(uint8_t mode); // 0=Auto, 1=Cool, 2=Dry, 3=Fan, 4=Heat
    uint8_t getMode();
    
    // Swing Control
    void setSwingV(bool enable);
    void setSwingH(bool enable);
    void setSwingVPosition(int position); // 0=auto, 1=top, 2=mid, 3=bottom
    void setSwingHPosition(int position); // 0=auto, 1=left, 2=mid, 3=right
    bool getSwingV();
    bool getSwingH();
    
    // Timer Control
    void setTimer(uint16_t minutes);
    uint16_t getTimer();
    void clearTimer();
    
    // Command Control
    void sendCommand();
    void sendAllSettings(); // Send all configured settings at once
    void sendRawCommand(); // Alternative method for Chinese ACs
    
    // Status
    bool isReady();
    String getStateString();
};

// Simplified AC Setting structure for Gree
struct GreeACSetting {
    bool power;
    uint8_t temperature;  // 16-30°C
    uint8_t fanSpeed;     // 0=Auto, 1=Low, 2=Med, 3=High  
    uint8_t mode;         // 0=Auto, 1=Cool, 2=Dry, 3=Fan, 4=Heat
    bool swingV;
    bool swingH;
    uint16_t timer;       // minutes, 0=off
};

// Global AC controller instance
extern GreeACController greeAC;

// Simplified API functions (backward compatibility)
void initIR();
bool isIRReadyForControl();

#endif
