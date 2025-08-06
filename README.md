# ESP32-S3 AC Controller Project Documentation

## 🎯 Project Target

Design and implement a **smart AC controller** using an **ESP32-S3 (32 N16R8)** development board. The system will:

- Read temperature and humidity from a digital sensor.
- Automatically control an air conditioner based on time and temperature.
- Support IR remote learning and IR control of the AC.
- Offer a web-based UI for configuration.
- Display system status on a 0.91" OLED screen.
- Log data to an IoT platform (future extension).
- Be modular, low-power, and based on FreeRTOS tasks.

---

## 🔌 Hardware and Modules Used

| Module                 | Description                              | Interface |
| ---------------------- | ---------------------------------------- | --------- |
| **ESP32-S3 Dev Board** | Main controller with Wi-Fi, PSRAM, USB-C | Core MCU  |
| **SHT3x Sensor**       | Temperature & Humidity (I²C)             | I²C       |
| **IR Transmitter**     | Sends IR signals to Gree AC              | GPIO      |
| **Wind Speed Sensor**  | Fan-based rotation encoder               | GPIO      |
| **0.91" OLED Display** | SSD1306 I²C screen for real-time info    | I²C       |

**Note**: ❌ **IR Receiver removed** - Not required for Gree AC (uses built-in library)

---

## 🧠 System Behavior & Logic

### Temperature-Based Control:

- **If temperature < 26°C**, turn **OFF** the AC.
- **If day (8:00–19:00)**: target = **27°C**, wind level = **3**.
- **If night (19:00–8:00)**: target = **28°C**, wind level = **2**, direction = toward wind sensor.

### Web UI Functions:

- ✅ Set day/night temperature and wind level
- ✅ Monitor system status and control AC manually  
- ✅ View real-time sensor data
- ❌ **IR Learning removed** - Not needed for Gree AC (uses built-in commands)
- Hosted by ESP32 using AsyncWebServer

### OLED Display Shows:

- Temperature
- AC status
- Day/Night mode

### IoT Logging (Planned):

- Use MQTT or HTTP to send temperature and AC state to cloud.

---

## 🧵 FreeRTOS Task Breakdown

| Task Name      | Purpose                                   | Notes                |
| -------------- | ----------------------------------------- | -------------------- |
| `controlTask`  | Temperature monitoring + AC control logic | Runs every 60s       |
| `displayTask`  | Updates OLED screen                       | Runs every 5s        |
| (Future) OTA   | Manage OTA updates                        | Optional enhancement |
| (Future) Cloud | Handle cloud logging                      | Optional enhancement |

---

## 🌐 PlatformIO Setup

**platformio.ini** includes:

```ini
[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
lib_deps =
  ESP Async WebServer
  IRremoteESP8266
  adafruit/Adafruit SHT31 Library
  adafruit/Adafruit SSD1306
  adafruit/Adafruit GFX Library
  esphome/AsyncTCP
```

---

## � **IR Receiver Removal (Important)**

### **Why No IR Receiver?**
This project uses the **Gree AC library** from `IRremoteESP8266`, which provides:
- ✅ **Built-in IR commands** for all Gree AC functions
- ✅ **No learning required** - works immediately 
- ✅ **Professional-grade** IR codes (more reliable than captured ones)
- ✅ **Simplified hardware** - only IR LED needed

### **Hardware Impact**
- **GPIO 14** now **available for other uses** (was IR_RECV_PIN)
- **Component savings**: No IR receiver sensor needed
- **Wiring simplified**: One less component to connect

### **Software Benefits**  
- **Instant ready** - no learning phase required
- **Code reduction** - ~500+ lines of IR learning code removed
- **Memory savings** - no IR code storage needed
- **Better reliability** - library-tested commands

**📄 See `IR_RECEIVER_REMOVAL.md` for detailed documentation**

---

## 📦 Dependencies

```
esp32_ac_controller/
├── platformio.ini
└── src/
    └── main.cpp  ← All code in one structured file
```

---

## 🚀 Optional Future Extensions

- OTA firmware updates via browser or PlatformIO
- MQTT logging to ThingsBoard / Adafruit IO
- Use wind sensor for intelligent direction adjustment
- Servo control for motorized AC direction
- Deep sleep or light sleep for power optimization

---

Let me know if you want diagrams, flowcharts, or pinout illustrations included!

