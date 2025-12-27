<!-- OPENSPEC:START -->
# OpenSpec Instructions

These instructions are for AI assistants working in this project.

Always open `@/openspec/AGENTS.md` when the request:
- Mentions planning or proposals (words like proposal, spec, change, plan)
- Introduces new capabilities, breaking changes, architecture shifts, or big performance/security work
- Sounds ambiguous and you need the authoritative spec before coding

Use `@/openspec/AGENTS.md` to learn:
- How to create and apply change proposals
- Spec format and conventions
- Project structure and guidelines

Keep this managed block so 'openspec update' can refresh the instructions.

<!-- OPENSPEC:END -->

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP-IDF (Espressif IoT Development Framework) project targeting ESP32 microcontrollers and variants. The project uses CMake with ESP-IDF's build system and is configured for minimal builds (only includes essential components).

## Build System Commands

ESP-IDF uses the `idf.py` command-line tool. Ensure `IDF_PATH` environment variable is set before running these commands.

### Basic Build & Flash
```bash
# Full build
idf.py build

# Flash to device (replace PORT with actual serial port, e.g., /dev/ttyUSB0)
idf.py -p PORT flash

# Monitor serial output
idf.py -p PORT monitor

# Build, flash, and monitor in one command
idf.py -p PORT flash monitor
```

### Configuration
```bash
# Open menuconfig TUI for project configuration
idf.py menuconfig

# Clean build artifacts
idf.py fullclean
```

### Development Workflow
```bash
# Build and immediately monitor (useful for iterative development)
idf.py build flash monitor

# Exit monitor: Ctrl+]
```

## Testing

### Automated Testing with pytest
```bash
# Run all tests
pytest pytest_hello_world.py

# Run specific test
pytest pytest_hello_world.py::test_hello_world

# Run with verbose output
pytest -v pytest_hello_world.py
```

Tests use `pytest-embedded-idf` framework and support:
- Hardware testing on actual ESP32 devices
- QEMU emulation testing (`test_hello_world_host`)
- Linux host testing (`test_hello_world_linux`)

## Code Quality Tools

### Clang Format
```bash
# Format all C/C++ files according to .clang-format
clang-format -i main/*.c

# Check formatting without modifying
clang-format --dry-run -Werror main/*.c
```

### Clang Tidy
```bash
# Run static analysis
clang-tidy main/*.c -- -I$IDF_PATH/components/...
```

Configurations are defined in `.clang-format`, `.clang-tidy`, and `.clangd`.

## Architecture & Project Structure

### ESP-IDF Component System
```
smart_garage/
├── CMakeLists.txt          # Top-level CMake (includes ESP-IDF project.cmake)
├── sdkconfig               # Project configuration (generated/modified by menuconfig)
├── main/                   # Main component (required)
│   ├── CMakeLists.txt      # Component registration
│   └── hello_world_main.c  # Entry point (app_main function)
└── build/                  # Build output directory
```

### Component Registration
Components are registered in `main/CMakeLists.txt` using `idf_component_register()`:
- **SRCS**: Source files to compile
- **PRIV_REQUIRES**: Private component dependencies (e.g., `spi_flash`)
- **INCLUDE_DIRS**: Include directories for the component

### Minimal Build Configuration
This project uses `MINIMAL_BUILD ON` in the top-level CMakeLists.txt, which:
- Only includes explicitly required components
- Reduces build time and binary size
- Requires explicit component dependencies in `PRIV_REQUIRES` or `REQUIRES`

### Application Entry Point
ESP-IDF applications start execution at `app_main()` function (not `main()`). This function:
- Runs in the context of the "main" FreeRTOS task
- Should not return (or should call `esp_restart()` to reboot)
- Can create additional FreeRTOS tasks as needed

### FreeRTOS Integration
- All timing uses FreeRTOS primitives: `vTaskDelay(ms / portTICK_PERIOD_MS)`
- Task creation: `xTaskCreate()` or `xTaskCreatePinnedToCore()` for SMP
- ESP-IDF provides thread-safe wrappers for most peripherals

## Important ESP-IDF Concepts

### sdkconfig
- Project configuration stored in `sdkconfig` (version controlled)
- Modified via `idf.py menuconfig` TUI
- Contains hardware features, WiFi settings, partition tables, etc.
- `sdkconfig.ci` can contain CI-specific overrides

### Target Selection
```bash
# Set target chip (default: esp32)
idf.py set-target esp32c3
idf.py set-target esp32s3

# This regenerates sdkconfig for the new target
```

### Serial Port Detection
```bash
# List available serial ports
idf.py -p PORT monitor  # Will suggest available ports if PORT is wrong

# Common ports:
# - Linux: /dev/ttyUSB0, /dev/ttyACM0
# - macOS: /dev/cu.usbserial-*, /dev/cu.SLAB_USBtoUART
# - Windows: COM3, COM4, etc.
```

## Common Troubleshooting

- **Build errors after pulling changes**: Run `idf.py fullclean` to clear cached state
- **"Port not found" errors**: Check USB cable, drivers (CP210x/CH340), and port permissions
- **"Brownout detector" resets**: Insufficient power supply; use quality USB cable/power source
- **Cannot flash**: Hold BOOT button during flash or check `CONFIG_ESPTOOLPY_BAUD` in menuconfig
