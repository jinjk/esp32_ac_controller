#ifndef IR_CONTROL_H
#define IR_CONTROL_H

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
#include <Preferences.h>
#include "config.h"

// IR Button mapping enum (must be declared before function declarations)
enum IRButton {
    IR_POWER_ON,
    IR_POWER_OFF,
    IR_TEMP_UP,
    IR_TEMP_DOWN,
    IR_FAN_LOW,
    IR_FAN_MED,
    IR_FAN_HIGH,
    IR_MODE_COOL,
    IR_MODE_HEAT,
    IR_MODE_AUTO,
    IR_SWING_ON,
    IR_SWING_OFF,
    IR_TIMER_ON,
    IR_TIMER_OFF
};

// IR control functions
void initIR();
void loadAllIRCodes();
int countLearnedButtons();
String getIRCodeForButton(IRButton button);
String getButtonPrefsKey(IRButton button);
void saveIRCodeForButton(IRButton button, const String& code);
void handleIRLearning();
void sendIRCommand(const String& hexCode);
void sendIRButton(IRButton button);
void startIRLearning();
void stopIRLearning();
bool processIRLearning();
void moveToNextLearningStep();
void applyACSetting(const ACSetting& setting);
void saveIRCode(const String& code);        // Legacy compatibility
String loadIRCode();                        // Legacy compatibility
void irLearningTask(void* parameter);       // FreeRTOS task for IR learning

// IR Code storage structure
struct IRCodeMap {
    String powerOn;
    String powerOff;
    String tempUp;
    String tempDown;
    String fanLow;
    String fanMed;
    String fanHigh;
    String modeCool;
    String modeHeat;
    String modeAuto;
    String swingOn;
    String swingOff;
    String timerOn;
    String timerOff;
};

// IR learning state
struct IRLearningState {
  bool isLearning;
  IRButton currentButton;
  int learnedButtons;
  int totalButtons;
  unsigned long stepStartTime;
};

// Global IR objects and state
extern IRCodeMap irCodes;
extern IRLearningState learningState;
extern const char* buttonNames[];

// Global IR objects
extern IRrecv irrecv;
extern IRsend irsend;
extern decode_results results;
extern Preferences preferences;

#endif
