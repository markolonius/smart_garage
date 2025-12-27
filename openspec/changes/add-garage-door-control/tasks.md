# Implementation Tasks: Add Garage Door Control with Matter/Thread

## 1. Development Environment Setup

- [ ] 1.1 Install ESP-IDF v5.1+ (verify with `idf.py --version`)
- [ ] 1.2 Clone ESP-Matter SDK repository (`git clone --recursive https://github.com/espressif/esp-matter.git`)
- [ ] 1.3 Set up ESP-Matter environment (source `esp-matter/export.sh`)
- [ ] 1.4 Verify ESP32-H2 target support (`idf.py set-target esp32h2`)
- [ ] 1.5 Install chip-tool for Matter testing (`cd esp-matter/connectedhomeip/connectedhomeip && scripts/examples/gn_build_example.sh examples/chip-tool out/host`)

**Validation**: Run `idf.py build` on Hello World to verify environment, check Matter examples compile successfully

---

## 2. Project Structure and Build Configuration

- [ ] 2.1 Create component directories: `mkdir -p components/{garage_door,sensors,matter_bridge,storage}`
- [ ] 2.2 Create component CMakeLists.txt for each component (register with `idf_component_register()`)
- [ ] 2.3 Update root CMakeLists.txt to include new components
- [ ] 2.4 Configure sdkconfig for Matter/Thread:
  - [ ] Enable IEEE 802.15.4 support (`CONFIG_IEEE802154_ENABLED=y`)
  - [ ] Enable BLE (`CONFIG_BT_ENABLED=y`, `CONFIG_BT_BLE_ENABLED=y`)
  - [ ] Enable NVS encryption (`CONFIG_NVS_ENCRYPTION=y`)
  - [ ] Set Matter vendor/product IDs (via `idf.py menuconfig`)
- [ ] 2.5 Add ESP-Matter dependencies to main/CMakeLists.txt (`PRIV_REQUIRES esp_matter`)

**Validation**: `idf.py build` succeeds with new component structure, Matter/Thread features enabled in sdkconfig

---

## 3. Hardware Interface Component (GPIO Drivers)

### 3.1 Reed Switch Driver

- [ ] 3.1.1 Create `components/sensors/reed_switch.h` with public API
- [ ] 3.1.2 Implement `reed_switch_init()` with GPIO configuration (input, pull-up, interrupts)
- [ ] 3.1.3 Implement GPIO ISR for edge detection (both rising/falling)
- [ ] 3.1.4 Implement debouncing logic (50ms timer, task context)
- [ ] 3.1.5 Implement `reed_switch_read_state()` for synchronous read
- [ ] 3.1.6 Implement `reed_switch_register_callback()` for async notifications
- [ ] 3.1.7 Implement `reed_switch_get_position()` combining both switches
- [ ] 3.1.8 Add NVS support for GPIO pin configuration
- [ ] 3.1.9 Add thread safety (mutex for state access)

**Validation**: Unit test with manual reed switch triggers, verify debouncing, check ISR execution time <10µs

### 3.2 Relay Control Driver

- [ ] 3.2.1 Create `components/sensors/relay_control.h` with public API
- [ ] 3.2.2 Implement `relay_init()` with GPIO configuration (output, pull-down, initial LOW)
- [ ] 3.2.3 Implement `relay_activate_pulse()` with FreeRTOS timer (500ms default)
- [ ] 3.2.4 Implement fail-safe timeout (force LOW after 600ms)
- [ ] 3.2.5 Implement rate limiting (1 second minimum between pulses)
- [ ] 3.2.6 Add NVS support for pulse duration and rate limit configuration
- [ ] 3.2.7 Add thread safety (mutex for relay state)
- [ ] 3.2.8 Implement concurrent activation prevention (EBUSY return code)

**Validation**: Hardware test with oscilloscope (verify pulse duration ±10ms), test rate limiting, verify fail-safe triggers at 600ms

---

## 4. Storage Component (NVS Management)

