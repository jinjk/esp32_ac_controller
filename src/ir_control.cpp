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
    
    Serial.println("Initializing Gree AC for Chinese market compatibility");
    
    // Set initial state - AC off with reasonable defaults
    ac.off();
    ac.setTemp(24);  // Default temperature
    ac.setFan(kGreeAuto);  // Auto fan
    ac.setMode(kGreeCool);  // Cool mode
    ac.setSwingVertical(false, kGreeSwingAuto);
    ac.setSwingHorizontal(kGreeSwingAuto);
    
    // Try sending a test command to verify communication
    Serial.println("Sending test power toggle command...");
    ac.send();
    delay(500);
    ac.send(); // Send twice for reliability
    
    Serial.println("Gree AC controller initialized for Chinese market");
    Serial.println("AC ready for control");
}

// Power control
void GreeACController::powerOn() {
    Serial.println("Configuring AC Power ON for Chinese Gree AC...");
    ac.on();
    _isOn = true;
    Serial.println("AC: Power configured to ON (ready to send)");
}

void GreeACController::powerOff() {
    Serial.println("Configuring AC Power OFF for Chinese Gree AC...");
    ac.off();
    _isOn = false;
    Serial.println("AC: Power configured to OFF (ready to send)");
}

bool GreeACController::isPowerOn() {
    return _isOn;
}

// Temperature control
void GreeACController::setTemperature(uint8_t temp) {
    if (temp >= 16 && temp <= 32) {
        ac.setTemp(temp);
        Serial.printf("AC: Temperature configured to %d°C (not sent yet)\n", temp);
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
    Serial.printf("AC: Fan speed configured to %d (not sent yet)\n", speed);
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
    Serial.printf("AC: Mode configured to %d (not sent yet)\n", mode);
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
    Serial.printf("AC: Vertical swing configured %s (not sent yet)\n", enable ? "ON" : "OFF");
}

void GreeACController::setSwingH(bool enable) {
    ac.setSwingHorizontal(enable ? kGreeSwingAuto : kGreeSwingHOff);
    Serial.printf("AC: Horizontal swing configured %s (not sent yet)\n", enable ? "ON" : "OFF");
}

// Enhanced swing position control
void GreeACController::setSwingVPosition(int position) {
    switch (position) {
        case 0: // Auto
            ac.setSwingVertical(true, kGreeSwingAuto);
            Serial.println("AC: Vertical swing configured to Auto (not sent yet)");
            break;
        case 1: // Top
            ac.setSwingVertical(false, kGreeSwingUp);
            Serial.println("AC: Vertical swing configured to Top (not sent yet)");
            break;
        case 2: // Mid
            ac.setSwingVertical(false, kGreeSwingMiddle);
            Serial.println("AC: Vertical swing configured to Mid (not sent yet)");
            break;
        case 3: // Bottom
            ac.setSwingVertical(false, kGreeSwingDown);
            Serial.println("AC: Vertical swing configured to Bottom (not sent yet)");
            break;
        default:
            ac.setSwingVertical(true, kGreeSwingAuto);
            Serial.println("AC: Vertical swing configured to Auto (default, not sent yet)");
            break;
    }
}

void GreeACController::setSwingHPosition(int position) {
    switch (position) {
        case 0: // Auto
            ac.setSwingHorizontal(kGreeSwingAuto);
            Serial.println("AC: Horizontal swing configured to Auto (not sent yet)");
            break;
        case 1: // Left
            ac.setSwingHorizontal(kGreeSwingHLeft);
            Serial.println("AC: Horizontal swing configured to Left (not sent yet)");
            break;
        case 2: // Mid
            ac.setSwingHorizontal(kGreeSwingHMiddle);
            Serial.println("AC: Horizontal swing configured to Mid (not sent yet)");
            break;
        case 3: // Right
            ac.setSwingHorizontal(kGreeSwingHRight);
            Serial.println("AC: Horizontal swing configured to Right (not sent yet)");
            break;
        default:
            ac.setSwingHorizontal(kGreeSwingAuto);
            Serial.println("AC: Horizontal swing configured to Auto (default, not sent yet)");
            break;
    }
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
        uint16_t hours = minutes / 60;
        Serial.printf("AC: Timer configured to %d minutes (%d hours) (not sent yet)\n", minutes, hours);
    }
}

uint16_t GreeACController::getTimer() {
    return ac.getTimer();
}

void GreeACController::clearTimer() {
    ac.setTimer(0);
    Serial.println("AC: Timer cleared (not sent yet)");
}

// Send command to AC
void GreeACController::sendCommand() {
    Serial.println("=== Sending IR Command ===");
    Serial.printf("Raw IR Data: 0x%016llX\n", ac.getRaw());
    Serial.printf("Command details - Power: %s, Temp: %d°C, Fan: %d, Mode: %d\n", 
                  ac.getPower() ? "ON" : "OFF", ac.getTemp(), ac.getFan(), ac.getMode());
    
    // Send command multiple times for Chinese AC compatibility
    ac.send();
    delay(200);
    ac.send(); // Double send for reliability with Chinese models
    delay(100);
    
    Serial.println("IR command sent successfully (double transmission)");
    Serial.println("=========================");
}

// Send all configured settings at once (optimized for multiple changes)
void GreeACController::sendAllSettings() {
    Serial.println("=== Sending Complete AC Configuration ===");
    Serial.printf("Power: %s, Temp: %d°C, Fan: %d, Mode: %d\n", 
                  ac.getPower() ? "ON" : "OFF", ac.getTemp(), ac.getFan(), ac.getMode());
    Serial.printf("Raw IR Data: 0x%016llX\n", ac.getRaw());
    
    // Send the complete configuration (all attributes set, single transmission)
    ac.send();
    delay(500);
    ac.send(); // Double send for Chinese AC reliability
    delay(500);
    ac.send(); // Triple send for Chinese AC reliability
    delay(100);
    Serial.println("Complete AC configuration sent successfully");
    Serial.println("==========================================");
}

// Alternative method for stubborn Chinese Gree ACs
void GreeACController::sendRawCommand() {
    Serial.println("Trying alternative transmission method for Chinese AC...");
    
    // Try sending with longer delays and multiple attempts
    for (int i = 0; i < 3; i++) {
        ac.send();
        delay(300); // Longer delay between attempts
        Serial.printf("Transmission attempt %d/3\n", i + 1);
    }
    
    Serial.println("Alternative transmission completed");
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

bool isIRReadyForControl() {
    return greeAC.isReady();
}
