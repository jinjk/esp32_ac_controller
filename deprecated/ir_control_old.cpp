#include "ir_control.h"

// Global IR objects
IRrecv irrecv(IR_RECV_PIN);
IRsend irsend(IR_SEND_PIN);
decode_results results;
Preferences preferences;

// Global IR code mapping and learning state
IRCodeMap irCodes;
IRLearningState learningState;

// Button names for display
const char* buttonNames[] = {
    "Power ON",
    "Power OFF", 
    "Temperature UP",
    "Temperature DOWN",
    "Fan LOW",
    "Fan MEDIUM",
    "Fan HIGH",
    "Mode COOL",
    "Mode HEAT",
    "Mode AUTO",
    "Swing ON",
    "Swing OFF",
    "Timer ON",
    "Timer OFF"
};

void initIR() {
  irsend.begin();
  irrecv.enableIRIn();
  
  preferences.begin("acprefs", false);
  
  // Load all saved IR codes
  loadAllIRCodes();
  
  // Initialize learning state
  learningState.isLearning = false;
  learningState.currentButton = IR_POWER_ON;
  learningState.learnedButtons = countLearnedButtons();
  learningState.totalButtons = sizeof(buttonNames) / sizeof(buttonNames[0]);
  learningState.isReadyForControl = false;
  
  // Update readiness flag based on learned codes
  updateIRControlReadiness();
  
  Serial.println("IR system initialized");
  #include "ir_control.h"

// Global Gree AC controller
GreeACController greeAC;

// Constructor
GreeACController::GreeACController() : ac(IR_SEND_PIN), irsend(IR_SEND_PIN) {
    isInitialized = false;
}

void GreeACController::init() {
    irsend.begin();
    ac.begin();
    
    // Set default state
    ac.off();
    ac.setTemp(24);
    ac.setFan(kGreeAuto);
    ac.setMode(kGreeCool);
    ac.setSwingVertical(false, kGreeSwingAuto);
    ac.setSwingHorizontal(false);
    ac.setXFan(false);
    ac.setLight(true);
    ac.setSleep(false);
    ac.setTurbo(false);
    
    isInitialized = true;
    Serial.println("Gree AC Controller initialized");
}

void GreeACController::powerOn() {
    if (!isInitialized) return;
    ac.on();
    sendCommand();
    Serial.println("AC: Power ON");
}

void GreeACController::powerOff() {
    if (!isInitialized) return;
    ac.off();
    sendCommand();
    Serial.println("AC: Power OFF");
}

bool GreeACController::isPowerOn() {
    return ac.getPower();
}

void GreeACController::setTemperature(uint8_t temp) {
    if (!isInitialized) return;
    if (temp < 16) temp = 16;
    if (temp > 30) temp = 30;
    ac.setTemp(temp);
    Serial.printf("AC: Temperature set to %d°C
", temp);
}

uint8_t GreeACController::getTemperature() {
    return ac.getTemp();
}

void GreeACController::setFanSpeed(uint8_t speed) {
    if (!isInitialized) return;
    
    uint8_t fanSetting;
    switch (speed) {
        case 0: fanSetting = kGreeAuto; break;
        case 1: fanSetting = kGreeFanMin; break;
        case 2: fanSetting = kGreeFanMed; break;
        case 3: fanSetting = kGreeFanMax; break;
        default: fanSetting = kGreeAuto; break;
    }
    
    ac.setFan(fanSetting);
    Serial.printf("AC: Fan speed set to %d
", speed);
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

void GreeACController::setMode(uint8_t mode) {
    if (!isInitialized) return;
    
    uint8_t modeSetting;
    switch (mode) {
        case 0: modeSetting = kGreeAuto; break;
        case 1: modeSetting = kGreeCool; break;
        case 2: modeSetting = kGreeDry; break;
        case 3: modeSetting = kGreeFan; break;
        case 4: modeSetting = kGreeHeat; break;
        default: modeSetting = kGreeCool; break;
    }
    
    ac.setMode(modeSetting);
    Serial.printf("AC: Mode set to %d
", mode);
}

uint8_t GreeACController::getMode() {
    uint8_t mode = ac.getMode();
    switch (mode) {
        case kGreeCool: return 1;
        case kGreeDry: return 2;
        case kGreeFan: return 3;
        case kGreeHeat: return 4;
        default: return 0; // Auto
    }
}

void GreeACController::setSwingV(bool enable) {
    if (!isInitialized) return;
    ac.setSwingVertical(enable, kGreeSwingAuto);
    Serial.printf("AC: Vertical swing %s
", enable ? "ON" : "OFF");
}

void GreeACController::setSwingH(bool enable) {
    if (!isInitialized) return;
    ac.setSwingHorizontal(enable);
    Serial.printf("AC: Horizontal swing %s
", enable ? "ON" : "OFF");
}

bool GreeACController::getSwingV() {
    return ac.getSwingVertical();
}

bool GreeACController::getSwingH() {
    return ac.getSwingHorizontal();
}

void GreeACController::setTimer(uint16_t minutes) {
    if (!isInitialized) return;
    // Gree AC timer is usually in hours, convert minutes to hours
    uint8_t hours = (minutes + 30) / 60; // Round to nearest hour
    if (hours > 24) hours = 24;
    
    // Note: Timer implementation depends on specific Gree model
    // Some models use different timer functions
    Serial.printf("AC: Timer set to %d minutes (%d hours)
", minutes, hours);
}

uint16_t GreeACController::getTimer() {
    // Return timer in minutes
    return 0; // Implementation depends on model
}

void GreeACController::clearTimer() {
    if (!isInitialized) return;
    // Clear timer implementation
    Serial.println("AC: Timer cleared");
}

void GreeACController::applySettings(const ACSetting& setting) {
    if (!isInitialized) return;
    
    // Convert legacy ACSetting to Gree format
    GreeACSetting greeSettings = convertToGreeSettings(setting);
    
    // Apply all settings
    if (greeSettings.power) {
        ac.on();
    } else {
        ac.off();
    }
    
    ac.setTemp(greeSettings.temperature);
    setFanSpeed(greeSettings.fanSpeed);
    setMode(greeSettings.mode);
    setSwingV(greeSettings.swingV);
    setSwingH(greeSettings.swingH);
    
    if (greeSettings.timer > 0) {
        setTimer(greeSettings.timer);
    }
    
    sendCommand();
}

void GreeACController::sendCommand() {
    if (!isInitialized) return;
    ac.send();
    Serial.println("AC: Command sent");
}

bool GreeACController::isReady() {
    return isInitialized;
}

String GreeACController::getStateString() {
    if (!isInitialized) return "Not initialized";
    
    String state = "Power: ";
    state += isPowerOn() ? "ON" : "OFF";
    state += ", Temp: " + String(getTemperature()) + "°C";
    state += ", Fan: " + String(getFanSpeed());
    state += ", Mode: " + String(getMode());
    state += ", SwingV: " + String(getSwingV() ? "ON" : "OFF");
    state += ", SwingH: " + String(getSwingH() ? "ON" : "OFF");
    
    return state;
}

// Legacy API compatibility functions
void initIR() {
    greeAC.init();
    Serial.println("Gree AC system initialized - Learning not required!");
}

void applyACSetting(const ACSetting& setting) {
    if (!greeAC.isReady()) {
        Serial.println("Gree AC not ready");
        return;
    }
    
    greeAC.applySettings(setting);
}

bool isIRReadyForControl() {
    // Gree AC is always ready once initialized - no learning required!
    return greeAC.isReady();
}

GreeACSetting convertToGreeSettings(const ACSetting& setting) {
    GreeACSetting greeSettings;
    
    greeSettings.power = true; // ACSetting assumes power on
    greeSettings.temperature = (uint8_t)setting.temp;
    
    // Convert wind speed to fan speed
    switch (setting.wind) {
        case 1: greeSettings.fanSpeed = 1; break; // Low
        case 2: greeSettings.fanSpeed = 2; break; // Med
        case 3: greeSettings.fanSpeed = 3; break; // High
        case 4: greeSettings.fanSpeed = 3; break; // High (if wind=4)
        case 5: greeSettings.fanSpeed = 3; break; // High (if wind=5)
        default: greeSettings.fanSpeed = 0; break; // Auto
    }
    
    greeSettings.mode = 1; // Cool mode (most common)
    greeSettings.swingV = false; // Default to no swing
    greeSettings.swingH = false;
    greeSettings.timer = 0; // No timer
    
    return greeSettings;
}
  Serial.printf("IR control ready: %s\n", learningState.isReadyForControl ? "YES" : "NO");
}

void loadAllIRCodes() {
  irCodes.powerOn = preferences.getString("ir_power_on", "");
  irCodes.powerOff = preferences.getString("ir_power_off", "");
  irCodes.tempUp = preferences.getString("ir_temp_up", "");
  irCodes.tempDown = preferences.getString("ir_temp_down", "");
  irCodes.fanLow = preferences.getString("ir_fan_low", "");
  irCodes.fanMed = preferences.getString("ir_fan_med", "");
  irCodes.fanHigh = preferences.getString("ir_fan_high", "");
  irCodes.modeCool = preferences.getString("ir_mode_cool", "");
  irCodes.modeHeat = preferences.getString("ir_mode_heat", "");
  irCodes.modeAuto = preferences.getString("ir_mode_auto", "");
  irCodes.swingOn = preferences.getString("ir_swing_on", "");
  irCodes.swingOff = preferences.getString("ir_swing_off", "");
  irCodes.timerOn = preferences.getString("ir_timer_on", "");
  irCodes.timerOff = preferences.getString("ir_timer_off", "");
}

int countLearnedButtons() {
  int count = 0;
  if (irCodes.powerOn.length() > 0) count++;
  if (irCodes.powerOff.length() > 0) count++;
  if (irCodes.tempUp.length() > 0) count++;
  if (irCodes.tempDown.length() > 0) count++;
  if (irCodes.fanLow.length() > 0) count++;
  if (irCodes.fanMed.length() > 0) count++;
  if (irCodes.fanHigh.length() > 0) count++;
  if (irCodes.modeCool.length() > 0) count++;
  if (irCodes.modeHeat.length() > 0) count++;
  if (irCodes.modeAuto.length() > 0) count++;
  if (irCodes.swingOn.length() > 0) count++;
  if (irCodes.swingOff.length() > 0) count++;
  if (irCodes.timerOn.length() > 0) count++;
  if (irCodes.timerOff.length() > 0) count++;
  return count;
}

String getIRCodeForButton(IRButton button) {
  switch (button) {
    case IR_POWER_ON: return irCodes.powerOn;
    case IR_POWER_OFF: return irCodes.powerOff;
    case IR_TEMP_UP: return irCodes.tempUp;
    case IR_TEMP_DOWN: return irCodes.tempDown;
    case IR_FAN_LOW: return irCodes.fanLow;
    case IR_FAN_MED: return irCodes.fanMed;
    case IR_FAN_HIGH: return irCodes.fanHigh;
    case IR_MODE_COOL: return irCodes.modeCool;
    case IR_MODE_HEAT: return irCodes.modeHeat;
    case IR_MODE_AUTO: return irCodes.modeAuto;
    case IR_SWING_ON: return irCodes.swingOn;
    case IR_SWING_OFF: return irCodes.swingOff;
    case IR_TIMER_ON: return irCodes.timerOn;
    case IR_TIMER_OFF: return irCodes.timerOff;
    default: return "";
  }
}

String getButtonPrefsKey(IRButton button) {
  switch (button) {
    case IR_POWER_ON: return "ir_power_on";
    case IR_POWER_OFF: return "ir_power_off";
    case IR_TEMP_UP: return "ir_temp_up";
    case IR_TEMP_DOWN: return "ir_temp_down";
    case IR_FAN_LOW: return "ir_fan_low";
    case IR_FAN_MED: return "ir_fan_med";
    case IR_FAN_HIGH: return "ir_fan_high";
    case IR_MODE_COOL: return "ir_mode_cool";
    case IR_MODE_HEAT: return "ir_mode_heat";
    case IR_MODE_AUTO: return "ir_mode_auto";
    case IR_SWING_ON: return "ir_swing_on";
    case IR_SWING_OFF: return "ir_swing_off";
    case IR_TIMER_ON: return "ir_timer_on";
    case IR_TIMER_OFF: return "ir_timer_off";
    default: return "";
  }
}

void saveIRCodeForButton(IRButton button, const String& code) {
  String key = getButtonPrefsKey(button);
  if (key.length() > 0) {
    preferences.putString(key.c_str(), code);
    
    // Update the in-memory map
    switch (button) {
      case IR_POWER_ON: irCodes.powerOn = code; break;
      case IR_POWER_OFF: irCodes.powerOff = code; break;
      case IR_TEMP_UP: irCodes.tempUp = code; break;
      case IR_TEMP_DOWN: irCodes.tempDown = code; break;
      case IR_FAN_LOW: irCodes.fanLow = code; break;
      case IR_FAN_MED: irCodes.fanMed = code; break;
      case IR_FAN_HIGH: irCodes.fanHigh = code; break;
      case IR_MODE_COOL: irCodes.modeCool = code; break;
      case IR_MODE_HEAT: irCodes.modeHeat = code; break;
      case IR_MODE_AUTO: irCodes.modeAuto = code; break;
      case IR_SWING_ON: irCodes.swingOn = code; break;
      case IR_SWING_OFF: irCodes.swingOff = code; break;
      case IR_TIMER_ON: irCodes.timerOn = code; break;
      case IR_TIMER_OFF: irCodes.timerOff = code; break;
    }
    
    // Update learning statistics and readiness
    updateIRControlReadiness();
    Serial.printf("Saved IR code for %s: %s\n", buttonNames[button], code.c_str());
  }
}

void sendIRCommand(const String& hexCode) {
  if (hexCode.length() > 0) {
    // Convert hex string to uint64_t
    uint64_t code = strtoull(hexCode.c_str(), NULL, 16);
    irsend.sendNEC(code, 32); // Send with 32-bit length
    Serial.println("Sent IR code: " + hexCode);
  } else {
    Serial.println("No IR code to send");
  }
}

void sendIRButton(IRButton button) {
  if (!learningState.isReadyForControl) {
    Serial.println("WARNING: IR system not ready for AC control. Please complete IR learning first.");
    return;
  }
  
  String code = getIRCodeForButton(button);
  if (code.length() > 0) {
    sendIRCommand(code);
    Serial.printf("Sent %s command\n", buttonNames[button]);
  } else {
    Serial.printf("No IR code learned for %s\n", buttonNames[button]);
  }
}

void startIRLearning() {
  learningState.isLearning = true;
  learningState.currentButton = IR_POWER_ON;
  learningState.stepStartTime = millis();
  
  irrecv.enableIRIn();
  Serial.printf("Starting IR learning sequence. Step 1/%d: Please press %s\n", 
                learningState.totalButtons, buttonNames[learningState.currentButton]);
}

void stopIRLearning() {
  learningState.isLearning = false;
  irrecv.disableIRIn();
  learningState.learnedButtons = countLearnedButtons();
  Serial.printf("IR learning stopped. Total learned: %d/%d\n", 
                learningState.learnedButtons, learningState.totalButtons);
}

bool processIRLearning() {
  if (!learningState.isLearning) return false;
  
  // Check timeout (30 seconds per button)
  if (millis() - learningState.stepStartTime > 30000) {
    Serial.printf("Timeout waiting for %s. Skipping to next button.\n", 
                  buttonNames[learningState.currentButton]);
    moveToNextLearningStep();
    return true;
  }
  
  if (irrecv.decode(&results)) {
    String irCode = resultToHexidecimal(&results);
    
    if (irCode.length() > 2 && irCode != "FFFFFFFF") {  // Valid code received
      saveIRCodeForButton(learningState.currentButton, irCode);
      Serial.printf("Learned %s: %s\n", buttonNames[learningState.currentButton], irCode.c_str());
      
      moveToNextLearningStep();
    }
    
    irrecv.resume();
    return true;
  }
  
  return false;
}

void moveToNextLearningStep() {
  learningState.currentButton = (IRButton)(learningState.currentButton + 1);
  
  if (learningState.currentButton >= learningState.totalButtons) {
    // Learning complete
    stopIRLearning();
    Serial.println("IR learning sequence completed!");
    return;
  }
  
  learningState.stepStartTime = millis();
  Serial.printf("Step %d/%d: Please press %s\n", 
                learningState.currentButton + 1, learningState.totalButtons, 
                buttonNames[learningState.currentButton]);
}

void applyACSetting(const ACSetting& setting) {
  if (!learningState.isReadyForControl) {
    Serial.println("WARNING: IR system not ready for AC control. Please complete IR learning first.");
    return;
  }
  
  // Use specific buttons based on setting values
  if (setting.temp > 24.0) {
    sendIRButton(IR_TEMP_UP);
  } else if (setting.temp < 20.0) {
    sendIRButton(IR_TEMP_DOWN);
  }
  
  // Apply fan setting
  switch (setting.wind) {
    case 1: sendIRButton(IR_FAN_LOW); break;
    case 2: sendIRButton(IR_FAN_MED); break;
    case 3: sendIRButton(IR_FAN_HIGH); break;
  }
  
  Serial.printf("Applied AC Setting: Temp %.1f°C, Wind %d\n", setting.temp, setting.wind);
}

bool isIRReadyForControl() {
  return learningState.isReadyForControl;
}

void updateIRControlReadiness() {
  // Define minimum required codes for basic AC control
  // At minimum we need: Power ON/OFF, Temp UP/DOWN, and at least one fan speed
  bool hasPowerControl = (irCodes.powerOn.length() > 0 && irCodes.powerOff.length() > 0);
  bool hasTempControl = (irCodes.tempUp.length() > 0 && irCodes.tempDown.length() > 0);
  bool hasFanControl = (irCodes.fanLow.length() > 0 || irCodes.fanMed.length() > 0 || irCodes.fanHigh.length() > 0);
  
  // Update readiness flag
  learningState.isReadyForControl = hasPowerControl && hasTempControl && hasFanControl;
  learningState.learnedButtons = countLearnedButtons();
  
  Serial.printf("IR Control Readiness Check:\n");
  Serial.printf("  Power Control: %s\n", hasPowerControl ? "OK" : "MISSING");
  Serial.printf("  Temperature Control: %s\n", hasTempControl ? "OK" : "MISSING");
  Serial.printf("  Fan Control: %s\n", hasFanControl ? "OK" : "MISSING");
  Serial.printf("  Ready for AC Control: %s\n", learningState.isReadyForControl ? "YES" : "NO");
}
