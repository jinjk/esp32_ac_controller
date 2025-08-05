# Web Interface Files

This directory contains the HTML files that will be uploaded to the ESP32's SPIFFS filesystem.

## Files

- `index.html` - Main web interface for the ESP32 AC Controller

## Features

The web interface now uses **REST API endpoints** for dynamic data:

1. **Pure HTML/CSS/JS** - No server-side templating
2. **REST API Integration** - Fetches data via HTTP calls
3. **Real-time Updates** - Polls APIs every 2 seconds
4. **IR Control Safety** - Shows warnings when IR system is not ready
5. **Responsive Design** - Works on mobile and desktop

## API Endpoints

### System Information
- **GET** `/api/system` - Current system status
```json
{
  "currentTemp": 24.5,
  "acOn": true,
  "mode": "Day",
  "currentHour": 14,
  "system": {
    "freeHeap": 234567,
    "uptime": 12345678,
    "activeTasks": 8,
    "chipCores": 2,
    "cpuFreq": 240,
    "flashSize": 16777216,
    "psramSize": 8388608
  },
  "ir": {
    "ready": true,
    "learnedButtons": 12,
    "totalButtons": 14,
    "isLearning": false
  }
}
```

### Settings
- **GET** `/api/settings` - Current AC settings
```json
{
  "day": {
    "temp": 24.0,
    "wind": 3,
    "startHour": 8,
    "endHour": 19
  },
  "night": {
    "temp": 26.0,
    "wind": 1
  }
}
```

### Task Management
- **GET** `/status` - Task status (existing)
- **POST** `/task` - Control tasks (existing)

### IR Control
- **POST** `/send_ir` - Send IR commands (existing)
- **POST** `/learn` - Start IR learning (existing)

## IR Safety Features

- **Warning Banner**: Displays when IR system is not ready for AC control
- **Button Disabling**: IR control buttons are disabled until minimum required codes are learned
- **Status Indicators**: Shows learned button count and readiness status

### Minimum Required IR Codes

For the system to be considered "ready for control":
- Power ON and Power OFF
- Temperature UP and Temperature DOWN
- At least one Fan speed (LOW, MED, or HIGH)

## Upload Instructions

To upload the HTML files to SPIFFS:

```bash
# Upload filesystem
pio run --target uploadfs

# Or build and upload both code and filesystem
pio run --target upload
pio run --target uploadfs
```

## Development

The interface automatically loads data on page load and refreshes every 2 seconds:

1. **Initial Load**: 
   - Fetches `/api/system` for current status
   - Fetches `/api/settings` for AC settings
   - Updates all UI elements

2. **Periodic Updates**:
   - Polls `/status` for task information
   - Refreshes `/api/system` for live data

3. **Event-Based Updates**:
   - Reloads settings after form submission
   - Updates IR status after learning tasks

## File Structure

```
data/
├── index.html          # Main web interface
└── README.md          # This file
```

## Customization

To modify the web interface:

1. Edit `data/index.html`
2. Upload to SPIFFS: `pio run --target uploadfs`
3. Reset the ESP32 or restart the web server

The ESP32 serves fresh JSON data through the REST APIs, keeping the HTML completely static and cacheable.
