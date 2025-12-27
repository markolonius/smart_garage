# Design: Basic Garage Door Control with Matter/Thread

## Context

This is the foundational implementation for an ESP32-H2 smart garage system. The ESP32-H2 is a RISC-V single-core microcontroller with Thread/BLE support but no Wi-Fi, making Matter over Thread the optimal protocol choice for smart home integration.

### Constraints
- ESP32-H2 hardware: Single-core RISC-V, 320KB SRAM, 4MB flash, no Wi-Fi
- Genie garage door opener: Dry contact relay interface (500ms pulse)
- Safety-critical application: Door operation must never compromise user safety
- Matter certification path: Code must align with Matter spec for future certification

### Stakeholders
- End users: Expect reliable, safe garage door control via smart home apps
- Integration partners: Google Home, Apple Home require Matter compliance
- Safety: Must prevent door damage and user injury

## Goals / Non-Goals

### Goals
- ✅ Implement Matter Window Covering cluster for garage door control
- ✅ Support Thread networking with BLE commissioning
- ✅ Provide reliable door state tracking via reed switches
- ✅ Ensure safety through timeout protection and obstruction detection
- ✅ Enable remote control via Matter-compatible smart home platforms
- ✅ Maintain responsive operation (<500ms command-to-action latency)

### Non-Goals
- ❌ Environmental monitoring (temperature/humidity) - deferred to future change
- ❌ Vehicle presence detection (HC-SR04) - deferred to future change
- ❌ Mobile app development - relying on existing Matter apps
- ❌ Cloud connectivity - local Matter/Thread network only
- ❌ OTA updates - deferred to future change
- ❌ Historical data logging - deferred to future change

## Decisions

### Decision 1: Matter Window Covering Cluster vs Door Lock Cluster

**Choice**: Window Covering cluster (Type 514)

**Rationale**:
- Window Covering cluster is designed for garage doors in Matter 1.0 spec
- Provides lift/tilt control mapping naturally to open/close operations
- Includes position state (0% = closed, 100% = open) for future enhancement
- Door Lock cluster implies security device, which has different UX expectations in smart home apps

**Alternatives Considered**:
- Door Lock cluster: Semantically incorrect; garage doors aren't security locks
- Custom cluster: Requires Matter certification complexity and reduces interoperability

### Decision 2: State Machine Architecture

**Choice**: Explicit 5-state FSM (closed, opening, open, closing, stopped)

**States**:
```
CLOSED ──[open cmd]──> OPENING ──[fully open]──> OPEN
   ↑                      ↓                        ↓
   └───[close cmd]── CLOSING ←────[close cmd]─────┘
              ↕
          STOPPED (obstruction/timeout)
```

**Rationale**:
- Clear state transitions prevent race conditions
- Explicit "stopped" state for safety scenarios (obstruction, timeout)
- Reed switches provide ground truth for OPEN/CLOSED states
- Transitional states (OPENING/CLOSING) tracked via timer + safety monitoring

**Alternatives Considered**:
- Binary state (open/closed): Insufficient for tracking in-progress operations
- Position-based (0-100%): Requires position sensor (future HC-SR04 integration)

### Decision 3: Safety Implementation Strategy

**Choice**: Multi-layered safety with hardware + software enforcement

**Layers**:
1. **Hardware Safety**: Reed switches as ground truth, relay pulse limitation (500ms max)
2. **Software Safety**: Operation timeout (configurable, default 30s), state validation
3. **Obstruction Detection**: Monitor reed switch changes during operation (immediate stop if unexpected)
4. **Fail-Safe**: Watchdog timer, communication loss detection (stop operation if Matter connection lost)

**Rationale**:
- Defense in depth: Multiple independent safety mechanisms
- Hardware enforces physical limits (reed switches, relay timeout)
- Software enforces logical safety (state machine, operation timeout)
- Aligns with UL 325 safety principles (commercial garage door standard)

**Alternatives Considered**:
- Software-only safety: Insufficient for safety-critical application
- Rely on Genie opener's built-in safety: Adds defense-in-depth, not replacement

