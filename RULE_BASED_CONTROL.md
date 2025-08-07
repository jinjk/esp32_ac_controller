# Rule-Based AC Control System 🎯

## Overview

The ESP32 AC Controller now features a powerful **rule-based control system** that allows you to create custom automation rules for your air conditioner based on time, temperature, or both conditions.

## ✨ Key Features

### 🎯 **Flexible Rule Creation**
- Create unlimited rules (up to 10 maximum)
- Combine time and temperature conditions
- Set specific AC actions for each rule
- Enable/disable rules on demand

### 🕐 **Time-Based Control**
- Set specific hour ranges (e.g., 8AM-7PM)
- Support overnight periods (e.g., 7PM-8AM)
- Use -1 for "any time" conditions

### 🌡️ **Temperature-Based Control**
- Set minimum temperature thresholds
- Set maximum temperature thresholds
- Use -999 for "any temperature" conditions

### 🎛️ **Complete AC Control**
- Turn AC on/off
- Set target temperature (16-32°C)
- Control fan speed (Auto, Low, Medium, High)
- Select AC mode (Cool, Heat, Dry, Fan, Auto)

## 📋 Rule Logic

### **First Match Wins**
- Rules are evaluated in order (ID ascending)
- First matching rule is applied
- Remaining rules are skipped
- No rule = AC unchanged

### **Condition Matching**
- **Time conditions**: Must match if specified
- **Temperature conditions**: Must match if specified
- **Both must be true** for rule to apply

## 🌟 Default Rules

The system comes with 3 pre-configured rules:

### **Rule 1: Cool Day** 
- **Time**: 8AM - 7PM
- **Temperature**: ≥26°C
- **Action**: AC ON, 27°C, Fan High, Cool mode

### **Rule 2: Cool Night**
- **Time**: 7PM - 8AM  
- **Temperature**: ≥26°C
- **Action**: AC ON, 28°C, Fan Low, Cool mode

### **Rule 3: Turn Off When Cool**
- **Time**: Any time
- **Temperature**: ≤25.9°C
- **Action**: AC OFF

## 🖥️ Web Interface

### **📊 Status Dashboard**
- Current temperature and time
- Active rule information
- Real-time AC status

### **📝 Rule Management**
- View all rules in priority order
- Create new rules instantly
- Edit existing rules
- Enable/disable rules
- Delete unwanted rules

### **✏️ Rule Editor**
- **Basic Info**: Name, enabled status
- **Time Conditions**: Start/end hours
- **Temperature Conditions**: Min/max thresholds  
- **AC Actions**: Power, temperature, fan, mode

## 🔧 API Endpoints

### **GET /api/rules**
Returns all rules with current status

### **POST /api/rules** 
Creates a new rule with default settings

### **PUT /api/rules**
Updates an existing rule (requires ID + parameters)

### **DELETE /api/rules?id=X**
Deletes rule with specified ID

### **GET /api/rules/active**
Returns currently active rule and system status

## 💡 Example Use Cases

### **🏠 Basic Home Automation**
```
Rule 1: Hot Day Cooling
- Time: 10AM - 6PM
- Temp: ≥28°C
- Action: AC ON, 26°C, Fan High

Rule 2: Night Comfort  
- Time: 10PM - 6AM
- Temp: ≥25°C
- Action: AC ON, 27°C, Fan Low

Rule 3: Energy Saving
- Time: Any
- Temp: ≤24°C  
- Action: AC OFF
```

### **🏢 Office Environment**
```
Rule 1: Work Hours
- Time: 8AM - 5PM  
- Temp: ≥26°C
- Action: AC ON, 24°C, Fan Medium

Rule 2: After Hours
- Time: 5PM - 8AM
- Temp: ≥30°C
- Action: AC ON, 28°C, Fan Low

Rule 3: Weekend Mode
- Time: Saturday/Sunday (use multiple rules)
- Temp: ≥27°C
- Action: AC ON, 26°C, Fan Low
```

### **🌡️ Climate-Specific Rules**
```
Rule 1: Extreme Heat
- Time: Any
- Temp: ≥35°C
- Action: AC ON, 22°C, Fan High

Rule 2: Moderate Heat
- Time: Any  
- Temp: 28-34°C
- Action: AC ON, 25°C, Fan Medium

Rule 3: Comfortable
- Time: Any
- Temp: ≤27°C
- Action: AC OFF
```

## 🔄 System Behavior

### **60-Second Evaluation Cycle**
- Rules checked every 60 seconds
- Active rule applied immediately
- Status logged to console and web UI

### **Manual Override Handling**
- System always sends commands every cycle
- Manual AC changes get overridden
- Ensures rules are always enforced

### **Error Handling**
- Invalid rules are skipped
- System continues with remaining rules
- Web UI shows error messages

## 🎯 Benefits

### ✅ **User-Friendly**
- Intuitive web interface
- Real-time status updates
- Easy rule creation and editing

### ✅ **Flexible** 
- Any combination of conditions
- Multiple rule strategies
- Dynamic enable/disable

### ✅ **Reliable**
- First-match rule logic
- Automatic rule enforcement
- Manual override protection

### ✅ **Efficient**
- Lightweight rule engine
- Minimal memory usage
- Fast rule evaluation

## 🚀 Getting Started

1. **Access Web Interface**: Visit `http://[ESP32-IP]/`
2. **View Current Rules**: Scroll to "Rule-Based AC Control" section
3. **Create First Rule**: Click "➕ Add New Rule"
4. **Configure Conditions**: Set time and temperature ranges
5. **Set AC Actions**: Choose power, temp, fan, mode settings
6. **Save and Test**: Save rule and monitor status

The rule-based system transforms your AC controller into a smart, automated climate control solution! 🌡️✨
