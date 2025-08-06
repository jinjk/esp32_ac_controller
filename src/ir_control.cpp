#include "ir_control.h"
#include "config.h"

// Global Gree AC controller instance
GreeACController greeAC;

// Constructor
GreeACController::GreeACController() : ac(IR_SEND_PIN), irsend(IR_SEND_PIN) {
    // Initialize state variables
    _isOn = false;
}

void GreeACController::init() {
    // Initialize the IR sender
    irsend.begin();
    ac.begin();
    
    // Set initial state - AC off with reasonable defaults
    ac.off();
    ac.setTemp(24);  // Default temperature
    ac.setFan(kGreeAuto);  // Auto fan
    ac.setMode(kGreeCool);  // Cool mode
    ac.setSwingVertical(false, kGreeSwingAuto);
    ac.setSwingHorizontal(kGreeSwingAuto);
    
    Serial.println("Gree AC controller initialized");
    Serial.println("AC ready for control");
}

// Power control
void GreeACController::powerOn() {
    ac.on();
    sendCommand();
    _isOn = true;
    Serial.println("AC: Power ON");
}

void GreeACController::powerOff() {
    ac.off();
    sendCommand();
    _isOn = false;
    Serial.println("AC: Power OFF");
}

bool GreeACController::isPowerOn() {
    return _isOn;
}

// Temperature control
void GreeACController::setTemperature(uint8_t temp) {
    if (temp >= 16 && temp <= 32) {
        ac.setTemp(temp);
        sendCommand();
        Serial.printf("AC: Temperature set to %d°C\n", temp);
    }
}

uint8_t GreeACController::getTemperature() {
    return ac.getTemp();
}

// Fan speed control
void GreeACController::setFanSpeed(uint8_t speed) {
    uint8_t fanSpeed;
    switch (speed) {
        case 0: fanSpeed = kGreeAuto; break;
        case 1: fanSpeed = kGreeFanMin; break;
        case 2: fanSpeed = kGreeFanMed; break;
        case 3: fanSpeed = kGreeFanMax; break;
        default: fanSpeed = kGreeAuto; break;
    }
    
    ac.setFan(fanSpeed);
    sendCommand();
    Serial.printf("AC: Fan speed set to %d\n", speed);
}

uint8_t GreeACController::getFanSpeed() {
    uint8_t fan = ac.getFan();
    switch (fan) {
        case kGreeFanMin: return 1;
        case kGreeFanMed: return 2;
        case kGreeFanMax: return 3;
        default: return 0; // Auto
    }
}

// Mode control
void GreeACController::setMode(uint8_t mode) {
    uint8_t acMode;
    switch (mode) {
        case 0: acMode = kGreeCool; break;
        case 1: acMode = kGreeHeat; break;
        case 2: acMode = kGreeDry; break;
        case 3: acMode = kGreeFan; break;
        case 4: acMode = kGreeAuto; break;
        default: acMode = kGreeCool; break;
    }
    
    ac.setMode(acMode);
    sendCommand();
    Serial.printf("AC: Mode set to %d\n", mode);
}

uint8_t GreeACController::getMode() {
    uint8_t mode = ac.getMode();
    switch (mode) {
        case kGreeHeat: return 1;
        case kGreeDry: return 2;
        case kGreeFan: return 3;
        case kGreeAuto: return 4;
        default: return 0; // Cool
    }
}

// Swing control
void GreeACController::setSwingV(bool enable) {
    ac.setSwingVertical(enable, kGreeSwingAuto);
    sendCommand();
    Serial.printf("AC: Vertical swing %s\n", enable ? "ON" : "OFF");
}

void GreeACController::setSwingH(bool enable) {
    ac.setSwingHorizontal(enable ? kGreeSwingAuto : kGreeSwingHOff);
    sendCommand();
    Serial.printf("AC: Horizontal swing %s\n", enable ? "ON" : "OFF");
}