### Decision 4: Thread Network Topology

**Choice**: End Device role with sleepy mode disabled

**Rationale**:
- End Device: Simplest Thread role, relies on border router for routing
- No sleep mode: Garage door must respond immediately to commands (<500ms latency)
- Border router dependency acceptable: Smart home ecosystems provide Thread border routers

**Alternatives Considered**:
- Router role: Unnecessary complexity for single-endpoint device
- Sleepy End Device: Incompatible with <500ms response time requirement

### Decision 5: Component Architecture

**Choice**: 4-component modular design

**Components**:
1. **garage_door**: State machine, business logic, safety orchestration
2. **sensors**: Reed switch driver, debouncing, interrupt handling
3. **matter_bridge**: Matter device, Window Covering cluster, Thread networking
4. **storage**: NVS wrapper for configuration (GPIO pins, timeouts, calibration)

**Rationale**:
- Separation of concerns: Hardware drivers isolated from business logic
- Testability: Each component can be unit tested independently
- ESP-IDF convention: Component-based architecture with CMake registration
- Future extensibility: Add DHT11, HC-SR04 as new components without refactoring

**Alternatives Considered**:
- Monolithic main.c: Poor testability and maintainability
- Single "hardware" component: Couples unrelated drivers (sensors, relay, future additions)

### Decision 6: GPIO Pin Assignment Strategy

**Choice**: Runtime configuration via NVS with compile-time defaults

**Default Pin Mapping** (ESP32-H2-DevKitM-1):
```
GPIO 2  → Reed switch (door closed position)
GPIO 3  → Reed switch (door open position)
GPIO 4  → Relay control (Genie trigger)
GPIO 8  → Status LED (optional, visual feedback)
```

**Rationale**:
- NVS configuration: Enables pin reassignment without recompiling (field customization)
- Compile-time defaults: Sane defaults for ESP32-H2 dev board
- Reserved pins avoided: UART (GPIO 16/17), JTAG (GPIO 0/1/4/5), strapping pins

**Alternatives Considered**:
- Hard-coded pins: Inflexible for custom PCB designs
- Menuconfig only: Requires recompilation for pin changes

## Component Interaction Flow

### Initialization Sequence
```
1. app_main() starts
2. NVS initialization (load config)
3. GPIO driver initialization (sensors, relay)
4. State machine initialization (read reed switches → determine initial state)
5. Matter device initialization
6. Thread network commissioning (BLE) / join existing network
7. Start monitoring tasks:
   - Reed switch monitoring (interrupt-driven)
   - Safety watchdog (periodic check)
   - Matter event handler
```

### Door Operation Flow
```
1. Matter command received (Window Covering UpOrOpen/DownOrClose)
2. matter_bridge validates state, calls garage_door API
3. garage_door state machine validates transition
4. Activate relay (500ms pulse via GPIO)
5. Update state to OPENING/CLOSING
6. Start safety timer (30s timeout)
7. Monitor reed switches:
   - If target reed switch triggers → state = OPEN/CLOSED, stop timer
   - If timeout expires → state = STOPPED, safety alert
   - If unexpected reed switch → obstruction, state = STOPPED
8. Report state change to Matter (Window Covering current position attribute)
```

### Safety Event Flow
```
Obstruction detected (unexpected reed switch):
1. Immediately set state = STOPPED
2. Log safety event to NVS
3. Send Matter alert (optional: future notification capability)
4. Require manual intervention (stop command from Matter or physical button)

Timeout detected:
1. Set state = STOPPED
2. Log timeout event to NVS
3. User must diagnose issue (door jammed, reed switch failure)
```

## Risks / Trade-offs

### Risk 1: Matter Certification Complexity
- **Risk**: Matter certification may require spec changes
- **Mitigation**: Follow ESP-Matter SDK examples closely, align with Window Covering cluster spec
- **Fallback**: Initial release without Matter logo, add certification in future update

### Risk 2: Reed Switch Reliability
- **Risk**: Reed switches may fail (magnet displacement, wear)
- **Mitigation**: Debouncing (50ms), redundant safety (timeout), graceful degradation
- **Monitoring**: Log reed switch transition events for diagnostics