- [ ] 4.1 Create `components/storage/storage_manager.h` with public API
- [ ] 4.2 Implement `storage_init()` with NVS partition initialization
- [ ] 4.3 Implement GPIO configuration storage (`storage_save_gpio_config()`, `storage_load_gpio_config()`)
- [ ] 4.4 Implement door state persistence (`storage_save_door_state()`, `storage_load_door_state()`)
- [ ] 4.5 Implement event logging (`storage_log_event()`, `storage_get_logs()`)
- [ ] 4.6 Implement circular buffer for event logs (max 100 events)
- [ ] 4.7 Add factory reset support (`storage_factory_reset()`)
- [ ] 4.8 Enable NVS encryption for Thread credentials

**Validation**: Test NVS write/read with power cycle, verify encryption, test circular buffer overflow, validate factory reset

---

## 5. Garage Door Control Component (State Machine)

- [ ] 5.1 Create `components/garage_door/garage_door_control.h` with public API
- [ ] 5.2 Define state machine types (`door_state_t` enum: CLOSED, OPENING, OPEN, CLOSING, STOPPED)
- [ ] 5.3 Implement `garage_door_init()` with state recovery from reed switches
- [ ] 5.4 Implement state machine transition logic (`validate_transition()`, `update_state()`)
- [ ] 5.5 Implement `garage_door_open()` API:
  - [ ] Validate state transition (CLOSED → OPENING)
  - [ ] Activate relay pulse
  - [ ] Start operation timeout timer (30s default)
  - [ ] Monitor reed switches for completion
- [ ] 5.6 Implement `garage_door_close()` API (similar to open)
- [ ] 5.7 Implement `garage_door_stop()` API (transition to STOPPED state)
- [ ] 5.8 Implement `garage_door_get_state()` API
- [ ] 5.9 Implement operation timeout handler (transition to STOPPED, log event)
- [ ] 5.10 Implement obstruction detection (unexpected reed switch during operation)
- [ ] 5.11 Add FreeRTOS task for safety monitoring (priority 7, 10ms loop)
- [ ] 5.12 Add thread safety (mutex for state machine access)
- [ ] 5.13 Integrate with storage component for event logging

**Validation**: Unit tests for all state transitions, timeout scenarios, obstruction detection; verify mutex prevents race conditions

---

## 6. Matter Bridge Component (Matter/Thread Integration)

### 6.1 Matter Device Setup

- [ ] 6.1.1 Create `components/matter_bridge/matter_device.h` with public API
- [ ] 6.1.2 Initialize Matter stack in `matter_device_init()` (based on ESP-Matter examples)
- [ ] 6.1.3 Configure device identification (vendor ID, product ID, serial number)
- [ ] 6.1.4 Set up root endpoint (endpoint 0) with basic information cluster
- [ ] 6.1.5 Create endpoint 1 for Window Covering cluster
- [ ] 6.1.6 Configure Window Covering cluster attributes:
  - [ ] Type (Rollershade or Drapery)
  - [ ] CurrentPositionLiftPercent100ths (0-10000)
  - [ ] TargetPositionLiftPercent100ths (0-10000)
  - [ ] OperationalStatus (Lift Moving/Stopped)
- [ ] 6.1.7 Register Window Covering command handlers (UpOrOpen, DownOrClose, StopMotion)

**Validation**: Verify Matter device descriptor with chip-tool, check cluster list on endpoint 1

### 6.2 Thread Network Configuration

- [ ] 6.2.1 Initialize Thread stack via ESP-Matter APIs
- [ ] 6.2.2 Configure Thread device role as End Device (non-Router)
- [ ] 6.2.3 Implement Thread network join logic with credential loading from NVS
- [ ] 6.2.4 Add Thread network loss recovery (retry for 5 minutes)
- [ ] 6.2.5 Implement IPv6 address assignment and validation

**Validation**: Verify Thread network join with border router (check with `ot-cli state`, IPv6 address assignment)

### 6.3 BLE Commissioning

