# ESP32-S3 AC Controller Circuit Diagram

## Overview
This document provides the complete circuit diagram and GPIO connection details for the ESP32-S3 based AC Controller with rule-based automation system.

## Components Required

### Main Components
- **ESP32-S3 Development Board** (38-pin or 30-pin)
- **SHT31 Temperature/Humidity Sensor** (I2C)
- **OLED Display 128x32** (I2C, SSD1306)
- **IR LED Transmitter** (940nm)
- **IR LED Driver Circuit** (optional amplification)

### Supporting Components
- **2x 4.7kΩ Resistors** (I2C pull-up)
- **1x 220Ω Resistor** (IR LED current limiting)
- **1x 2N2222 NPN Transistor** (IR LED driver - optional)
- **Breadboard or PCB**
- **Jumper Wires**
- **5V Power Supply** (for ESP32-S3)

## GPIO Pin Assignment

| GPIO | Function | Component | Notes |
|------|----------|-----------|-------|
| GPIO13 | IR_SEND_PIN | IR LED Transmitter | PWM output for Gree AC control |
| GPIO21 | OLED_SDA | OLED Display | I2C Data line |
| GPIO22 | OLED_SCL | OLED Display | I2C Clock line |
| GPIO21 | SDA | SHT31 Sensor | Shared I2C Data line |
| GPIO22 | SCL | SHT31 Sensor | Shared I2C Clock line |
| GPIO12 | Available | - | Available for future use |
| GPIO14 | Available | - | Available for future use |

## Circuit Diagram

