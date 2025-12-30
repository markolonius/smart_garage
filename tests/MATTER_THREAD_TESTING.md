# Matter/Thread Integration Testing

Test procedures for Matter protocol with Thread networking and BLE commissioning.

## Test Environment Setup

### Hardware Requirements
- ESP32-H2 with Thread radio support
- Thread border router or Thread border router (for network formation)
- iOS 16.5+ or Android device (for Matter commissioning via chip-tool)
- Wi-Fi access point (for Thread border router internet access)
- Logic analyzer or Thread sniffer (optional, for Thread debugging)

### Software Requirements
- chip-tool (Matter controller): https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool
- ESP-IDF monitor (for device logs)
- Thread Wireshark dissector (for Thread protocol analysis)
- Serial terminal (for device console commands)

### Network Setup
1. **Thread Network**
   - Configure Thread border router with PAN ID, network key, channel
   - Ensure 802.15.4 network (6LoWPAN) is isolated
   - Verify Thread mesh is operational

2. **Wi-Fi Uplink** (for border router)
   - Configure border router with Wi-Fi internet access
   - Verify internet connectivity
   - DNS resolution working

3. **Commissioning Device**
   - Device in commissioning mode (BLE advertising)
   - Ready for pairing

## BLE Commissioning Tests

### Test Cases

1. **BLE Advertisement**
   - Device advertises on BLE when commissioned
   - Advertisement contains Matter service UUID
   - Discriminator is accessible
   - Device is discoverable by chip-tool

2. **BLE Pairing Flow**
   ```bash
   # Commission device via BLE
   chip-tool pairing ble-thread <node-id> <operational-dataset> 20202021 3840
   ```
   - Where:
     - `<node-id>`: Arbitrary node ID (e.g., 0x7283)
     - `<operational-dataset>`: Thread network credentials (hex encoded)
     - `20202021`: Default setup passcode
     - `3840`: Default discriminator

3. **Setup Passcode Verification**
   - Verify passcode is required for commissioning
   - Test with correct passcode: Should succeed
   - Test with incorrect passcode: Should fail

4. **Discriminator Verification**
   - Verify discriminator is used to identify device
   - Test with correct discriminator: Should succeed
   - Test with incorrect discriminator: Should timeout

5. **Manual Pairing Code**
   - Display pairing code in logs: `matter onboardingcodes`
   - Verify code format is correct
   - Test commissioning with manual code:
     ```bash
     chip-tool pairing code-thread <node-id> <ssid> <passphrase> <manual-code>
     ```

6. **QR Code Commissioning**
   - Generate QR code for device: See device display or logs
   - Scan QR code with Matter phone app
   - Verify successful commissioning

### Verification Method
- Use `ble_scan` command to verify BLE advertising
- Monitor serial logs during commissioning
- Check that device enters commissioned state
- Verify device obtains IPv6 address from Thread network

## Thread Network Tests

### Test Cases

1. **Network Join**
   - Device successfully joins Thread network
   - Obtains IPv6 global address
   - Registers RLOC (Routing Locator)
   - Network partitions are formed correctly

2. **Border Router Connectivity**
   - Device communicates with Thread border router
   - Can reach other Thread devices
   - Can reach internet via border router
   - Mesh connectivity is stable

3. **Network Recovery**
   - Device rejoins network after border router reboot
   - Device maintains network connection after link-local reset
   - Graceful handling of partition changes

4. **Thread Commissioning Retention**
   - Device retains commissioning data after reboot
   - Auto-joins Thread network on startup
   - No manual re-commissioning required

5. **Thread Metrics**
   - Parent link quality (RSSI, LQI)
   - Child count (for routers)
   - Route table entries
   - Network uptime

### Verification Method
- Use `matter esp thread dump` to see Thread status
- Check IPv6 address assignment
- Ping other Thread devices to verify connectivity
- Monitor Thread partition changes
- Use Thread Wireshark to analyze protocol messages

## Command Latency Tests

### Test Cases

