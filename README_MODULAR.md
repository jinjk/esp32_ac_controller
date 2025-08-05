# ESP32-S3 AC Controller - Modular Design

## Project Structure

The project has been refactored into a modular design for better maintainability and organization:

```
├── include/
│   ├── config.h          # Global configuration and pin definitions
│   ├── web_server.h      # Web server functionality
│   ├── display.h         # OLED display management
│   ├── sensor.h          # Temperature and sensor readings
│   ├── ir_control.h      # IR learning and transmission
│   └── ac_control.h      # AC control logic and timing
├── src/
│   ├── main.cpp          # Main application entry point
│   ├── config.cpp        # Global variables and configuration
│   ├── web_server.cpp    # Web interface and WiFi management
│   ├── display.cpp       # Display updates and task
│   ├── sensor.cpp        # Sensor reading functions
│   ├── ir_control.cpp    # IR functionality
│   └── ac_control.cpp    # AC control logic
└── platformio.ini        # PlatformIO configuration
```

## Module Description

### 1. **config.h/cpp**
- Central configuration file
- Pin definitions for ESP32-S3
- Global variables (temperature, AC status, settings)
- WiFi credentials and time zone settings

### 2. **web_server.h/cpp**
- WiFi initialization and management
- Web server setup and routes
- Modern responsive web interface
- Settings update handling

### 3. **display.h/cpp**
- OLED display initialization
- Real-time status display
- Display update task (runs on Core 1)

### 4. **sensor.h/cpp**
- SHT31 temperature sensor management
- Wind sensor reading (placeholder for future implementation)
- Temperature validation and error handling

### 5. **ir_control.h/cpp**
- IR receiver and transmitter management
- IR code learning and storage
- NEC protocol transmission
- Preferences storage for learned codes

### 6. **ac_control.h/cpp**
- Main AC control logic
- Day/night mode switching
- Temperature threshold monitoring
- Cloud logging functionality
- Control task (runs on Core 0)

### 7. **main.cpp**
- Application initialization
- Task creation and core assignment
- Main loop for IR learning

## Key Features

### Dual-Core Architecture
- **Core 0**: Time-critical AC control task (high priority)
- **Core 1**: Display updates, web server, IR learning (lower priority)

### Web Interface
- Modern responsive design with emojis
- Real-time status display
- Separate day/night settings
- IR code learning interface

### Error Handling
- WiFi connection timeout
- Sensor reading validation
- Display initialization checks
- Comprehensive serial logging

### Modular Benefits
- **Maintainability**: Each module has a specific responsibility
- **Reusability**: Modules can be easily reused in other projects
- **Testing**: Individual modules can be tested separately
- **Collaboration**: Multiple developers can work on different modules
- **Debugging**: Easier to isolate and fix issues

## How to Use

1. **Update WiFi credentials** in `src/config.cpp`
2. **Compile and upload** to ESP32-S3
3. **Access web interface** at the displayed IP address
4. **Learn IR codes** using the web interface
5. **Configure day/night settings** as needed

## Pin Configuration (ESP32-S3)

| Function | GPIO | Notes |
|----------|------|-------|
| IR Receiver | 14 | Input |
| IR Transmitter | 13 | Output |
| Wind Sensor | 12 | Analog input |
| OLED SDA | 21 | I2C Data |
| OLED SCL | 22 | I2C Clock |

## Future Enhancements

- [ ] MQTT integration for IoT cloud logging
- [ ] OTA (Over-The-Air) firmware updates
- [ ] Multiple IR codes for different AC functions
- [ ] Mobile app integration
- [ ] Advanced scheduling features
- [ ] Energy usage monitoring
