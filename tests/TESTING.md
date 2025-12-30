# Unit and Integration Testing Procedures

This document provides testing procedures for the Smart Garage Door Controller.

## Test Environment Setup

### Hardware Requirements
- ESP32-H2 development board or ESP32
- 2x Reed switches (for door position sensing)
- 1x Relay module (for door control)
- Multimeter (for continuity testing)
- Logic analyzer or oscilloscope (optional, for timing verification)

### Software Requirements
- ESP-IDF v5.1 or later
- Python 3.8+ with pytest
- Serial terminal (e.g., Minicom, screen, or Serial Monitor)
- chip-tool (for Matter/Thread testing, see COMMISSIONING.md)

### Safety Precautions
⚠️ **WARNING**: Garage doors are powerful and can cause serious injury. Follow all safety guidelines:
1. Keep clear of door path during testing
2. Have emergency release ready
3. Test with door disconnected from motor first
4. Never bypass safety sensors during testing
5. Use appropriate current limiting when testing relays

## Unit Tests

### State Machine Tests

**File**: `test_state_machine.py` (placeholder for future implementation)

#### Test Cases

1. **Initial State**
   - Verify door starts in CLOSED state on first boot
   - Check initial position is 0%

2. **Valid State Transitions**
   - CLOSED → OPENING → OPEN (open command)
   - OPEN → CLOSING → CLOSED (close command)
   - OPENING → STOPPED (stop command)
   - CLOSING → STOPPED (stop command)
   - STOPPED → OPENING (open command)
   - STOPPED → CLOSING (close command)

3. **Invalid State Transitions**
   - OPEN → OPENING should be rejected (already opening)
   - CLOSING → CLOSING should be rejected (already closing)
   - CLOSED → CLOSED should be rejected (already closed)

4. **Position Tracking**
   - Position should be 0% when CLOSED
   - Position should be 100% when OPEN
   - Position should maintain last known value while moving

#### Verification Method
- Monitor door state via serial output
- Verify state transitions match expected behavior
- Check position updates are correct
- Verify timeout handling works (e.g., stop if no end switch reached)

### Reed Switch Debouncing Tests

#### Test Cases

1. **Debounce Window**
   - Verify debounce time is between 20ms and 200ms
   - Confirm rapid switch bounces (glitches) are filtered out
   - Test with multimeter to measure actual debounce time

2. **Closed Switch Detection**
   - When door is closed, magnetic field absent
   - Reed switch should report OPEN (GPIO HIGH)
   - Verify with multimeter continuity test

3. **Open Switch Detection**
   - When door is open, magnetic field present
   - Reed switch should report CLOSED (GPIO LOW)
   - Verify with multimeter continuity test

4. **Both Switches Closed**
   - Both closed and open switches should be OPEN
   - Door position should be reported as 0%

5. **Both Switches Open**
   - Both closed and open switches should be CLOSED
   - Door position should be reported as 100%

6. **Mixed Switch States**
   - Both switches should never be in same state while door moving
   - Mixed states (one closed, one open) should trigger ERROR state

#### Verification Method
- Use multimeter to measure GPIO levels
- Manually actuate door and observe switch states
- Monitor serial output for state detection
- Test with switch disconnected to verify error handling

### Relay Timing Tests

#### Test Cases

1. **Pulse Duration**
   - Default pulse duration: 500ms
   - Configurable range: 100-1000ms
   - Verify pulse length is long enough to trigger door
   - Verify pulse is not too long (can cause overheating)

2. **Minimum Interval**
   - Default minimum: 1000ms (1 second)
   - Prevents rapid-fire commands
   - Test with multiple quick commands - second command should be rejected

3. **Maximum Pulse Duration**
   - Default maximum: 600ms
   - Safety limit for stuck relay detection
   - If pulse exceeds max, trigger error

4. **Open Command**
   - `garage_door_open()` should pulse relay once
   - Door should start moving
   - State should transition to OPENING

5. **Close Command**
   - `garage_door_close()` should pulse relay once
   - Door should start moving
   - State should transition to CLOSING

6. **Stop Command**
   - `garage_door_stop()` should NOT pulse relay
   - Should stop door if currently moving
   - State should transition to STOPPED

#### Verification Method
- Use oscilloscope or logic analyzer to measure relay pulse width
- Verify timing with serial output logs
- Test with actual garage door mechanism
- Measure power consumption to detect stuck relays

### NVS Persistence Tests

#### Test Cases

1. **GPIO Configuration Persistence**
   - Configure GPIO pins in NVS
   - Reboot device
   - Verify configuration is loaded correctly
   - Check that pins match configured values

