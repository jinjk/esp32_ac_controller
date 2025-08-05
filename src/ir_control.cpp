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
  Serial.printf("Loaded %d/%d IR codes\n", learningState.learnedButtons, learningState.totalButtons);
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