```
                           ESP32-S3 Development Board
                        ┌─────────────────────────────────┐
                        │                                 │
                        │  ┌─────┐         ┌─────┐        │
                        │  │ USB │         │RESET│        │
                        │  └─────┘         └─────┘        │
                        │                                 │
Power Supply            │  3V3 ●                    ● GND │ ──┐
    ┌──────────────────── EN  ●                    ● D43 │   │
    │                   │ GPIO36●                    ● D44 │   │
    │     ┌──────────────── GPIO35●                    ● D1  │   │
    │     │             │ GPIO37●                    ● D2  │   │
    │     │             │ GPIO38●                    ● D42 │   │
    │     │             │ GPIO39●                    ● D41 │   │
    │     │             │ GPIO40●                    ● D40 │   │
    │     │             │ GPIO41●                    ● D39 │   │
    │     │             │ GPIO42●                    ● D38 │   │
    │     │   ┌──────────── GPIO2 ●                    ● D37 │   │
    │     │   │         │ GPIO1 ●                    ● D36 │   │
    │     │   │         │ GND   ●                    ● D35 │   │
    │     │   │         │ GPIO46●                    ● D0  │   │
    │     │   │         │ GPIO45●                    ● D45 │   │
    │     │   │         │ GPIO48●                    ● D48 │   │
    │     │   │         │ GPIO47●                    ● D47 │   │
    │     │   │         │ GPIO21●─────────────────────────────┼───┐ SDA (I2C)
    │     │   │         │ GPIO20●                    ● D21 │   │ │
    │     │   │         │ GPIO19●                    ● D20 │   │ │
    │     │   │         │ GPIO18●                    ● D19 │   │ │
    │     │   │         │ GPIO17●                    ● D18 │   │ │
    │     │   │         │ GPIO16●                    ● D17 │   │ │
    │     │   │         │ GPIO15●                    ● D16 │   │ │
    │     │   │         │ GPIO14●  (Available)       ● D15 │   │ │
    │     │   │   ┌──────── GPIO13●  (IR_SEND_PIN)     ● D14 │   │ │
    │     │   │   │     │ GPIO12●  (Available)        ● D13 │   │ │
    │     │   │   │     │ GPIO11●                    ● D12 │   │ │
    │     │   │   │     │ GPIO10●                    ● D11 │   │ │
    │     │   │   │     │ GPIO9 ●                    ● D10 │   │ │
    │     │   │   │     │ GPIO8 ●                    ● D9  │   │ │
    │     │   │   │     │ GPIO7 ●                    ● D8  │   │ │
    │     │   │   │     │ GPIO6 ●                    ● D7  │   │ │
    │     │   │   │     │ GPIO5 ●                    ● D6  │   │ │
    │     │   │   │     │ GPIO4 ●                    ● D5  │   │ │
    │     │   │   │     │ GPIO3 ●                    ● D4  │   │ │
    │     │   │   │ ┌────── GPIO22● (OLED_SCL)        ● D3  │   │ │
    │     │   │   │ │   │ 5V    ●                    ● D22 │ ──┘ │ SCL (I2C)
    │     │   │   │ │   │ GND   ●                    ● GND │ ────┘
    │     │   │   │ │   └─────────────────────────────────┘
    │     │   │   │ │
    │     │   │   │ └─────────────────────────────────────┐
    │     │   │   │                                       │
    │     │   │   └─────────────────────┐                 │
    │     │   │                         │                 │
    │     │   │      IR LED Driver      │                 │
    │     │   │     ┌─────────────┐     │                 │
    │     │   └─────┤ 220Ω        │     │                 │
    │     │         │             │     │                 │
    │     │         │    ┌────────┴──┐  │                 │
    │     │         │    │IR LED(940nm│  │                 │
    │     │         │    │     ▲     │  │                 │
    │     │         │    │     │     │  │                 │
    │     │         └────┴─────┼─────┘  │                 │
    │     │                    │        │                 │
    │     └────────────────────┴────────┘                 │
    │                          GND                        │
    │                                                     │
    │              I2C Bus (Shared)                       │
    │         ┌─────────┬─────────────────────────────────┘
    │         │         │
    │    ┌────▼────┐    │         ┌────────────────┐
    │    │ 4.7kΩ   │    │         │ 4.7kΩ          │
    │    │Pull-up  │    │         │Pull-up         │
    │    │         │    │         │                │
┌───▼────┴─────────▼────▼─────┐   │   ┌────────────▼──────────┐
│        SHT31 Sensor         │   │   │     OLED Display      │
│   ┌─────────────────────┐   │   │   │  ┌─────────────────┐  │
│   │  ●VCC  ●GND  ●SDA●SCL│   │   │   │  │ 128 x 32 pixels │  │
│   └─────────────────────┘   │   │   │  │   SSD1306       │  │
│    │      │     │    │      │   │   │  │                 │  │
└────┼──────┼─────┼────┼──────┘   │   │  └─────────────────┘  │
     │      │     │    └──────────┼───┘  │ ●VCC ●GND ●SDA●SCL │
     │      │     └───────────────┼──────┴─────────────────────┘
     │      │                     │              │   │    │
     │      └─────────────────────┼──────────────┘   │    │
     │                            │                  │    │
     └────────────────────────────┼──────────────────┘    │
                                  │                       │
                            ┌─────▼─────┐           ┌─────▼─────┐
                            │    3.3V   │           │    GND    │
                            │   Power   │           │  Common   │
                            │   Rail    │           │   Ground  │
                            └───────────┘           └───────────┘
```

## Detailed Connection Tables

### Power Connections
| ESP32-S3 Pin | Connection | Notes |
|--------------|------------|-------|
| 5V | Power Supply + | 5V input (USB or external) |
| 3V3 | Sensor VCC | 3.3V output for sensors |
| GND | Common Ground | Multiple GND connections available |

### I2C Bus Connections (Shared Bus)
| ESP32-S3 Pin | Signal | SHT31 Pin | OLED Pin | Pull-up Resistor |
|--------------|--------|-----------|-----------|------------------|
| GPIO21 | SDA | SDA | SDA | 4.7kΩ to 3.3V |
| GPIO22 | SCL | SCL | SCL | 4.7kΩ to 3.3V |
| 3V3 | VCC | VCC | VCC | - |
| GND | GND | GND | GND | - |

