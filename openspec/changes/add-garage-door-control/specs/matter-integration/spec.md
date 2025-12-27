# Spec Delta: Matter Integration

## ADDED Requirements

### Requirement: Matter Window Covering Cluster Implementation
The system SHALL implement Matter Window Covering cluster (Type 514) to expose garage door control via Matter protocol.

#### Scenario: Cluster initialization
- **GIVEN** Matter device is starting
- **WHEN** window covering cluster is created
- **THEN** cluster supports UpOrOpen and DownOrClose commands
- **AND** cluster reports CurrentPositionLiftPercent100ths attribute
- **AND** TargetPositionLiftPercent100ths attribute is writable
- **AND** OperationalStatus attribute reflects current operation

#### Scenario: UpOrOpen command handling
- **GIVEN** Matter device is commissioned
- **WHEN** UpOrOpen command is received from Matter controller
- **THEN** command is forwarded to garage door open API
- **AND** OperationalStatus updates to "Lift Opening"
- **AND** CurrentPositionLiftPercent100ths updates when operation completes

#### Scenario: DownOrClose command handling
- **GIVEN** Matter device is commissioned
- **WHEN** DownOrClose command is received from Matter controller
- **THEN** command is forwarded to garage door close API
- **AND** OperationalStatus updates to "Lift Closing"
- **AND** CurrentPositionLiftPercent100ths updates when operation completes

#### Scenario: StopMotion command handling
- **GIVEN** door is in OPENING or CLOSING state
- **WHEN** StopMotion command is received
- **THEN** command is forwarded to garage door stop API
- **AND** OperationalStatus updates to "Lift Stopped"

#### Scenario: Attribute reporting on state change
- **GIVEN** door state changes
- **WHEN** state transitions from OPENING to OPEN
- **THEN** CurrentPositionLiftPercent100ths updates to 10000 (100%)
- **AND** OperationalStatus updates to "Lift Idle"
- **AND** attribute change notification is sent to subscribed Matter controllers

### Requirement: Thread Network Support
The system SHALL support Thread networking using IEEE 802.15.4 radio with Matter over Thread protocol.

#### Scenario: Thread network initialization
- **GIVEN** ESP32-H2 boots with Thread enabled
- **WHEN** Matter stack initializes
- **THEN** Thread radio is configured on IEEE 802.15.4
- **AND** Thread stack version 1.3+ is initialized
- **AND** Device operates as Thread End Device (not Router)

#### Scenario: Thread network joining
- **GIVEN** Thread network credentials are provisioned
- **WHEN** device attempts to join network
- **THEN** device scans for Thread border routers
- **AND** attaches to network within 30 seconds
- **AND** obtains IPv6 address from border router
- **AND** Matter endpoint is reachable via IPv6

#### Scenario: Thread network persistence
- **GIVEN** device has joined Thread network
- **WHEN** device reboots
- **THEN** Thread credentials are loaded from NVS
- **AND** device rejoins network automatically
- **AND** rejoin completes within 10 seconds

#### Scenario: Thread network loss recovery
- **GIVEN** device is connected to Thread network
- **WHEN** border router becomes unavailable
- **THEN** device attempts to find alternative border router
- **AND** retries for up to 5 minutes
- **AND** if recovery fails, enters commissioned-but-disconnected state
- **AND** door control remains functional locally (state tracking)

### Requirement: BLE Commissioning Support
The system SHALL support Matter commissioning via Bluetooth Low Energy 5.2 for initial network onboarding.

#### Scenario: BLE advertisement for commissioning
- **GIVEN** device is not commissioned
- **WHEN** device boots
- **THEN** BLE advertising starts with Matter service UUID
- **AND** advertisement includes discriminator and vendor ID
- **AND** QR code payload is generated for scanning

#### Scenario: BLE commissioning flow
- **GIVEN** Matter controller scans for devices
- **WHEN** user initiates pairing via controller app
- **THEN** BLE connection is established
- **AND** PASE (Password Authenticated Session Establishment) completes
- **AND** Thread network credentials are provisioned
- **AND** device joins Thread network
- **AND** commissioning completes within 2 minutes

#### Scenario: Commissioned device behavior
- **GIVEN** device is already commissioned
- **WHEN** device boots
- **THEN** BLE advertising is disabled (unless factory reset)
- **AND** device joins Thread network directly
- **AND** Matter endpoint is available for control

#### Scenario: Factory reset triggers BLE advertising
- **GIVEN** device is commissioned
- **WHEN** factory reset is triggered (long button press)
- **THEN** Thread credentials are erased from NVS
- **AND** Matter fabric is removed
- **AND** BLE advertising restarts for recommissioning

### Requirement: Matter Device Identification
The system SHALL provide Matter device metadata for discovery and identification in smart home ecosystems.

#### Scenario: Device type declaration
- **GIVEN** Matter device is initialized
- **WHEN** device descriptor cluster is queried
- **THEN** device type is "Window Covering" (Type 514)
- **AND** vendor ID matches ESP32 vendor (0xFFF1 for development)
- **AND** product ID identifies garage door controller

#### Scenario: Device information attributes
- **GIVEN** Matter controller reads basic information cluster
- **WHEN** attributes are requested
- **THEN** VendorName is "Espressif"
- **AND** ProductName is "Smart Garage Door"
- **AND** SoftwareVersionString matches firmware version
- **AND** SerialNumber is unique per device (derived from MAC address)

