# ESP32-S3 AC Controller - IR Mapping System Documentation

## 🎯 Overview
The AC controller now features a comprehensive IR button mapping system that can learn and store codes for all AC remote control functions, replacing the previous single-code limitation.

## 🔧 Technical Implementation

### IR Button Types (14 Functions)
- **Power Controls**: ON/OFF
- **Temperature**: UP/DOWN 
- **Fan Speed**: LOW/MEDIUM/HIGH
- **Mode Selection**: COOL/HEAT/AUTO
- **Swing Control**: ON/OFF
- **Timer Control**: ON/OFF

### Core Components

#### 1. IRButton Enum
```cpp
enum IRButton {
    IR_POWER_ON, IR_POWER_OFF,
    IR_TEMP_UP, IR_TEMP_DOWN,
    IR_FAN_LOW, IR_FAN_MED, IR_FAN_HIGH,
    IR_MODE_COOL, IR_MODE_HEAT, IR_MODE_AUTO,
    IR_SWING_ON, IR_SWING_OFF,
    IR_TIMER_ON, IR_TIMER_OFF
};
```

#### 2. IRCodeMap Structure
```cpp
struct IRCodeMap {
    String powerOn, powerOff;
    String tempUp, tempDown;
    String fanLow, fanMed, fanHigh;
    String modeCool, modeHeat, modeAuto;
    String swingOn, swingOff;
    String timerOn, timerOff;
};
```

#### 3. IRLearningState Management
```cpp
struct IRLearningState {
    bool isLearning;
    IRButton currentButton;
    int learnedButtons;
    int totalButtons;
    unsigned long stepStartTime;
};
```

## 🚀 Key Features

### Guided Learning Process
- **Sequential Learning**: Step-by-step guided process for all 14 buttons
- **Timeout Handling**: 30-second timeout per button with automatic skip
- **Progress Tracking**: Real-time display of learning progress (X/14 buttons)
- **Graceful Completion**: Task automatically terminates when learning finishes

### Persistent Storage
- **Individual Storage**: Each button code stored separately in preferences
- **Automatic Loading**: Codes loaded at system initialization
- **Legacy Compatibility**: Maintains backward compatibility with old single-code system

### Advanced AC Control
- **Smart Temperature Control**: Uses temp up/down based on target temperature
- **Fan Speed Mapping**: Directly maps wind speed settings to fan buttons
- **Mode-Specific Commands**: Dedicated buttons for cool/heat/auto modes

## 🌐 Web Interface Enhancements

### Real-Time Status Display
```
📡 IR Buttons: 8/14 learned
```

### Dynamic Learning Progress
```
Learning Power ON
5/14 buttons learned
```

### Interactive Button Testing
- **Visual Grid**: 14 buttons arranged in logical groups
- **Instant Feedback**: Success/error messages with visual indicators
- **Icon-Based UI**: Intuitive icons for each button function

### Button Test Interface
```
⚡ Power ON    ⭕ Power OFF    🔥 Temp UP     ❄️ Temp DOWN
💨 Fan LOW     🌪️ Fan MED      🌊 Fan HIGH    ❄️ Cool Mode
🔥 Heat Mode   🤖 Auto Mode   ↔️ Swing ON   ⏸️ Swing OFF
⏰ Timer ON    ⏱️ Timer OFF
```

## 📊 System Integration

### Task Manager Integration
- **Dynamic Task Creation**: IR learning task created only when needed
- **Automatic Cleanup**: Task deletes itself upon completion
- **Resource Optimization**: No background resources when not learning

### Power Management
- **Efficient Learning**: 100ms delays during learning mode
- **Auto-disable**: IR receiver disabled when not in use
- **Background Compatible**: Works with existing power management

### AC Control Logic
```cpp
void applyACSetting(const ACSetting& setting) {
    if (setting.temp > 24.0) {
        sendIRButton(IR_TEMP_UP);
    } else if (setting.temp < 20.0) {
        sendIRButton(IR_TEMP_DOWN);
    }
    
    switch (setting.wind) {
        case 1: sendIRButton(IR_FAN_LOW); break;
        case 2: sendIRButton(IR_FAN_MED); break;  
        case 3: sendIRButton(IR_FAN_HIGH); break;
    }
}
```

## 🎮 Usage Instructions

### Learning IR Codes
1. **Start Learning**: Click "Start" on IR Learning task in web interface
2. **Follow Prompts**: System will prompt for each button sequentially
3. **Point & Press**: Point AC remote at ESP32-S3 and press requested button
4. **Auto-Progress**: System automatically moves to next button
5. **Completion**: Task completes when all 14 buttons are learned

### Testing Learned Codes
1. **Access Test Panel**: Scroll to "IR Button Test" section in web interface
2. **Click Buttons**: Click any button to test the learned IR code
3. **Visual Feedback**: Success/error messages appear below buttons
4. **Real AC Control**: Buttons directly control your AC unit

### Manual AC Control
- Use the button test interface to manually control your AC
- Each button sends the appropriate IR code to your AC unit
- Perfect for testing individual functions before automation

## 🔄 Migration from Single-Code System

### Automatic Compatibility
- **Legacy Support**: Old `learnedIRHex` still works for power control
- **Gradual Migration**: Can use old and new systems simultaneously  
- **No Data Loss**: Existing learned codes preserved

### Enhanced Functionality
- **Multiple Functions**: Control all AC features, not just power
- **Intelligent Control**: System chooses appropriate commands based on settings
- **Better User Experience**: Visual feedback and progress tracking

## 🛠️ Technical Benefits

### Professional-Grade Implementation
- **Robust Error Handling**: Timeout management and validation
- **Memory Efficient**: Only stores learned codes, skips empty ones
- **Scalable Design**: Easy to add more button types in future

### Development Quality
- **Clean Architecture**: Modular design with clear separation of concerns
- **Well-Documented**: Comprehensive function documentation and validation
- **Type Safety**: Strong typing with enums and structs

### Validation Status
✅ All core components implemented and validated
✅ Web interface integration complete
✅ Backward compatibility maintained  
✅ Power efficiency preserved
✅ Ready for production use

The IR mapping system transforms the ESP32-S3 AC Controller from a basic single-function device into a comprehensive AC remote replacement with intelligent automation capabilities.
