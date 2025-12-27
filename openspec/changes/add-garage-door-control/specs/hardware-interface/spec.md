# Spec Delta: Hardware Interface

## ADDED Requirements

### Requirement: Reed Switch Driver Initialization
The system SHALL initialize GPIO pins for reed switch monitoring with interrupt-driven detection.

#### Scenario: GPIO pin configuration
- **GIVEN** system is starting
- **WHEN** reed switch driver initializes
- **THEN** closed position reed switch GPIO is configured as input with pull-up resistor
- **AND** open position reed switch GPIO is configured as input with pull-up resistor
- **AND** GPIO interrupts are enabled for both edges (rising and falling)

#### Scenario: Pin assignment from NVS
- **GIVEN** NVS contains custom GPIO configuration
- **WHEN** driver initializes
- **THEN** GPIO pins are loaded from NVS
- **AND** if NVS is empty, default pins are used (GPIO 2 = closed, GPIO 3 = open)
- **AND** invalid pin numbers are rejected with error

#### Scenario: Pin conflict detection
- **GIVEN** GPIO pins are being configured
- **WHEN** same pin is assigned to multiple functions
- **THEN** initialization fails with error code
- **AND** error is logged identifying conflicting pin assignment

### Requirement: Reed Switch Debouncing
The system SHALL debounce reed switch signals to prevent false triggers from mechanical bounce.

#### Scenario: Bounce filtering during transition
- **GIVEN** reed switch is transitioning from inactive to active
- **WHEN** multiple edges are detected within 50ms
- **THEN** only the final stable state is reported
- **AND** intermediate bounces are ignored

#### Scenario: Stable state detection
- **GIVEN** reed switch has transitioned
- **WHEN** state remains stable for 50ms
- **THEN** state change event is generated
- **AND** event is posted to door control task queue

#### Scenario: Debounce timer configuration
- **GIVEN** system is initialized
- **WHEN** debounce configuration is loaded from NVS
- **THEN** debounce period is applied (range: 10-200ms, default: 50ms)
- **AND** invalid periods are rejected with fallback to default

### Requirement: Reed Switch State Reading
The system SHALL provide synchronous and asynchronous methods for reading reed switch states.

#### Scenario: Synchronous state read
- **GIVEN** driver is initialized
- **WHEN** `reed_switch_read_state()` is called
- **THEN** current GPIO level is read immediately
- **AND** state is returned as ACTIVE or INACTIVE
- **AND** read completes in <1 microsecond

#### Scenario: Asynchronous state change notification
- **GIVEN** application is monitoring reed switches
- **WHEN** reed switch state changes (after debouncing)
- **THEN** callback function is invoked with new state
- **AND** callback executes in task context (not ISR)

#### Scenario: Dual reed switch state query
- **GIVEN** both reed switches are installed
- **WHEN** `reed_switch_get_position()` is called
- **THEN** function returns DOOR_CLOSED if closed switch is active
- **AND** returns DOOR_OPEN if open switch is active
- **AND** returns DOOR_INTERMEDIATE if neither or both switches are active

### Requirement: Reed Switch Interrupt Handling
The system SHALL handle GPIO interrupts efficiently with minimal ISR execution time.

#### Scenario: ISR execution time
- **GIVEN** reed switch GPIO interrupt triggers
- **WHEN** ISR executes
- **THEN** ISR completes in <10 microseconds
- **AND** ISR only captures timestamp and posts event to queue
- **AND** debouncing and state processing occur in task context

#### Scenario: Interrupt prioritization
- **GIVEN** multiple interrupts occur simultaneously
- **WHEN** reed switch ISR is triggered
- **THEN** ISR executes at priority level 3 (high but not critical)
- **AND** ISR does not block higher-priority safety interrupts

### Requirement: Relay Control Driver Initialization
The system SHALL initialize GPIO pin for relay control with fail-safe defaults.

#### Scenario: Relay GPIO configuration
- **GIVEN** system is starting
- **WHEN** relay driver initializes
- **THEN** relay control GPIO is configured as output
- **AND** GPIO is set to LOW (relay inactive) by default
- **AND** GPIO pull-down resistor is enabled for fail-safe

#### Scenario: Relay pin assignment from NVS
- **GIVEN** NVS contains relay GPIO configuration
- **WHEN** driver initializes
- **THEN** relay GPIO is loaded from NVS
- **AND** if NVS is empty, default pin is used (GPIO 4)
- **AND** relay state is verified LOW before completing initialization

### Requirement: Relay Pulse Control
The system SHALL activate relay with precise timing control and automatic timeout protection.

#### Scenario: Relay activation pulse
- **GIVEN** door operation is initiated
- **WHEN** `relay_activate_pulse()` is called with duration 500ms
- **THEN** GPIO is set HIGH
- **AND** timer is started for 500ms
- **AND** GPIO is set LOW after timer expires
- **AND** pulse duration accuracy is ±10ms

#### Scenario: Pulse duration configuration
- **GIVEN** system is initialized
- **WHEN** relay pulse duration is loaded from NVS
- **THEN** duration is applied (range: 100-2000ms, default: 500ms)
- **AND** invalid durations are rejected with fallback to default

#### Scenario: Relay fail-safe timeout
- **GIVEN** relay activation is in progress
- **WHEN** relay GPIO has been HIGH for >600ms (hardware timeout)
- **THEN** GPIO is forced LOW immediately
- **AND** hardware fault event is logged
- **AND** relay driver enters safe state (no further activations until reset)

#### Scenario: Concurrent pulse prevention
- **GIVEN** relay is currently active
- **WHEN** new pulse request is received
- **THEN** request is rejected with error code EBUSY
- **AND** current pulse continues uninterrupted