### Requirement: Matter Endpoint Structure
The system SHALL expose a single Matter endpoint for garage door control with Window Covering cluster.

#### Scenario: Endpoint configuration
- **GIVEN** Matter device is created
- **WHEN** endpoints are configured
- **THEN** endpoint 1 contains Window Covering cluster (server role)
- **AND** endpoint 1 contains Descriptor cluster
- **AND** endpoint 1 contains Identify cluster
- **AND** root endpoint (0) contains basic information and network commissioning clusters

#### Scenario: Endpoint discovery
- **GIVEN** Matter device is commissioned
- **WHEN** controller queries endpoint list
- **THEN** endpoint 0 (root) and endpoint 1 (window covering) are reported
- **AND** endpoint 1 cluster list includes Window Covering cluster ID (0x0102)

### Requirement: Matter Command Response Time
The system SHALL respond to Matter commands within 500ms to meet smart home responsiveness expectations.

#### Scenario: Command acknowledgment latency
- **GIVEN** Matter controller sends UpOrOpen command
- **WHEN** command is received by device
- **THEN** command acknowledgment is sent within 100ms
- **AND** door operation begins within 200ms
- **AND** attribute update is sent within 500ms

#### Scenario: Network latency budget
- **GIVEN** Matter command travels over Thread network
- **WHEN** end-to-end latency is measured
- **THEN** Thread network latency is <100ms (border router to device)
- **AND** device processing latency is <200ms
- **AND** total command-to-action latency is <500ms

### Requirement: Matter Security and Authentication
The system SHALL implement Matter security requirements including device attestation and secure sessions.

#### Scenario: Device attestation
- **GIVEN** device is being commissioned
- **WHEN** Matter controller requests device attestation
- **THEN** device provides attestation certificate chain
- **AND** certificate is signed by Espressif PAA (Product Attestation Authority)
- **AND** certificate validation succeeds

#### Scenario: Secure session establishment
- **GIVEN** Matter controller attempts to communicate
- **WHEN** session is established
- **THEN** CASE (Certificate Authenticated Session Establishment) is used for operational communication
- **AND** all commands are encrypted with session keys
- **AND** message integrity is verified

#### Scenario: ACL enforcement
- **GIVEN** multiple Matter controllers are commissioned
- **WHEN** controller attempts to send command
- **THEN** Access Control List (ACL) is checked
- **AND** only authorized controllers can control door
- **AND** unauthorized commands are rejected with status UNSUPPORTED_ACCESS

### Requirement: Matter Subscription and Reporting
The system SHALL support Matter attribute subscriptions for real-time state updates to controllers.

#### Scenario: Attribute subscription
- **GIVEN** Matter controller is connected
- **WHEN** controller subscribes to CurrentPositionLiftPercent100ths attribute
- **THEN** subscription is established with min interval 1s, max interval 10s
- **AND** attribute reports are sent on value change
- **AND** reports are sent at max interval if no changes

#### Scenario: State change notification
- **GIVEN** controller is subscribed to door state attributes
- **WHEN** door transitions from CLOSED to OPENING
- **THEN** OperationalStatus attribute change is reported immediately
- **AND** CurrentPositionLiftPercent100ths update is sent when transition completes

### Requirement: Matter Over-The-Air Updates
The system SHALL support Matter OTA software updates for firmware upgrades (basic implementation).

#### Scenario: OTA provider discovery
- **GIVEN** Matter device is commissioned
- **WHEN** OTA provider announces availability
- **THEN** device queries for available updates
- **AND** software version is compared with available version

#### Scenario: OTA update deferral
- **GIVEN** OTA update is available
- **WHEN** door is in OPENING or CLOSING state
- **THEN** OTA update is deferred until door reaches stable state (OPEN/CLOSED)
- **AND** update resumes after state stabilizes

### Requirement: Matter Fail-Safe Mode
The system SHALL implement Matter fail-safe timer during commissioning to prevent bricking.

#### Scenario: Fail-safe timer during commissioning
- **GIVEN** commissioning is in progress
- **WHEN** fail-safe timer is armed (default 60 seconds)
- **THEN** partial configuration is rolled back if timer expires
- **AND** device returns to pre-commissioning state
- **AND** BLE advertising resumes for retry

#### Scenario: Fail-safe disarm on success
- **GIVEN** fail-safe timer is armed
- **WHEN** commissioning completes successfully
- **THEN** fail-safe timer is disarmed
- **AND** configuration is committed to NVS
- **AND** device operates normally

### Requirement: Matter Diagnostics and Logging
The system SHALL provide Matter-specific diagnostics for network debugging and troubleshooting.

#### Scenario: Thread network diagnostics
- **GIVEN** Matter device is operational
- **WHEN** diagnostics are queried via Matter CLI (chip-tool)
- **THEN** Thread network metrics are reported (RSSI, hop count, leader address)
- **AND** IPv6 addresses are listed
- **AND** Thread operational dataset is accessible

#### Scenario: Matter stack event logging
- **GIVEN** Matter stack is running
- **WHEN** significant events occur (commission, disconnect, command)
- **THEN** events are logged to NVS with timestamp
- **AND** logs include Matter transaction IDs for correlation
- **AND** maximum 50 Matter events are retained (circular buffer)