2. **Relay Configuration Persistence**
   - Configure pulse duration in NVS
   - Reboot device
   - Verify configuration is loaded correctly
   - Check that duration matches configured value

3. **Default Configuration Fallback**
   - Erase NVS partition
   - Reboot device
   - Verify defaults are used:
     - Reed closed pin: GPIO 2
     - Reed open pin: GPIO 3
     - Relay pin: GPIO 4
     - Pulse duration: 500ms
     - Max pulse duration: 600ms
     - Minimum interval: 1000ms

4. **Configuration Validation**
   - Try to save invalid GPIO pin (e.g., 0 or > 39)
   - Should reject invalid configuration
   - Try to save invalid timing (e.g., 0ms or negative)
   - Should reject invalid timing

#### Verification Method
- Use serial console to erase NVS
- Reboot and verify defaults loaded
- Modify config via console commands and verify persistence
- Check NVS contents with `esp nvs` commands

### Thread Safety Tests

#### Test Cases

1. **Callback Mutex Protection**
   - Multiple threads should be able to register callbacks safely
   - State updates should not corrupt when multiple callbacks fire
   - Verify no race conditions in state machine

2. **Concurrent State Reads**
   - Multiple tasks should be able to read door state simultaneously
   - Reads should return consistent values
   - No deadlocks should occur

3. **Concurrent Commands**
   - Submit open, close, and stop commands rapidly
   - Only one should execute at a time
   - Busy commands should be rejected with error

4. **Event Group Usage**
   - FreeRTOS event groups should notify state changes
   - Bits should be set and cleared properly
   - Waiting tasks should wake up on state change

#### Verification Method
- Create test tasks that perform concurrent operations
- Monitor for deadlocks or crashes
- Check that mutual exclusion is working
- Verify event notifications work reliably

### Hardware Integration Tests

#### Test Cases

1. **GPIO Initialization**
   - Reed switches configured as INPUT with pull-up resistors
   - Relay configured as OUTPUT
   - Correct pull-up/down configuration applied

2. **Interrupt Configuration**
   - GPIO interrupts enabled on reed switches
   - Edge detection configured (falling or rising)
   - Interrupt handler registered and functioning

3. **Relay Driver State**
   - Relay driver tracks active/inactive state
   - Enforces minimum interval between pulses
   - Prevents overheating with max duration limit

4. **Position Calculation**
   - Position 0% when both switches CLOSED
   - Position 100% when both switches OPEN
   - Position ~50% when one switch OPEN, one CLOSED
   - Position updates correctly during movement

5. **State Callback Registration**
   - Reed switch changes trigger door state callback
   - State updates propagate to registered listeners
   - Matter device updates on state changes

#### Verification Method
- Use multimeter to verify GPIO electrical properties
- Monitor GPIO levels with logic analyzer
- Trigger actual door mechanism and observe behavior
- Test with disconnected sensors to verify error detection

### Safety Features Tests

#### Test Cases

1. **Operation Timeout**
   - Door movement stops if no end switch reached within timeout
   - Default timeout: 30 seconds
   - Should work for both open and close operations
   - Timeout should be configurable

2. **Obstruction Detection**
   - If door stops before reaching limit, indicates obstruction
   - Should enter ERROR state
   - Error should be logged with appropriate severity
   - Manual intervention required to clear

3. **Rapid Command Protection**
   - Minimum interval enforced between commands
   - Default: 1 second
   - Commands issued too quickly are rejected
   - Protects against rapid-fire attacks or glitches

4. **Manual Override**
   - Manual wall switch can stop movement at any time
   - Should override automatic timeout
   - Resets timeout timer
   - Should work regardless of current state

#### Verification Method
- Block door mechanism and attempt open/close - should timeout
- Obstruct door path and test - should detect and stop
- Issue rapid commands - should be rejected
- Test manual wall switch - should stop immediately

### Matter Integration Tests (Stub Mode)

#### Test Cases

1. **Matter Device Initialization**
   - `matter_device_init()` returns ESP_OK
   - Creates event group for state changes
   - Registers door state callback
   - Logs warning about stub mode

2. **Matter Device Deinitialization**
   - `matter_device_deinit()` returns ESP_OK
   - Stops Matter task
   - Cleans up event group
   - Frees allocated resources

3. **Door State Update**
   - `matter_device_update_door_state()` updates position attribute
   - Updates operational status based on movement
   - Logs appropriate debug information
   - (In full implementation) would update ESP-Matter attributes

4. **Stub Mode Operation**
   - Logs configuration instructions
   - Provides interface matching expected behavior
   - Can be expanded to full ESP-Matter implementation

