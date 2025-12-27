# Project Context

## Purpose
Smart garage door controller and monitoring system built on ESP32-H2 with Matter/Thread support for seamless smart home integration. The system provides:
- Remote garage door control via Matter-compatible smart home platforms
- Real-time door position monitoring (open/closed states)
- Environmental monitoring (temperature and humidity)
- Distance sensing for vehicle presence detection
- Thread mesh networking for reliable wireless communication
- Matter protocol for universal smart home compatibility

## Tech Stack

### Hardware Platform
- **MCU**: ESP32-H2 (RISC-V, Thread/BLE 5.2, Matter-ready)
- **Sensors**:
  - HC-SR04 ultrasonic distance sensor (vehicle detection)
  - DHT11 temperature/humidity sensor (environmental monitoring)
  - Reed switches (door open/closed state detection)
- **Actuator**: Genie garage door opener control interface

### Software Framework
- **ESP-IDF**: Espressif IoT Development Framework (v5.x+)
- **Matter SDK**: ESP-Matter framework for Thread/Matter protocol support
- **FreeRTOS**: Real-time operating system (integrated in ESP-IDF)
- **Build System**: CMake with ESP-IDF build system
- **Language**: C (embedded systems), C++ (optional for Matter components)

### Wireless Protocols
- **Matter**: Application layer protocol for smart home interoperability
- **Thread**: IPv6-based mesh networking (IEEE 802.15.4)
- **BLE**: Bluetooth Low Energy 5.2 (commissioning and backup connectivity)

### Development Tools
- **idf.py**: ESP-IDF command-line interface
- **Clang Format/Tidy**: Code formatting and static analysis
- **pytest-embedded-idf**: Hardware-in-the-loop testing framework
- **Matter commissioning tools**: chip-tool, Google Home, Apple Home

## Project Conventions

### Code Style
- **Formatting**: `.clang-format` configuration (enforced via clang-format)
- **Static Analysis**: `.clang-tidy` rules for code quality
- **Naming Conventions**:
  - Functions: `snake_case` (e.g., `garage_door_open()`)
  - Variables: `snake_case` (e.g., `door_state`, `sensor_reading`)
  - Constants/Macros: `UPPER_SNAKE_CASE` (e.g., `MAX_DISTANCE_CM`, `DHT11_PIN`)
  - Types: `snake_case_t` suffix (e.g., `door_state_t`, `sensor_config_t`)
- **Comments**: Doxygen-style for public APIs, inline for complex logic
- **File Organization**:
  - Header guards: `#ifndef COMPONENT_NAME_H` / `#define COMPONENT_NAME_H`
  - Include order: System headers → ESP-IDF headers → Project headers

### Architecture Patterns

#### Component-Based Architecture
```
smart_garage/
├── components/
│   ├── garage_door/       # Door control logic and state machine
│   ├── sensors/           # HC-SR04, DHT11, reed switch drivers
│   ├── matter_bridge/     # Matter device implementation
│   └── storage/           # NVS for persistent configuration
├── main/                  # Application entry point and orchestration
└── test/                  # Unit and integration tests
```

#### Design Patterns
- **State Machine**: Garage door controller (closed → opening → open → closing → closed)
- **Observer Pattern**: Sensor data publishing to Matter endpoints
- **Component Registration**: ESP-IDF component system for modular design
- **Event-Driven**: FreeRTOS tasks and event groups for asynchronous operations

#### Thread Safety
- All shared resources protected with FreeRTOS mutexes/semaphores
- Thread-safe sensor reading with atomic operations where applicable
- Matter thread context managed by ESP-Matter framework

### Testing Strategy

#### Hardware Testing
- **pytest-embedded-idf**: Automated testing on real ESP32-H2 hardware
- **QEMU Emulation**: Host-based testing for logic validation (where applicable)
- **Manual Testing**: Physical sensor validation and door operation safety tests

#### Test Coverage
- **Unit Tests**: Individual component functionality (sensor drivers, state machine)
- **Integration Tests**: Matter commissioning, Thread network joining
- **Regression Tests**: CI/CD validation on each commit
- **Safety Tests**: Door obstruction handling, sensor failure modes

#### Test Execution
```bash
# Run all tests
pytest test/pytest_*.py

# Run specific test suite
pytest test/pytest_garage_door.py -v

# Hardware-specific tests (requires device connection)
pytest test/pytest_matter_commissioning.py --port /dev/ttyUSB0
```

### Git Workflow
- **Branching Strategy**:
  - `main`: Stable releases only
  - `develop`: Integration branch for features
  - `feature/*`: Individual feature development
  - `fix/*`: Bug fixes
  - `matter/*`: Matter/Thread protocol updates
