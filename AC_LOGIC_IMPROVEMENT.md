# AC Control Logic Improvement

## Problem Identified
The original AC control logic had a significant flaw where `acOn` (boolean state) was not properly synchronized with `activeSetting` (day/night configuration).

### Original Issue:
- AC settings would only be applied when turning ON from OFF state
- If AC was already running during day→night or night→day transitions, settings wouldn't update
- This could result in running with day settings during night hours (or vice versa)

## Solution Implemented

### Key Changes:
1. **Added Setting Tracking**: Static variable `currentSetting` tracks the currently applied AC configuration
2. **Setting Change Detection**: Logic detects when day/night settings change while AC is running
3. **Dynamic Setting Updates**: AC settings are updated automatically during transitions

### New Logic Flow:

```cpp
// Track currently applied setting
static ACSetting currentSetting = {-1, -1, 0, 0};

// Detect if settings have changed (day/night transition)
bool settingsChanged = (currentSetting.temp != activeSetting.temp || 
                       currentSetting.wind != activeSetting.wind);

// Apply new logic:
if (temperature < 26.0) {
    // Turn OFF if too cold
    if (acOn) {
        acOn = false;
        currentSetting = {-1, -1, 0, 0}; // Reset when AC is off
    }
} else {
    if (!acOn) {
        // Turn ON with current settings
        acOn = true;
        applyACSetting(activeSetting);
        currentSetting = activeSetting;
    } else if (settingsChanged) {
        // AC already ON but settings changed - update them!
        applyACSetting(activeSetting);
        currentSetting = activeSetting;
    }
}
```

## Benefits

1. **Seamless Transitions**: Settings automatically update during day↔night transitions
2. **Power Efficiency**: AC operates with appropriate settings for each time period
3. **User Comfort**: No manual intervention needed for setting changes
4. **Logging**: Clear messages when settings change while AC is running

## Example Scenarios

### Scenario 1: Day to Night Transition
- **Before**: AC running with day settings (high fan) through the night
- **After**: AC automatically switches to night settings (low fan) at transition time

### Scenario 2: Night to Day Transition  
- **Before**: AC running with night settings (low temp) during hot day hours
- **After**: AC automatically switches to day settings (higher temp) at transition time

## Code Quality
- ✅ Maintains backward compatibility
- ✅ No breaking changes to existing API
- ✅ Clear logging for debugging
- ✅ Efficient memory usage (static variable)
- ✅ Build verified successful

This improvement ensures the AC controller truly adapts to time-based schedules without manual intervention.
