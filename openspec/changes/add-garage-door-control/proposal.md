# Change: Add Basic Garage Door Control with Matter/Thread Support

## Why

Enable remote control and monitoring of a garage door through Matter/Thread protocol, providing seamless integration with smart home ecosystems (Apple Home, Google Home, etc.). This establishes the foundation for a complete smart garage system by implementing core door control functionality with safety features.

Currently, the project only contains ESP-IDF "Hello World" boilerplate. This change delivers the first user-facing capability: controlling a Genie garage door opener remotely via Matter-compatible platforms.

## What Changes

- **NEW**: Garage door state machine (closed → opening → open → closing → closed)
- **NEW**: Reed switch driver for door position sensing (open/closed states)
- **NEW**: Relay control interface for Genie garage door opener (500ms pulse)
- **NEW**: Matter Window Covering cluster implementation (up/down → open/close)
- **NEW**: Thread network support with BLE commissioning
- **NEW**: Safety features:
  - Operation timeout protection (auto-stop after configurable duration)
  - Obstruction detection during operation
  - Invalid state transition prevention
  - Fail-safe on communication loss
- **NEW**: NVS-based persistent configuration storage
- **NEW**: FreeRTOS task orchestration for concurrent sensor monitoring and Matter event handling

This change introduces three new capabilities:
1. **garage-door-control**: Core door state machine and control logic
2. **matter-integration**: Matter/Thread networking and Window Covering cluster
3. **hardware-interface**: GPIO drivers for reed switches and relay control

## Impact

### Affected Specs
- **NEW**: `specs/garage-door-control/spec.md` - Door control requirements
- **NEW**: `specs/matter-integration/spec.md` - Matter/Thread networking requirements
- **NEW**: `specs/hardware-interface/spec.md` - Hardware interface requirements

### Affected Code
- `main/hello_world_main.c` → Replaced with garage door application entry point
- `main/CMakeLists.txt` → Updated component dependencies (esp_matter, driver, nvs_flash)
- **NEW**: `components/garage_door/` - Door control logic and state machine
- **NEW**: `components/sensors/` - Reed switch driver
- **NEW**: `components/matter_bridge/` - Matter device implementation
- **NEW**: `components/storage/` - NVS configuration management
- `CMakeLists.txt` → Updated to include new components
- `sdkconfig` → Matter/Thread configuration settings (ESP-Matter SDK, Thread stack)

### Build System Changes
- Add ESP-Matter SDK as dependency (requires ESP-IDF v5.1+)
- Enable IEEE 802.15.4 radio support for Thread
- Configure BLE for Matter commissioning
- Enable NVS partition for persistent storage

### Hardware Requirements
- ESP32-H2 development board (Thread/BLE capable)
- 2x reed switches (door open/closed position detection)
- 1x relay module (5V with optocoupler isolation)
- Genie garage door opener with accessible control terminals
- Stable 3.3V power supply

### Testing Requirements
- Unit tests for state machine logic
- Hardware integration tests with reed switches and relay
- Matter commissioning validation (chip-tool)
- Thread network join validation
- Safety feature verification (timeout, obstruction detection)
- Manual operational testing with physical door

### Migration Notes
This is the first functional implementation - no migration needed. The "Hello World" code will be replaced entirely.
