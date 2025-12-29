# Smart Garage Door Controller

A Matter-over-Thread garage door controller for ESP32-H2 that enables smart home integration with Google Home, Apple Home, and other Matter-compatible platforms.

## Features

- **Matter Window Covering Cluster**: Seamless integration with Matter-compatible smart home platforms
- **5-State State Machine**: CLOSED, OPENING, OPEN, CLOSING, STOPPED
- **Safety Features**: 
  - Reed switch debouncing and obstruction detection
  - Operation timeout protection (configurable, default 30s)
  - Relay pulse limiting (500ms default with fail-safe timeout)
  - Rate limiting between activations
- **Thread Networking**: Low-latency, reliable communication via Thread mesh network
- **BLE Commissioning**: Easy onboarding with QR code scanning
- **NVS Storage**: Configuration persistence across power cycles
- **Event Logging**: Diagnostic event history stored in non-volatile memory

## Hardware Requirements

- **Microcontroller**: ESP32-H2 (supports Thread and BLE)
- **Sensors**: 
  - 2x Reed switches (door closed/open position detection)
- **Actuator**: 
  - 1x Relay module (dry contact for Genie garage door opener)
- **Power**: 5V DC power supply
- **Optional**: Status LED for visual feedback

## Quick Start

### Prerequisites

1. Install ESP-IDF v5.1+ following [ESP-IDF Setup Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32h2/get-started/index.html)
2. Clone ESP-Matter SDK: `git clone --recursive https://github.com/espressif/esp-matter.git`
3. Set up ESP-Matter environment: `source esp-matter/export.sh`
4. Connect hardware (see [HARDWARE_SETUP.md](docs/HARDWARE_SETUP.md) for wiring diagram)

### Building

```bash
idf.py set-target esp32h2
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### Commissioning

1. Power on device - BLE advertising will start automatically
2. Scan QR code with Google Home, Apple Home, or chip-tool
3. Follow pairing process for your platform
4. Device will join Thread network and begin reporting door state

See [COMMISSIONING.md](docs/COMMISSIONING.md) for detailed commissioning guide.

## Project Structure

```
smart_garage/
├── CMakeLists.txt              # Top-level build configuration
├── sdkconfig                  # ESP-IDF configuration (auto-generated)
├── components/
│   ├── garage_door/           # State machine and business logic
│   │   ├── garage_door_control.h
│   │   ├── garage_door_control.c
│   │   └── CMakeLists.txt
│   ├── sensors/               # Hardware drivers (reed switches, relay)
│   │   ├── reed_switch.h
│   │   ├── reed_switch.c
│   │   ├── relay_control.h
│   │   ├── relay_control.c
│   │   └── CMakeLists.txt
│   ├── storage/              # NVS wrapper for configuration
│   │   ├── storage_manager.h
│   │   ├── storage_manager.c
│   │   └── CMakeLists.txt
│   └── matter_bridge/       # Matter/Thread integration (stub)
│       ├── matter_device.h
│       ├── matter_device.c
│       └── CMakeLists.txt
├── main/
│   ├── CMakeLists.txt
│   └── garage_main.c         # Application entry point
└── docs/
    ├── HARDWARE_SETUP.md
    ├── COMMISSIONING.md
    └── TROUBLESHOOTING.md
```

## Default Pin Configuration (ESP32-H2-DevKitM-1)

| GPIO | Function | Direction |
|------|----------|-----------|
| 2 | Reed Switch (Closed) | Input (pull-up) |
| 3 | Reed Switch (Open) | Input (pull-up) |
| 4 | Relay Control | Output |
| 8 | Status LED (optional) | Output |

**Note**: GPIO configuration is stored in NVS and can be changed without recompiling via the storage API.

## Safety Features

### Reed Switch Debouncing
- 50ms debounce timer filters mechanical bounce
- Edge-triggered interrupts with task context processing

### Operation Timeout
- Configurable timeout (default 30s)
- Automatically stops door if operation takes too long
- Logs timeout event to NVS

### Obstruction Detection
- Monitors reed switch state during door movement
- Triggers STOPPED state if unexpected state detected
- Requires manual intervention (stop command) to clear

### Relay Fail-Safe
- Maximum pulse duration enforced (600ms)
- Rate limiting (minimum 1s between activations)
- Force LOW after timeout

## API Overview

### Door Control
```c
esp_err_t garage_door_init(void);
esp_err_t garage_door_open(void);
esp_err_t garage_door_close(void);
esp_err_t garage_door_stop(void);
door_state_t garage_door_get_state(void);
```

### Reed Switch
```c
esp_err_t reed_switch_init(const reed_switch_config_t *config);
door_position_t reed_switch_get_position(void);
esp_err_t reed_switch_register_callback(reed_switch_callback_t callback);
```

### Relay
```c
esp_err_t relay_init(gpio_num_t gpio_num);
esp_err_t relay_activate(void);
esp_err_t relay_set_config(const relay_config_t *config);
```

### Storage
```c
esp_err_t storage_init(void);
esp_err_t storage_save_gpio_config(const storage_gpio_config_t *config);
esp_err_t storage_load_gpio_config(storage_gpio_config_t *config);
esp_err_t storage_log_event(event_type_t type, int32_t value);
```

## Troubleshooting

See [TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) for common issues and debugging tips.

## License

SPDX-License-Identifier: CC0-1.0

## Contributing

This project follows the [ESP-IDF Coding Style Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/contribute/style-guide.html). Use clang-format and clang-tidy to maintain code quality.

## Status

**Current Version**: 0.2.0-dev

**Implemented**:
- ✅ Project structure and build configuration
- ✅ Reed switch driver with debouncing
- ✅ Relay control with safety limits
- ✅ NVS storage and configuration
- ✅ 5-state garage door control state machine
- ✅ Main application integration

**In Progress**:
- ⚠️ Matter/Thread integration (ESP-Matter SDK required)
- ⚠️ BLE commissioning
- ⚠️ Testing and validation

**Planned**:
- 📋 Status LED indicators
- 📋 Unit and integration tests
- 📋 Matter certification
- 📋 OTA updates
- 📋 Environmental monitoring (DHT11)
- 📋 Vehicle presence detection (HC-SR04)