#### Verification Method
- Monitor serial output during initialization
- Trigger door state changes and observe Matter updates
- Verify stub logs provide helpful guidance
- Check that no crashes occur in stub mode

## Integration Tests

### Full System Tests

1. **Cold Boot Test**
   - Power on device
   - Verify correct initial state (CLOSED)
   - Verify NVS configuration loaded
   - Verify all components initialized

2. **Normal Operation Test**
   - Open door and wait for completion
   - Verify state transitions correctly
   - Check position updates
   - Close door and wait for completion
   - Verify state transitions correctly

3. **Stop Operation Test**
   - Start opening door
   - Issue stop command mid-operation
   - Verify door stops safely
   - Verify state is STOPPED

4. **Error Recovery Test**
   - Disconnect one reed switch
   - Verify ERROR state is entered
   - Manually reset error condition
   - Verify device recovers

5. **Configuration Change Test**
   - Modify GPIO configuration via serial console
   - Save to NVS
   - Reboot device
   - Verify new configuration applied

6. **Long-Running Stability Test**
   - Cycle door 100 times
   - Monitor for memory leaks
   - Check for state machine errors
   - Verify consistent behavior

## Test Execution

### Automated Test Commands

```bash
# Run all unit tests (requires Python implementation)
pytest tests/ -v

# Run specific test suite
pytest tests/test_garage_door.py::TestStateMachine -v

# Run tests with coverage
pytest tests/ --cov=components --cov-report=html
```

### Manual Test Checklist

Print and use this checklist for manual testing:

```
Unit Tests
□ State machine initial state
□ State transitions (open, close, stop)
□ Invalid transitions rejected
□ Position tracking accurate
□ Reed switch debouncing working
□ Relay pulse timing correct
□ NVS persistence working
□ Thread safety verified

Safety Features
□ Operation timeout works
□ Obstruction detection works
□ Rapid command protection enabled
□ Manual override functional

Hardware Integration
□ GPIOs initialized correctly
□ Interrupts firing properly
□ Position calculation accurate
□ State callbacks registered

Integration Tests
□ Cold boot test successful
□ Normal operation test passed
□ Stop operation test passed
□ Error recovery test passed
□ Configuration change test passed
□ Stability test (100 cycles) passed

Matter Integration (Stub)
□ Matter device initializes
□ Door state updates propagate
□ Deinitialization works
□ Stub mode logs correctly
```

## Test Reporting

### Test Results Template

```
Test Date: ___________
Tester: ___________
Hardware: ESP32-H2 / ESP32
Firmware Version: ___________

Test Results:
[ ] State Machine Tests - PASSED / FAILED
[ ] Reed Switch Tests - PASSED / FAILED
[ ] Relay Timing Tests - PASSED / FAILED
[ ] NVS Persistence Tests - PASSED / FAILED
[ ] Thread Safety Tests - PASSED / FAILED
[ ] Hardware Integration Tests - PASSED / FAILED
[ ] Safety Features Tests - PASSED / FAILED
[ ] Integration Tests - PASSED / FAILED

Issues Found:
1. _________________________________
2. _________________________________
3. _________________________________

Recommendations:
1. _________________________________
2. _________________________________
3. _________________________________
```

## Continuous Integration

### Pre-Commit Testing

Before committing code changes:

1. Run automated tests: `pytest tests/ -v`
2. Manual smoke test on hardware
3. Check for memory leaks with `esp-idf monitor`
4. Verify no warnings or errors in logs

### CI/CD Integration

Add to CI pipeline (when available):

```yaml
test:
  script:
    - pytest tests/ --cov=components
  after_success:
    - generate coverage report
```

## Troubleshooting

### Common Issues

1. **Door Won't Move**
   - Check relay connections
   - Verify GPIO configuration
   - Check power supply to relay
   - Verify door mechanism not physically blocked

2. **State Machine Stuck**
   - Check reed switch connections
   - Verify interrupt handler is firing
   - Check for software deadlock
   - Review state machine logic

3. **NVS Errors**
   - Check NVS partition size
   - Verify NVS initialized before use
   - Check for corrupted NVS data
   - Erase NVS and reconfigure if needed

4. **Matter Connection Issues**
   - Verify Thread network is running
   - Check BLE advertising status
   - Verify commissioning credentials
   - Check chip-tool network access

## References

- ESP-IDF Testing Guide: https://docs.espressif.com/projects/esp-idf/en/latest/esp32-guides/unit-testing.html
- Matter Testing Guide: [COMMISSIONING.md](./COMMISSIONING.md)
- pytest Documentation: https://docs.pytest.org/