### Requirement: Relay Debounce and Rate Limiting
The system SHALL enforce minimum time between relay activations to prevent rapid cycling.

#### Scenario: Relay activation rate limit
- **GIVEN** relay has just completed a pulse
- **WHEN** new activation is requested within 1 second
- **THEN** request is queued or rejected (configurable)
- **AND** activation does not occur until rate limit period expires

#### Scenario: Rate limit configuration
- **GIVEN** system is initialized
- **WHEN** relay rate limit is loaded from NVS
- **THEN** minimum interval is applied (range: 500-5000ms, default: 1000ms)
- **AND** invalid intervals are rejected with fallback to default

### Requirement: GPIO Error Handling and Diagnostics
The system SHALL detect and report GPIO hardware errors with graceful degradation.

#### Scenario: GPIO initialization failure
- **GIVEN** GPIO driver initialization is attempted
- **WHEN** GPIO pin configuration fails (hardware fault)
- **THEN** error code is returned
- **AND** error is logged with pin number and failure reason
- **AND** system enters safe state (door control disabled)

#### Scenario: GPIO state mismatch detection
- **GIVEN** GPIO is expected to be in specific state
- **WHEN** read-back verification shows different state
- **THEN** hardware fault is logged
- **AND** state is re-applied (write retry)
- **AND** if retry fails, relay driver is disabled for safety

#### Scenario: Reed switch failure detection
- **GIVEN** both reed switches are inactive for extended period
- **WHEN** door operation has been in OPENING or CLOSING for >60 seconds
- **THEN** potential reed switch failure is flagged
- **AND** diagnostic event is logged
- **AND** user is notified via Matter alert (future capability)

### Requirement: Hardware Configuration Persistence
The system SHALL persist GPIO configuration to NVS for field customization and recovery.

#### Scenario: Save GPIO configuration
- **GIVEN** GPIO pins are configured
- **WHEN** `gpio_config_save()` is called
- **THEN** pin assignments are written to NVS namespace "hw_config"
- **AND** write is verified with read-back
- **AND** success status is returned

#### Scenario: Load GPIO configuration
- **GIVEN** system is booting
- **WHEN** GPIO drivers initialize
- **THEN** pin assignments are loaded from NVS namespace "hw_config"
- **AND** if NVS is empty or corrupt, default pins are used
- **AND** loaded configuration is validated before application

#### Scenario: Factory reset hardware configuration
- **GIVEN** factory reset is triggered
- **WHEN** NVS is erased
- **THEN** GPIO configuration reverts to compile-time defaults
- **AND** default pins are re-applied on next boot

### Requirement: GPIO Diagnostics and Testing
The system SHALL provide diagnostic tools for GPIO testing and troubleshooting.

#### Scenario: GPIO self-test on boot
- **GIVEN** system is booting with diagnostics enabled
- **WHEN** GPIO self-test executes
- **THEN** each GPIO pin is configured and verified
- **AND** pull-up/pull-down resistors are tested
- **AND** interrupt functionality is validated
- **AND** test results are logged

#### Scenario: Relay test pulse
- **GIVEN** diagnostic mode is active
- **WHEN** relay test is requested
- **THEN** relay activates for 100ms (short pulse)
- **AND** GPIO state is verified during and after pulse
- **AND** test result (pass/fail) is returned

#### Scenario: Reed switch state reporting
- **GIVEN** diagnostic query is received
- **WHEN** `gpio_get_diagnostics()` is called
- **THEN** current state of all reed switches is reported
- **AND** interrupt counts for each switch are included
- **AND** last state change timestamps are provided

### Requirement: Power-On State Safety
The system SHALL ensure safe GPIO states during power-on and initialization.

#### Scenario: Relay inactive at power-on
- **GIVEN** ESP32-H2 is powering on
- **WHEN** hardware reset occurs
- **THEN** relay GPIO defaults to LOW (hardware pull-down)
- **AND** relay remains LOW until driver explicitly initializes
- **AND** accidental door activation is prevented

#### Scenario: Reed switch pull-ups at power-on
- **GIVEN** ESP32-H2 is powering on
- **WHEN** hardware reset occurs
- **THEN** reed switch GPIOs are configured as inputs with pull-ups
- **AND** floating inputs are prevented
- **AND** stable state is read immediately after initialization

### Requirement: GPIO Thread Safety
The system SHALL ensure thread-safe access to GPIO resources using FreeRTOS synchronization.

#### Scenario: Relay concurrent access protection
- **GIVEN** multiple tasks attempt to control relay
- **WHEN** `relay_activate_pulse()` is called concurrently
- **THEN** access is serialized via mutex
- **AND** only one pulse is active at a time
- **AND** conflicting requests are rejected with EBUSY

#### Scenario: Reed switch callback thread safety
- **GIVEN** reed switch state changes
- **WHEN** callback is invoked in task context
- **THEN** callback receives deep copy of state data
- **AND** callback execution does not block GPIO driver
- **AND** callback can safely call door control APIs

### Requirement: Hardware Abstraction Layer
The system SHALL provide hardware abstraction to support different ESP32 variants and custom PCBs.

#### Scenario: Platform-specific GPIO mapping
- **GIVEN** code is compiled for different ESP32 variant
- **WHEN** GPIO driver initializes
- **THEN** platform-specific pin constraints are respected
- **AND** unavailable pins are rejected at compile time (where possible)
- **AND** runtime validation catches invalid pin assignments

#### Scenario: Custom PCB support
- **GIVEN** custom PCB has different GPIO routing
- **WHEN** GPIO configuration is updated via NVS
- **THEN** new pin assignments are applied without recompilation
- **AND** configuration is validated against hardware capabilities
- **AND** invalid assignments are rejected with helpful error messages