- [ ] 6.3.1 Enable BLE advertising when not commissioned
- [ ] 6.3.2 Generate QR code payload for Matter commissioning
- [ ] 6.3.3 Implement PASE (Password Authenticated Session Establishment)
- [ ] 6.3.4 Handle Thread credential provisioning during commissioning
- [ ] 6.3.5 Disable BLE after successful commissioning
- [ ] 6.3.6 Re-enable BLE on factory reset

**Validation**: Commission device via chip-tool (`chip-tool pairing ble-thread ...`), verify QR code scanning with Google Home/Apple Home

### 6.4 Window Covering Cluster Logic

- [ ] 6.4.1 Implement UpOrOpen command handler:
  - [ ] Call `garage_door_open()`
  - [ ] Update OperationalStatus to "Lift Opening"
  - [ ] Update CurrentPositionLiftPercent100ths when complete
- [ ] 6.4.2 Implement DownOrClose command handler (similar to UpOrOpen)
- [ ] 6.4.3 Implement StopMotion command handler (call `garage_door_stop()`)
- [ ] 6.4.4 Implement attribute update callback from door state changes:
  - [ ] CLOSED → CurrentPositionLiftPercent100ths = 0 (0%)
  - [ ] OPEN → CurrentPositionLiftPercent100ths = 10000 (100%)
  - [ ] OPENING/CLOSING → OperationalStatus reflects movement
- [ ] 6.4.5 Add attribute subscription support (min 1s, max 10s intervals)

**Validation**: Control door via chip-tool/Matter app, verify attribute updates match door state, test subscriptions

### 6.5 Matter Security and Diagnostics

- [ ] 6.5.1 Implement device attestation with Espressif PAA
- [ ] 6.5.2 Configure Access Control Lists (ACL) for multi-controller support
- [ ] 6.5.3 Add Matter event logging (commission, command, disconnect events)
- [ ] 6.5.4 Implement fail-safe timer for commissioning (60s timeout)
- [ ] 6.5.5 Add Matter diagnostics (Thread network metrics, session info)

**Validation**: Verify attestation during commissioning, test ACL with multiple controllers, query diagnostics via chip-tool

---

## 7. Main Application Integration

- [ ] 7.1 Replace `main/hello_world_main.c` with `main/garage_main.c`
- [ ] 7.2 Implement `app_main()`:
  - [ ] Initialize NVS (`nvs_flash_init()`)
  - [ ] Initialize storage component
  - [ ] Initialize GPIO drivers (reed switches, relay)
  - [ ] Initialize garage door control component
  - [ ] Initialize Matter device and start commissioning
  - [ ] Start FreeRTOS tasks (safety monitoring, Matter event loop)
- [ ] 7.3 Add status LED control (optional):
  - [ ] Solid: door stationary
  - [ ] Fast blink: door moving
  - [ ] Slow blink: not commissioned
- [ ] 7.4 Implement graceful shutdown on critical errors
- [ ] 7.5 Update main/CMakeLists.txt with component dependencies

**Validation**: Full system boot test, verify initialization sequence, check FreeRTOS task creation

---

## 8. Testing and Validation

### 8.1 Unit Tests

- [ ] 8.1.1 Create pytest test suite `test/pytest_garage_door.py`
- [ ] 8.1.2 Test state machine transitions (all valid and invalid cases)
- [ ] 8.1.3 Test reed switch debouncing and edge cases
- [ ] 8.1.4 Test relay timing and fail-safe
- [ ] 8.1.5 Test NVS persistence and recovery
- [ ] 8.1.6 Test thread safety (concurrent API calls)

**Validation**: All unit tests pass (`pytest test/pytest_garage_door.py -v`)

### 8.2 Hardware Integration Tests