1. **Open Command Latency**
   - Issue open command via Matter
   - Measure time to command receipt
   - Measure time to door movement start
   - **Requirement**: < 500ms

2. **Close Command Latency**
   - Issue close command via Matter
   - Measure time to command receipt
   - Measure time to door movement start
   - **Requirement**: < 500ms

3. **Stop Command Latency**
   - Issue stop command via Matter
   - Measure time to command receipt
   - Measure time to door movement stop
   - **Requirement**: < 500ms

4. **State Read Latency**
   - Read door position attribute via Matter
   - Measure round-trip time
   - **Requirement**: < 300ms

5. **Attribute Subscription**
   - Subscribe to door position changes
   - Verify notifications arrive within 1 second
   - Verify notification data is correct

6. **Concurrent Controller Support**
   - Commission device with multiple Matter controllers
   - Issue commands from different controllers
   - Verify all commands are processed correctly

### Verification Method
- Use chip-tool with timestamp output
- Measure latency with logic analyzer on GPIO
- Check serial logs for command processing times
- Verify attribute updates happen within SLA

**Sample chip-tool commands:**
```bash
# Measure command latency
chip-tool timing onoff on <node-id> <endpoint-id>
chip-timing windowcovering go-to-percent <node-id> <endpoint-id> <position>

# Subscribe to attributes
chip-tool windowcovering subscribe 100 1 1 1 0
```

## Subscription Tests

### Test Cases

1. **Position Subscription**
   - Subscribe to CurrentPositionLiftPercentage100th
   - Verify updates arrive on state change
   - Multiple subscriptions should work correctly

2. **Operational Status Subscription**
   - Subscribe to OperationalStatus attribute
   - Verify updates arrive on state change
   - Multiple subscribers should all receive notifications

3. **Subscription Persistence**
   - Verify subscriptions persist across reboots
   - Device maintains subscriptions after Thread network changes
   - No manual re-subscription required

4. **Subscription Cancellation**
   - Verify subscriptions can be cancelled
   - Device stops sending notifications
   - Resources are freed properly

5. **Large Attribute Updates**
   - Verify notifications for multi-attribute updates
   - Test with rapid attribute changes
   - Verify no notifications are lost

6. **Event Logging Subscription** (if implemented)
   - Subscribe to Matter events
   - Verify events are logged correctly
   - Verify events are accessible via diagnostics

### Verification Method
- Monitor chip-tool subscription callbacks
- Check device serial logs for subscription activity
- Verify no memory leaks with long-running subscriptions
- Test with varying subscription intervals

**Sample subscription commands:**
```bash
# Subscribe to door position (min=1s, max=10s)
chip-tool windowcovering subscribe current-position 100 1 1 1 10000

# Subscribe to operational status
chip-tool windowcovering subscribe operational-status 100 1 1 10000
```

## Border Router Failover Tests

### Test Cases

1. **Primary Router Failure**
   - Power off primary Thread border router
   - Device detects link loss
   - Device searches for alternate parent
   - Device connects to backup router (if available)

2. **Router Recovery**
   - Primary router comes back online
   - Device detects improved link quality
   - Device may reconnect to primary router
   - Handover is graceful

3. **Network Partition**
   - Create network partition (separate Thread networks)
   - Device detects partitioned network
   - Device maintains connectivity to one partition
   - Partitions can merge when routers reconnect

4. **Partition Merge**
   - Two Thread networks merge back together
   - Device rejoins single network
   - No loss of commissioning data
   - Connections are reestablished automatically

5. **Multiple Border Routers**
   - Configure network with 2+ border routers
   - Device selects best parent based on metrics
   - Failover works automatically
   - Load balancing between routers

### Verification Method
- Monitor Thread parent changes
- Check RLOC address stability
- Verify device maintains commissioning during failover
- Test with multiple router scenarios

**Thread network setup commands:**
```bash
# On device
matter esp thread dump
matter esp thread ping <ipv6-address>

# On router
# Configure multiple border routers in Thread network settings
```

## Multi-Controller Tests