### IR Transmitter Circuit
| ESP32-S3 Pin | Component | Connection | Notes |
|--------------|-----------|------------|-------|
| GPIO13 | 220Ω Resistor | Anode of IR LED | Current limiting |
| GND | IR LED | Cathode | Complete circuit |

## I2C Device Addresses
| Device | I2C Address | Notes |
|--------|-------------|-------|
| SHT31 Temperature Sensor | 0x44 | Default address |
| OLED Display (SSD1306) | 0x3C | Common default address |

## Power Requirements
| Component | Voltage | Current | Power |
|-----------|---------|---------|-------|
| ESP32-S3 | 3.3V | ~240mA | ~0.8W |
| SHT31 Sensor | 3.3V | ~1.5mA | ~5mW |
| OLED Display | 3.3V | ~20mA | ~66mW |
| IR LED | 3.3V | ~100mA | ~330mW |
| **Total** | **5V Input** | **~361mA** | **~1.2W** |

## Pin Usage Summary
| Used Pins | Available Pins | Reserved Pins |
|-----------|----------------|---------------|
| 3 pins used | GPIO12, GPIO14 + others | GPIO0, GPIO46 (strapping) |
| GPIO13 (IR) | Many GPIO available | GPIO45, GPIO47 (strapping) |
| GPIO21 (SDA) | for expansion | USB pins (GPIO19-20) |
| GPIO22 (SCL) | | |

## Assembly Instructions

1. **Power Setup**: Connect ESP32-S3 to 5V power supply
2. **I2C Bus**: Wire GPIO21 (SDA) and GPIO22 (SCL) with 4.7kΩ pull-up resistors
3. **SHT31 Sensor**: Connect to I2C bus (address 0x44)
4. **OLED Display**: Connect to I2C bus (address 0x3C)
5. **IR LED**: Connect to GPIO13 with 220Ω current limiting resistor
6. **Ground**: Ensure all GND connections are common

## Software Configuration

The pin definitions are configured in `include/config.h`:
```cpp
#define IR_SEND_PIN 13      // GPIO13 - IR Transmitter
#define OLED_SDA 21         // GPIO21 - SDA for OLED
#define OLED_SCL 22         // GPIO22 - SCL for OLED
```

## Testing Checklist

- [ ] Power supply provides stable 5V
- [ ] I2C devices detected at correct addresses
- [ ] SHT31 reading valid temperature/humidity
- [ ] OLED display showing information
- [ ] IR LED transmitting (use camera to verify)
- [ ] WiFi connection established
- [ ] Web interface accessible
- [ ] Rule-based AC control functional

## Troubleshooting

### I2C Issues
- Check pull-up resistors (4.7kΩ)
- Verify device addresses with I2C scanner
- Ensure proper wiring and connections

### IR Transmission Issues  
- Use smartphone camera to check IR LED operation
- Verify current limiting resistor value
- Check GPIO13 PWM output with oscilloscope

### Power Issues
- Measure voltage levels at each component
- Check for loose connections
- Verify power supply capacity

## Safety Notes

⚠️ **Important Safety Guidelines:**
- Use appropriate current limiting resistors
- Verify all connections before powering on
- Keep IR LED current below maximum rating
- Ensure proper grounding of all components
- Use quality power supply with proper regulation

## Future Expansion Options

Available GPIO pins for additional features:
- **GPIO12**: General purpose I/O
- **GPIO14**: General purpose I/O
- **GPIO15-16**: ADC capable pins
- **GPIO17-18**: Additional I/O
- **Multiple other GPIOs**: Available for sensors, relays, etc.

Possible expansions:
- Room occupancy sensor (PIR)
- Light sensor (photoresistor/LDR)
- Relay for fan control
- Additional temperature sensors
- External LED status indicators
- Wind sensor (if needed in future)