- [ ] 8.2.1 Test reed switches with physical magnets (verify debouncing)
- [ ] 8.2.2 Test relay activation with oscilloscope (measure pulse duration)
- [ ] 8.2.3 Test complete open/close cycle with real door
- [ ] 8.2.4 Test timeout protection (block door mid-operation)
- [ ] 8.2.5 Test obstruction detection (trigger wrong reed switch)
- [ ] 8.2.6 Test power loss recovery (power cycle during operation)

**Validation**: Document test results with photos/oscilloscope captures

### 8.3 Matter/Thread Integration Tests

- [ ] 8.3.1 Test BLE commissioning with chip-tool
- [ ] 8.3.2 Test Thread network join and IPv6 connectivity
- [ ] 8.3.3 Test Matter command latency (<500ms end-to-end)
- [ ] 8.3.4 Test attribute subscriptions and real-time updates
- [ ] 8.3.5 Test border router failover (disconnect border router)
- [ ] 8.3.6 Test multi-controller support (commission with Google Home + Apple Home)
- [ ] 8.3.7 Test factory reset and recommissioning

**Validation**: All Matter tests pass, device appears correctly in smart home apps

### 8.4 Safety and Reliability Tests

- [ ] 8.4.1 Test operation timeout (manually block door, verify 30s stop)
- [ ] 8.4.2 Test obstruction detection (trigger reed switches manually)
- [ ] 8.4.3 Test rapid command sequences (verify rate limiting)
- [ ] 8.4.4 Test network disconnection during operation (verify local control)
- [ ] 8.4.5 Stress test (1000 open/close cycles, check for memory leaks)
- [ ] 8.4.6 Test concurrent Matter commands from multiple controllers

**Validation**: No safety failures, system remains stable under stress

---

## 9. Documentation and Code Quality

- [ ] 9.1 Run clang-format on all source files (`clang-format -i components/**/*.c components/**/*.h main/*.c`)
- [ ] 9.2 Run clang-tidy static analysis and fix warnings
- [ ] 9.3 Add Doxygen comments to all public APIs
- [ ] 9.4 Create `docs/HARDWARE_SETUP.md` (wiring diagrams, pin assignments)
- [ ] 9.5 Create `docs/COMMISSIONING.md` (step-by-step commissioning guide)
- [ ] 9.6 Create `docs/TROUBLESHOOTING.md` (common issues, debugging tips)
- [ ] 9.7 Update main README.md with project overview and build instructions

**Validation**: Doxygen generates docs without warnings, clang-tidy passes, README is clear and accurate

---

## 10. Final Validation and Release Preparation

- [ ] 10.1 Full system test on clean ESP32-H2 board (flash from scratch)
- [ ] 10.2 Verify commissioning with Google Home (Android)
- [ ] 10.3 Verify commissioning with Apple Home (iOS)
- [ ] 10.4 End-to-end latency test (command to door activation <500ms)
- [ ] 10.5 Memory usage check (verify <200KB SRAM used)
- [ ] 10.6 Flash usage check (verify <2MB flash used for future OTA)
- [ ] 10.7 Create release checklist for future versions
- [ ] 10.8 Tag initial release version (v0.1.0)

**Validation**: Complete system operational, all acceptance criteria met

---

## Dependencies and Parallelization

### Can be done in parallel:
- Tasks 3.1 and 3.2 (reed switch and relay drivers independent)
- Task 4 (storage) can start after project structure (Task 2)
- Task 8.1 (unit tests) can be written alongside implementation

### Sequential dependencies:
- Task 5 depends on Tasks 3 and 4 (needs GPIO drivers and storage)
- Task 6 depends on Task 5 (Matter integration needs door control API)
- Task 7 depends on all component tasks (2-6)
- Task 8.2-8.4 depend on Task 7 (need complete system)
- Task 9 can only be done after implementation complete
- Task 10 depends on all previous tasks

### Critical path:
1. Environment setup (Task 1) → 2. Project structure (Task 2) → 3. Hardware drivers (Task 3) → 5. State machine (Task 5) → 6. Matter integration (Task 6) → 7. Main app (Task 7) → 8. Testing (Task 8) → 10. Release (Task 10)