### Test Cases

1. **Multiple Fabric Pairing**
   - Commission device to different Matter fabrics (e.g., Google Home, Apple Home)
   - Each fabric has unique node ID
   - Device maintains separate access control lists (ACLs)

2. **Concurrent Command Execution**
   - Issue commands from different controllers simultaneously
   - Verify commands are processed in order
   - Verify no command corruption
   - Verify no state conflicts

3. **ACL Configuration** (if implemented)
   - Set up Access Control Lists (ACLs) for each fabric
   - Verify only authorized controllers can issue commands
   - Test unauthorized access is rejected

4. **Fabric Index Management**
   - Verify each fabric has unique index
   - Commands target correct fabric
   - State updates go to all subscribed fabrics

5. **Fabric Removal**
   - Remove fabric from device
   - Verify all subscriptions are cancelled
   - Verify access is revoked
   - Verify device can be re-commissioned

### Verification Method
- Use chip-tool to test multiple fabrics
- Verify separate node IDs for each fabric
- Check ACL enforcement with different controllers
- Monitor device behavior with multiple concurrent connections

**Multi-fabric testing:**
```bash
# Commission to second fabric
chip-tool pairing ble-thread 0x7284 <operational-dataset> 20202021 3841

# Test commands from different fabrics
chip-tool -f <fabric-index> onoff on 0x7283 1
chip-tool -f <fabric-index> windowcovering go-to-percent 0x7283 1 50
```

## Integration Test Scenarios

### End-to-End Tests

1. **Complete Commissioning Flow**
   - BLE advertising → Pairing → Thread join → Control
   - Verify entire flow works end-to-end
   - Document total time for each step
   - **Requirement**: Complete flow < 2 minutes

2. **Commissioning to Control**
   - Commission device → Issue command → Door moves
   - Verify command works after commissioning
   - Verify no manual restart required
   - **Requirement**: Control works within 30s of commissioning

3. **Reboot Persistence**
   - Commission device → Reboot → Control still works
   - Verify commissioning data retained
   - Verify Thread network auto-joins
   - Verify subscriptions maintained

4. **Network Partition Recovery**
   - Create network partition → Device reconnects → Control works
   - Verify no manual intervention required
   - Verify state is maintained across partition

5. **Long-Running Stability**
   - Commission device → Run 24 hours → Verify stability
   - Monitor for memory leaks
   - Monitor for connection drops
   - Verify all operations work consistently

### Stress Tests

1. **Rapid Command Sequence**
   - Issue 100 commands rapidly (open/close/stop)
   - Verify no crashes
   - Verify no state corruption
   - Verify command latency remains acceptable

2. **Concurrent Attribute Reads**
   - 10 controllers read attributes simultaneously
   - Verify no data corruption
   - Verify all reads return correct values

3. **Network Flapping**
   - Cycle Thread network connectivity
   - Device maintains operation across drops
   - Graceful reconnection to network

4. **Large Payload Handling**
   - Send complex Matter commands
   - Verify parsing works correctly
   - Verify no buffer overflows

### Verification Method
- Run automated scripts for end-to-end scenarios
- Monitor device logs for errors or warnings
- Check memory usage during stress tests
- Verify no unexpected reboots occur

## Test Execution

### Automated Test Commands

```bash
# BLE commissioning test
chip-tool pairing ble-thread 0x7283 <dataset> 20202021 3840

# Thread network check
chip-tool basic read 0x7283 0 0 40 2 0

# Command latency test (requires timing)
time chip-tool windowcovering go-to-percent 0x7283 1 100

# Subscribe to attributes
chip-tool windowcovering subscribe 100 1 1 1 10000

# Multi-fabric test (commission second fabric)
chip-tool pairing ble-thread 0x7284 <dataset> 20202021 3841
```

### Manual Test Checklist

Print and use this checklist for Matter/Thread testing:

```
BLE Commissioning
□ BLE advertising active
□ Device discoverable by chip-tool
□ Correct passcode required
□ Discriminator accessible
□ QR code working
□ Pairing succeeds with correct credentials
□ Pairing fails with incorrect credentials

Thread Network
□ Joins Thread network successfully
□ Obtains IPv6 address
□ Registers RLOC correctly
□ Retains commissioning after reboot
□ Auto-joins on power cycle
□ Network recovery works

Command Latency
□ Open command < 500ms
□ Close command < 500ms
□ Stop command < 500ms
□ State read < 300ms
□ Subscriptions within 1s

Subscriptions
□ Position updates arrive
□ Operational status updates arrive
□ Multiple subscriptions work
□ Subscriptions persist across reboots
□ No memory leaks with subscriptions

Border Router Failover
□ Detects router failure
□ Connects to backup router
□ Network partition recovery works
□ Graceful router change handling

Multi-Controller
□ Multiple fabrics can pair
□ Concurrent commands work
□ ACL enforcement working
□ Fabric removal works correctly
```

## Test Reporting

### Test Results Template

```
Test Date: ___________
Tester: ___________
Thread Border Router: ___________
Thread Network: Thread Network 1 / Thread Network 2

Matter/Thread Test Results:
[ ] BLE Commissioning - PASSED / FAILED
[ ] Thread Network Join - PASSED / FAILED
[ ] Command Latency - PASSED / FAILED
[ ] Attribute Subscriptions - PASSED / FAILED
[ ] Border Router Failover - PASSED / FAILED
[ ] Multi-Controller Support - PASSED / FAILED
[ ] End-to-End Integration - PASSED / FAILED
[ ] Stability Test (24h) - PASSED / FAILED

Latency Measurements:
- Open command: _______ ms (target: < 500ms)
- Close command: _______ ms (target: < 500ms)
- Stop command: _______ ms (target: < 500ms)
- State read: _______ ms (target: < 300ms)

Thread Network Metrics:
- IPv6 Address: ________________________
- Parent Link Quality: RSSI: ___, LQI: ___
- Network Uptime: _______
- Partition Changes: _______

Issues Found:
1. _________________________________
2. _________________________________
3. _________________________________

Recommendations:
1. _________________________________
2. _________________________________
3. _________________________________
```

## Troubleshooting

### Common Issues

1. **BLE Not Advertising**
   - Check device is in commissioning mode
   - Verify `matter ble start` command issued
   - Check for BLE initialization errors
   - Verify BLE radio is functional

2. **Thread Won't Join**
   - Check operational dataset is correct
   - Verify Thread credentials match router
   - Check Thread channel availability
   - Verify IEEE 802.15.4 radio is functional

3. **Commissioning Fails**
   - Verify passcode is correct (default: 20202021)
   - Verify discriminator is correct (default: 3840)
   - Check for interference on 2.4 GHz band
   - Verify device is not already commissioned

4. **Commands Not Working**
   - Verify Thread network is connected
   - Check device node ID is correct
   - Verify endpoint ID is correct (default: 1 for Window Covering)
   - Check for Matter protocol errors

5. **Subscriptions Not Receiving**
   - Verify subscription is active
   - Check network connectivity is stable
   - Verify subscription interval is reasonable
   - Check device is sending notifications

6. **Network Instability**
   - Check for interference on 2.4 GHz band
   - Verify Thread channel is clear
   - Check border router connectivity
   - Verify network credentials are correct

### Debug Commands

```bash
# Check Matter device status
matter esp status
matter esp node
matter esp config

# Check Thread network status
matter esp thread dump
matter esp thread ping <ipv6-address>

# Check BLE status
matter ble start
matter ble stop
matter ble state

# Dump onboarding codes
matter onboardingcodes

# Factory reset device
matter esp factoryreset
```

## References

- Matter Specification: https://csa-iot.org/all-solutions/matter/
- chip-tool Documentation: https://github.com/project-chip/connectedhomeip/tree/master/docs/development_controllers/chip-tool
- Thread Specification: https://www.threadgroup.org/ThreadSpec
- ESP-Matter Thread Guide: https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html
