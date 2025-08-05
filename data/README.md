# Web Interface Files

This directory contains the HTML files that will be uploaded to the ESP32's SPIFFS filesystem.

## Files

- `index.html` - Main web interface for the ESP32 AC Controller

## Features

The web interface now includes:

1. **Separated HTML from C++ code** - Easier to maintain and modify
2. **IR Control Readiness Indicator** - Shows warning when IR system is not ready
3. **Dynamic Data Injection** - ESP32 data is injected via JavaScript
4. **Responsive Design** - Works on mobile and desktop
5. **Real-time Status Updates** - Updates every 2 seconds

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

The ESP32 will automatically inject current sensor data, settings, and IR status into the HTML template.