### Risk 3: Thread Network Dependency
- **Risk**: Border router failure breaks functionality
- **Mitigation**: Local state tracking (door still responds to last command), visual LED feedback
- **Future**: Add fallback BLE control mode (out of scope for this change)

### Risk 4: Power Loss During Operation
- **Risk**: Door stuck mid-operation if power lost
- **Mitigation**: Genie opener's built-in safety (stops on power loss), reed switches provide state recovery on reboot
- **Future**: Battery backup support (out of scope)

### Risk 5: Single-Core RISC-V Performance
- **Risk**: Matter stack + FreeRTOS + sensor monitoring may strain single-core CPU
- **Mitigation**: Task priority tuning (safety monitoring = highest), profiling during testing
- **Monitoring**: Add CPU usage metrics to diagnostics

## Migration Plan

**N/A** - This is the initial implementation. The "Hello World" boilerplate will be completely replaced.

### Steps
1. Backup existing `main/hello_world_main.c` (for reference if needed)
2. Implement new components in parallel with existing code
3. Update `main/CMakeLists.txt` to include new components
4. Replace `app_main()` with garage door application
5. Configure sdkconfig for Matter/Thread (via `idf.py menuconfig`)
6. Test on hardware before removing Hello World code

### Rollback
If critical issues discovered:
1. Revert CMakeLists.txt changes
2. Restore hello_world_main.c
3. Rebuild with `idf.py fullclean && idf.py build`

## Open Questions

### Q1: Relay Activation Duration
- **Question**: Should relay pulse be configurable (default 500ms) or fixed?
- **Recommendation**: Configurable via NVS (default 500ms) for different opener models
- **Defer to**: Implementation phase (test with actual Genie opener)

### Q2: Matter Endpoint Structure
- **Question**: Single endpoint or multiple (one per future sensor)?
- **Recommendation**: Start with single endpoint (Window Covering), add endpoints in future changes
- **Rationale**: Simplicity for initial implementation, Matter supports dynamic endpoint addition

### Q3: Obstruction Detection Sensitivity
- **Question**: How to distinguish obstruction from normal operation variance?
- **Recommendation**: Use reed switch edge detection + timeout; future: add HC-SR04 distance monitoring
- **Defer to**: Testing phase (calibrate with physical door)

### Q4: Thread Network Credentials Storage
- **Question**: How to handle Thread network credentials securely?
- **Recommendation**: Use NVS encryption (ESP-IDF built-in), Matter commissioning handles credential provisioning
- **Action**: Enable NVS encryption in sdkconfig

### Q5: Status LED Behavior
- **Question**: What LED patterns for different states?
- **Recommendation**:
  - Solid: Door stationary (OPEN/CLOSED)
  - Blinking fast: Door moving (OPENING/CLOSING)
  - Blinking slow: Matter not commissioned
  - Off: Error state / STOPPED
- **Defer to**: Implementation phase (define LED patterns in code)

## Implementation Notes

### ESP-Matter SDK Integration
- Requires ESP-IDF v5.1+ and ESP-Matter repository cloned
- Follow ESP-Matter examples: `examples/light` for device structure, `examples/blemesh` for BLE commissioning
- Use `esp_matter::endpoint::window_covering` helper for cluster creation

### FreeRTOS Task Design
Recommended task priorities (0 = lowest, higher number = higher priority):
- **Matter event loop**: Priority 5 (ESP-Matter default)
- **Reed switch monitoring**: Priority 6 (higher than Matter for safety)
- **Safety watchdog**: Priority 7 (highest for critical safety checks)
- **Idle task**: Priority 0 (default)

### Testing Strategy
1. **Unit Tests**: State machine logic (pytest-embedded-idf)
2. **Integration Tests**: Reed switch + relay coordination
3. **Matter Commissioning**: Validate with chip-tool CLI
4. **Thread Network**: Verify border router communication
5. **Safety Tests**: Manually trigger timeout/obstruction scenarios
6. **End-to-End**: Commission to Google Home/Apple Home, test remote control
