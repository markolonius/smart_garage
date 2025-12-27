# Spec Delta: Garage Door Control

## ADDED Requirements

### Requirement: Door State Machine
The system SHALL implement a 5-state finite state machine for garage door control with states: CLOSED, OPENING, OPEN, CLOSING, STOPPED.

#### Scenario: Successful open operation
- **GIVEN** door is in CLOSED state
- **WHEN** open command is received
- **THEN** door transitions to OPENING state
- **AND** relay activates for 500ms
- **AND** door transitions to OPEN state when open reed switch triggers

#### Scenario: Successful close operation
- **GIVEN** door is in OPEN state
- **WHEN** close command is received
- **THEN** door transitions to CLOSING state
- **AND** relay activates for 500ms
- **AND** door transitions to CLOSED state when closed reed switch triggers

#### Scenario: Invalid state transition prevented
- **GIVEN** door is in OPENING state
- **WHEN** open command is received
- **THEN** command is rejected
- **AND** error is logged
- **AND** door remains in OPENING state

#### Scenario: Stop operation during movement
- **GIVEN** door is in OPENING or CLOSING state
- **WHEN** stop command is received
- **THEN** door transitions to STOPPED state
- **AND** safety event is logged

### Requirement: Operation Timeout Protection
The system SHALL automatically stop door operation if movement does not complete within a configurable timeout period.

#### Scenario: Timeout during opening
- **GIVEN** door is in OPENING state
- **WHEN** timeout period expires (default 30 seconds)
- **AND** open reed switch has not triggered
- **THEN** door transitions to STOPPED state
- **AND** timeout event is logged to NVS
- **AND** safety alert is raised

#### Scenario: Timeout during closing
- **GIVEN** door is in CLOSING state
- **WHEN** timeout period expires (default 30 seconds)
- **AND** closed reed switch has not triggered
- **THEN** door transitions to STOPPED state
- **AND** timeout event is logged to NVS
- **AND** safety alert is raised

#### Scenario: Configurable timeout value
- **GIVEN** system is initialized
- **WHEN** timeout configuration is loaded from NVS
- **THEN** timeout value is applied (range: 10-60 seconds, default: 30 seconds)
- **AND** invalid timeout values are rejected with fallback to default

### Requirement: Obstruction Detection
The system SHALL detect obstructions during door operation by monitoring reed switch state changes.

#### Scenario: Obstruction detected during opening
- **GIVEN** door is in OPENING state
- **WHEN** closed reed switch triggers unexpectedly
- **THEN** door transitions to STOPPED state immediately
- **AND** obstruction event is logged to NVS
- **AND** safety alert is raised

#### Scenario: Obstruction detected during closing
- **GIVEN** door is in CLOSING state
- **WHEN** open reed switch triggers unexpectedly
- **THEN** door transitions to STOPPED state immediately
- **AND** obstruction event is logged to NVS
- **AND** safety alert is raised

#### Scenario: Normal reed switch transition ignored
- **GIVEN** door is in OPENING state
- **WHEN** open reed switch triggers (expected behavior)
- **THEN** door transitions to OPEN state
- **AND** no obstruction is logged

### Requirement: State Persistence and Recovery
The system SHALL persist door state to NVS and recover gracefully after power loss or restart.

#### Scenario: State recovery after reboot
- **GIVEN** system was operating before power loss
- **WHEN** system restarts
- **THEN** reed switches are read to determine current state
- **AND** if both reed switches inactive, state = STOPPED
- **AND** if closed reed switch active, state = CLOSED
- **AND** if open reed switch active, state = OPEN

#### Scenario: Transitional state recovery
- **GIVEN** system was in OPENING or CLOSING state during power loss
- **WHEN** system restarts
- **THEN** state is determined by current reed switch positions
- **AND** incomplete operation is logged as interrupted event

### Requirement: Door Control API
The system SHALL provide a public API for initiating door operations with input validation.

#### Scenario: Open door API call
- **GIVEN** valid door control context
- **WHEN** `garage_door_open()` is called
- **THEN** state machine validates transition
- **AND** returns success if transition valid
- **AND** returns error code if transition invalid

#### Scenario: Close door API call
- **GIVEN** valid door control context
- **WHEN** `garage_door_close()` is called
- **THEN** state machine validates transition
- **AND** returns success if transition valid
- **AND** returns error code if transition invalid

#### Scenario: Stop door API call
- **GIVEN** door is in any state
- **WHEN** `garage_door_stop()` is called
- **THEN** door transitions to STOPPED state
- **AND** returns success

#### Scenario: Get state API call
- **GIVEN** door control initialized
- **WHEN** `garage_door_get_state()` is called
- **THEN** current state is returned
- **AND** state reflects most recent reed switch readings

### Requirement: Thread-Safe Operation
The system SHALL ensure thread-safe access to door state using FreeRTOS synchronization primitives.

#### Scenario: Concurrent state access
- **GIVEN** multiple FreeRTOS tasks accessing door state
- **WHEN** tasks call door control API concurrently
- **THEN** access is serialized via mutex
- **AND** state consistency is maintained
- **AND** no race conditions occur

#### Scenario: Interrupt-safe reed switch handling
- **GIVEN** reed switch ISR is triggered
- **WHEN** ISR attempts to update state
- **THEN** state update is deferred to task context via event queue
- **AND** ISR completes in <10 microseconds

### Requirement: Relay Control
The system SHALL control the Genie garage door opener via relay with precise timing and safety limits.

#### Scenario: Relay activation pulse
- **GIVEN** door operation is initiated
- **WHEN** relay control is triggered
- **THEN** GPIO pin is set HIGH for exactly 500ms (±10ms)
- **AND** GPIO pin returns to LOW state after pulse

#### Scenario: Relay pulse timeout
- **GIVEN** relay activation is in progress
- **WHEN** relay has been HIGH for >600ms (fail-safe)
- **THEN** GPIO is forced LOW
- **AND** hardware fault is logged
- **AND** system enters safe state

#### Scenario: Relay debounce protection
- **GIVEN** relay was recently activated
- **WHEN** new operation is requested within 1 second
- **THEN** request is queued or rejected (configurable)
- **AND** relay is not activated until debounce period expires

### Requirement: Diagnostic Logging
The system SHALL log all door operations, state transitions, and safety events for troubleshooting.

#### Scenario: Operation logging
- **GIVEN** door operation is initiated
- **WHEN** state transition occurs
- **THEN** event is logged with timestamp, previous state, new state, trigger reason

#### Scenario: Safety event logging
- **GIVEN** safety condition is detected (timeout, obstruction)
- **WHEN** door transitions to STOPPED state
- **THEN** event is logged to NVS with severity CRITICAL
- **AND** event includes timestamp, state at detection, detected condition

#### Scenario: Log retrieval
- **GIVEN** diagnostic logs exist in NVS
- **WHEN** `garage_door_get_logs()` is called
- **THEN** logs are returned in chronological order
- **AND** maximum of 100 most recent events are retrieved