- **Commit Conventions**: Conventional Commits (e.g., `feat: add HC-SR04 driver`, `fix: door state race condition`)
- **PR Requirements**:
  - All tests passing
  - clang-format compliance
  - Code review approval
  - Changelog update for user-facing changes

## Domain Context

### Garage Door Safety
- **Obstruction Detection**: Monitor sensor readings during door operation
- **Manual Override**: Physical button takes precedence over network commands
- **Fail-Safe**: Door stops if communication lost or sensor failure detected
- **Debouncing**: Reed switch and button inputs debounced (50ms minimum)

### Matter/Thread Networking
- **Commissioning**: BLE-based onboarding to Thread network
- **Device Types**: Matter Door Lock cluster (adapted for garage door)
- **Endpoints**:
  - EP1: Garage door control (lock/unlock → close/open)
  - EP2: Temperature/humidity sensor
  - EP3: Occupancy sensor (vehicle presence)
- **Thread Network**: Border router required (e.g., Apple HomePod, Google Nest Hub)

### Sensor Characteristics
- **HC-SR04**:
  - Range: 2cm - 400cm
  - Trigger: 10µs pulse on TRIG pin
  - Echo: Pulse width proportional to distance
  - Accuracy: ±3mm
- **DHT11**:
  - Temperature: 0-50°C (±2°C accuracy)
  - Humidity: 20-90% RH (±5% accuracy)
  - Read interval: Minimum 1 second
- **Reed Switches**:
  - Normally open contacts
  - Magnet proximity: <20mm activation
  - Debouncing required

### Genie Garage Door Opener Interface
- **Control Method**: Dry contact relay closure (simulate button press)
- **Pulse Duration**: 500ms relay activation
- **Safety**: Isolated from ESP32 power domain via optocoupler/relay

## Important Constraints

### Hardware Limitations
- **ESP32-H2 Specifics**:
  - No Wi-Fi (Thread/BLE only)
  - Limited GPIO pins (plan pin assignment carefully)
  - Single-core RISC-V (no dual-core task pinning)
  - 320KB SRAM, 4MB flash (typical)
- **Power Supply**: Stable 3.3V required; consider power consumption for battery backup
- **GPIO Allocation**:
  - HC-SR04: 2 pins (TRIG, ECHO)
  - DHT11: 1 pin (DATA)
  - Reed switches: 2 pins (top/bottom position)
  - Relay control: 1 pin (door trigger)
  - Reserved: UART (programming), JTAG (debugging)

### Regulatory Constraints
- **Thread Certification**: Required for commercial products
- **Matter Certification**: Required for official Matter logo usage
- **Safety Standards**: UL 325 compliance for garage door operators (if commercial)
- **RF Compliance**: FCC/CE certification for 2.4GHz operation

### Performance Requirements
- **Response Time**: <500ms from Matter command to door activation
- **Sensor Polling**:
  - Distance: 200ms intervals during operation
  - Temperature/Humidity: 30-second intervals
  - Reed switches: Interrupt-driven (immediate)
- **Thread Latency**: <100ms for local network communication
- **Battery Backup**: Optional UPS for power outage operation

## External Dependencies

### ESP-IDF Components
- `esp_matter`: Matter protocol stack
- `esp_thread`: OpenThread stack (via Matter)
- `driver`: GPIO, RMT, DHT driver, timer
- `nvs_flash`: Non-volatile storage for configuration
- `bt`: Bluetooth LE for Matter commissioning

### Matter Controller Requirements
- **Border Router**: Thread-enabled device (Apple HomePod Mini, Google Nest Hub, etc.)
- **Commissioning App**: Google Home, Apple Home, or chip-tool (development)
- **Network**: IPv6-capable local network for Matter over Thread

### Development Dependencies
- **ESP-IDF**: Version 5.1+ (Matter support requires recent versions)
- **ESP-Matter SDK**: Espressif Matter SDK repository
- **Python**: 3.8+ for idf.py and pytest
- **Tools**:
  - esptool (flashing)
  - openocd (debugging)
  - chip-tool (Matter testing)

### Cloud Services (Optional)
- **OTA Updates**: ESP RainMaker or custom HTTPS server for firmware updates
- **Data Logging**: InfluxDB, MQTT broker for historical sensor data
- **Voice Assistants**: Google Assistant, Amazon Alexa, Apple Siri (via Matter)

### Hardware Suppliers
- **ESP32-H2 Modules**: ESP32-H2-DevKitM-1 (development), custom PCB (production)
- **Sensors**: HC-SR04, DHT11, reed switches (standard electronic components)
- **Power Supply**: 5V → 3.3V regulator (AMS1117, LM1117, etc.)
- **Relay Module**: 5V relay with optocoupler isolation