bool GreeACController::getSwingV() {
    // Since there's no getSwingVertical, we'll track this state ourselves
    // For now, return false as a safe default
    return false;
}

bool GreeACController::getSwingH() {
    return ac.getSwingHorizontal() != kGreeSwingHOff;
}

// Timer control
void GreeACController::setTimer(uint16_t minutes) {
    if (minutes > 0 && minutes <= 1440) { // Max 24 hours
        ac.setTimer(minutes);
        sendCommand();
        uint16_t hours = minutes / 60;
        Serial.printf("AC: Timer set to %d minutes (%d hours)\n", minutes, hours);
    }
}

uint16_t GreeACController::getTimer() {
    return ac.getTimer();
}

void GreeACController::clearTimer() {
    ac.setTimer(0);
    sendCommand();
    Serial.println("AC: Timer cleared");
}

// Apply AC settings
void GreeACController::applySettings(const ACSetting& setting) {
    // Convert legacy setting format to Gree AC settings
    // ACSetting has: startHour, endHour, temp, wind
    
    setTemperature((uint8_t)setting.temp);
    setFanSpeed(setting.wind);  // wind maps to fan speed
    setMode(kGreeCool);  // Default to cool mode
    
    sendCommand();
}

// Send command to AC
void GreeACController::sendCommand() {
    ac.send();
    delay(100); // Small delay to ensure command is sent
}

// Check if AC is ready
bool GreeACController::isReady() {
    return true; // Gree AC is always ready
}

// Get AC state as string
String GreeACController::getStateString() {
    String state = "AC State: ";
    state += (_isOn ? "ON" : "OFF");
    state += ", Temp: " + String(ac.getTemp()) + "°C";
    state += ", Fan: " + String(getFanSpeed());
    state += ", Mode: " + String(getMode());
    state += ", SwingV: " + String(getSwingV() ? "ON" : "OFF");
    state += ", SwingH: " + String(getSwingH() ? "ON" : "OFF");
    
    uint16_t timer = ac.getTimer();
    if (timer > 0) {
        state += ", Timer: " + String(timer) + "min";
    }
    
    return state;
}

// Legacy API compatibility functions
void initIR() {
    greeAC.init();
}

void applyACSetting(const ACSetting& setting) {
    greeAC.applySettings(setting);
}

bool isIRReadyForControl() {
    return greeAC.isReady();
}

// Convert legacy ACSetting to Gree settings
GreeACSetting convertToGreeSettings(const ACSetting& setting) {
    GreeACSetting greeSetting;
    greeSetting.power = true;  // Default to on when applying settings
    greeSetting.temperature = (uint8_t)setting.temp;
    greeSetting.fanSpeed = setting.wind;
    greeSetting.mode = kGreeCool;  // Default to cool mode
    greeSetting.swingV = false;  // Default values
    greeSetting.swingH = false;
    greeSetting.timer = 0;  // No timer by default
    return greeSetting;
}

// Helper function to send IR command (legacy compatibility)
void sendIRCommand(const String& command) {
    // Parse command and apply appropriate AC setting
    if (command == "power_on") {
        greeAC.powerOn();
    } else if (command == "power_off") {
        greeAC.powerOff();
    } else if (command.startsWith("temp_")) {
        uint8_t temp = command.substring(5).toInt();
        greeAC.setTemperature(temp);
    } else if (command.startsWith("fan_")) {
        uint8_t fan = command.substring(4).toInt();
        greeAC.setFanSpeed(fan);
    } else if (command.startsWith("mode_")) {
        uint8_t mode = command.substring(5).toInt();
        greeAC.setMode(mode);
    } else if (command == "swing_v_on") {
        greeAC.setSwingV(true);
    } else if (command == "swing_v_off") {
        greeAC.setSwingV(false);
    } else if (command == "swing_h_on") {
        greeAC.setSwingH(true);
    } else if (command == "swing_h_off") {
        greeAC.setSwingH(false);
    }
}
