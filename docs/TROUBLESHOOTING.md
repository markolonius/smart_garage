# Troubleshooting Guide

This guide helps you diagnose and resolve common issues with the ESP32-H2 smart garage door controller.

## Table of Contents

- [Build and Flash Issues](#build-and-flash-issues)
- [Hardware Issues](#hardware-issues)
- [Door Control Issues](#door-control-issues)
- [Matter/Thread Issues](#matterthread-issues)
- [Performance Issues](#performance-issues)
- [Debugging](#debugging)

## Build and Flash Issues

### "idf.py: command not found"

**Cause**: ESP-IDF environment not set up.

**Solution**:
```bash
# Source ESP-IDF environment
source ~/esp/esp-idf/export.sh  # Adjust path as needed

# Verify
idf.py --version
```

### "No such file or directory: 'esp-idf'"

**Cause**: ESP-IDF not installed or wrong path.

**Solution**: Follow [ESP-IDF Setup Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32h2/get-started/index.html).

### Build errors: "component not found"

**Cause**: Component dependencies not properly configured in CMakeLists.txt.

**Solution**:
```bash
# Check CMakeLists.txt for correct PRIV_REQUIRES
cat main/CMakeLists.txt

# Should include: PRIV_REQUIRES "garage_door" "storage" "sensors"
```

### Flashing fails: "Invalid argument"

**Cause**: Wrong serial port or baud rate.

**Solution**:
```bash
# List available serial ports
idf.py -p list

# Use correct port
idf.py -p /dev/ttyUSB0 flash

# Lower baud rate (in menuconfig: Serial Flasher Config → Default baud rate)
idf.py -p /dev/ttyUSB0 -b 115200 flash
```

## Hardware Issues

### Reed Switch Not Detecting Door

**Symptoms**: Door position always shows UNKNOWN or wrong state.

**Causes**:
1. Magnet not aligned with reed switch
2. Switch wiring incorrect
3. Debouncing too aggressive

**Solutions**:

**1. Check magnet alignment:**
```bash
# Monitor reed switch state in real-time
idf.py monitor | grep "Position changed"

# Move door manually
# Should see position changes
```

- Magnets should be within 10mm (0.4") of switch
- Align magnet center with switch center
- Test by manually holding magnet near switch

**2. Verify wiring:**
```bash
# Use multimeter in continuity mode
# Test switch closure when magnet is near

# Expected: Continuity when magnet near, open when far
```

**3. Adjust debounce time** (in `reed_switch.c`):
```c
#define DEBOUNCE_MS 50  // Increase to 100ms if needed
```

### Relay Not Activating

**Symptoms**: Door doesn't respond to commands, relay doesn't click.

**Causes**:
1. GPIO not configured correctly
2. Insufficient power supply
3. Relay module defective

**Solutions**:

**1. Check GPIO with multimeter:**
```bash
# Activate relay via API
# Measure GPIO 4 voltage (should go HIGH briefly)

# Expected: 3.3V when activating, 0V when idle
```

**2. Test relay directly:**
```bash
# Connect relay IN directly to 5V
# Relay should click immediately

# If no click, relay module is defective
```

**3. Check power supply:**
```bash
# Measure voltage at ESP32-H2 5V pin
# Should be 5V ±0.2V under load

# If voltage drops below 4.5V when relay activates:
# - Use higher current power supply (need >500mA)
# - Add decoupling capacitors (100µF electrolytic)
```

### Device Randomly Restarts

**Symptoms**: ESP32-H2 resets unexpectedly, door stops mid-operation.

**Causes**:
1. Brownout (insufficient power)
2. Stack overflow
3. Watchdog timer timeout

**Solutions**:

**1. Check for brownout:**
```bash
idf.py monitor | grep "Brownout"

# If "Guru Meditation Error: Core 0 panic'ed (Brownout)":
# - Power supply voltage too low
# - Increase power supply current rating
# - Check for loose power connections
```

**2. Check stack sizes** (in source code):
```c
// Ensure tasks have sufficient stack
xTaskCreate(task, "task", 4096, NULL, 5, NULL);  // Increase from 2048
```

**3. Monitor heap usage**:
```bash
idf.py monitor | grep "free heap"

# If heap drops below 30KB:
# - Check for memory leaks
# - Reduce buffer sizes
# - Free unused resources
```

## Door Control Issues

### Door Won't Open or Close

**Symptoms**: Command sent but door doesn't move.

**Causes**:
1. State machine in wrong state
2. Relay rate limiting
3. Door physically blocked

**Solutions**:

**1. Check current state:**
```bash
idf.py monitor | grep "Door state"

# Valid transitions:
# CLOSED → OPENING
# OPEN → CLOSING
# STOPPED → OPENING or CLOSING

# If state is OPENING or CLOSING:
# - Wait for operation to complete or stop first
```

**2. Check rate limiting:**
```bash
# Default: minimum 1 second between activations
# If door was just activated, wait before sending new command

# Adjust rate limit (in relay_control.c):
relay_config_t config = {
    .min_interval_ms = 1000  // Reduce to 500ms for faster response
};
```

**3. Verify physical operation:**
- Test door with wall button
- Check for obstructions in door track
- Verify Genie opener is receiving power (LED on motor unit)
- Test relay activation manually (connect IN to 5V)

### Door Stops Mid-Operation

**Symptoms**: Door starts moving then stops, state becomes STOPPED.

**Causes**:
1. Operation timeout
2. Obstruction detected
3. Reed switch false trigger

**Solutions**:

**1. Check for timeout:**
```bash
idf.py monitor | grep "Operation timeout"

# If timeout occurs:
# - Default is 30 seconds
# - Adjust if door is slow:
  garage_door_set_timeout(45000);  // 45 seconds
```

**2. Check for obstruction detection:**
```bash
idf.py monitor | grep "Obstruction detected"

# If obstruction detected but door is clear:
# - Reed switch may be faulty or misaligned
# - Check switch wiring for noise
# - Adjust sensitivity (increase debounce time)
```

**3. Monitor reed switches during operation:**
```bash
# Observe door track
# Ensure reed switches only trigger at fully open/closed positions

# If switches trigger mid-operation:
# - Realign switches
# - Adjust magnet positions
# - Add shielding from vibration
```

### State Shows UNKNOWN

**Symptoms**: Door state never resolves to OPEN/CLOSED.

**Causes**:
1. Reed switches not working
2. Both reed switches triggered simultaneously
3. State machine initialization failed

**Solutions**:

**1. Test reed switches individually:**
```bash
# Manually trigger each switch
idf.py monitor | grep "Position changed"

# Move magnet near closed switch → Should see "CLOSED"
# Move magnet near open switch → Should see "OPEN"

# If no changes:
# - Check switch wiring (continuity test)
# - Verify GPIO configuration (pull-up enabled)
```

**2. Check for impossible states:**
```bash
# Both switches triggering simultaneously indicates:
# - Magnet interference
# - Incorrect switch placement
# - Electrical noise on GPIO lines

# Solutions:
# - Increase distance between reed switches (>6")
# - Add shielding (ground plane)
# - Filter with larger debounce time
```

**3. Clear NVS and reinitialize:**
```bash
# Factory reset to clear corrupted state
idf.py erase-flash
idf.py flash monitor
```

## Matter/Thread Issues

### Device Not Discoverable

**Symptoms**: QR code shows but Google Home/Apple Home can't find device.

**Causes**:
1. BLE not advertising
2. Matter stack not initialized
3. Device not powered

**Solutions**:

**1. Verify BLE advertising:**
```bash
# Check serial monitor for Matter init
idf.py monitor | grep "matter_device"

# Should see:
# I (xxx) matter_device: Matter initialized
# I (xxx) matter_device: QR Code: MT:YK90...

# If errors:
# - Check ESP-Matter SDK installation
# - Verify sdkconfig Matter settings
```

**2. Use BLE scanner app:**
```bash
# Install "nRF Connect" or "BLE Scanner" app
# Scan for "Smart Garage" or Matter device name

# If not found:
# - ESP32-H2 may have issue with BLE
# - Check if other BLE devices work
```

**3. Power cycle device:**
- Unplug and replug ESP32-H2
- Wait 10 seconds for BLE advertising to start
- Verify LED is slow blinking (commissioning mode)

### Thread Network Not Joining

**Symptoms**: Commissioning succeeds but device shows offline.

**Causes**:
1. Border router not reachable
2. Thread credentials wrong
3. RF interference

**Solutions**:

**1. Check border router status:**
```bash
# Google Nest Hub: Open Home app → Device → Thread network status
# Apple HomePod: Home app → HomeKit Thread Border Router

# Look for:
# - Status: Connected / Active
# - Network name and visible devices
```

**2. Verify Thread dataset:**
```bash
# Ensure dataset matches border router exactly
# From chip-tool:
./out/host/chip-tool otbr operational-dataset

# Compare with commissioning command
# All fields must match (network key, pan id, etc.)
```

**3. Improve RF link quality:**
- Move ESP32-H2 closer to border router (within 10 meters)
- Remove physical obstructions (metal, thick walls)
- Change ESP32-H2 channel (if configurable)
- Reduce 2.4GHz Wi-Fi interference (switch to 5GHz Wi-Fi)

### Device Goes Offline

**Symptoms**: Commissioned successfully but shows offline in app.

**Causes**:
1. Thread network partition
2. Device crash/restart
3. Border router issues

**Solutions**:

**1. Monitor device logs:**
```bash
idf.py monitor

# Look for:
# - Thread connectivity errors
# - Panic/reboot messages
# - Watchdog timeouts
```

**2. Check Thread partition status:**
```bash
# Use border router diagnostics
# Look for "Partitioned" or "Detached" devices

# If partitioned:
# - Move device closer to border router
# - Add Thread routers to extend mesh
# - Check for RF interference
```

**3. Restart border router:**
- Unplug and replug Thread border router
- Wait 2 minutes for network to reform
- Device should automatically reconnect

## Performance Issues

### High Latency (>500ms)

**Symptoms**: Delay between command and door movement.

**Causes**:
1. Thread network latency
2. Task priority issues
3. Rate limiting

**Solutions**:

**1. Check Task priorities**:
```c
// In garage_door_control.c
// Safety monitoring should be highest priority
xTaskCreate(safety_task, "safety", 2048, NULL, 7, NULL);  // Priority 7 (high)

// Matter event loop should be moderate
// Default is priority 5 (ESP-Matter default)
```

**2. Reduce rate limiting:**
```c
relay_config_t config = {
    .min_interval_ms = 500  // Reduce from 1000ms
};
```

**3. Monitor Thread latency:**
```bash
# Use chip-tool to measure command round-trip
./out/host/chip-tool windowcovering read current-position-lift-percent-100ths 1 0

# If >500ms response time:
# - Check RF link quality (RSSI)
# - Reduce Thread hop count
# - Move device closer to border router
```

### Memory Leaks

**Symptoms**: Heap continuously decreases, device eventually crashes.

**Diagnosis**:
```bash
idf.py monitor | grep "free heap"

# Watch heap value over time:
# t=0s: 245000 bytes free
# t=60s: 243000 bytes free  (-2KB/min)
# t=120s: 238000 bytes free  (-7KB/min) ← Leak detected
```

**Solutions**:

**1. Check for unfreed timers:**
```c
// Ensure timers are deleted when not needed
if (timer) {
    esp_timer_stop(timer);
    esp_timer_delete(timer);
    timer = NULL;
}
```

**2. Verify task cleanup:**
```c
// Tasks should delete own resources on exit
void task_to_delete(void *param) {
    // Clean up
    vSemaphoreDelete(mutex);
    
    // Delete self
    vTaskDelete(NULL);
}
```

**3. Use heap analysis tool:**
```bash
# Enable heap tracing in menuconfig
# Component config → Heap memory debugging → Enable heap tracing

# Generate heap snapshot
idf.py heap-trace
```

## Debugging

### Enable Detailed Logging

```bash
# Edit sdkconfig or use menuconfig
CONFIG_LOG_DEFAULT_LEVEL_INFO=y
CONFIG_LOG_MAXIMUM_LEVEL_DEBUG=y

# Rebuild and flash
idf.py build flash monitor
```

### Component-Specific Logging

```c
// Add debug logs to components
ESP_LOGD(TAG, "Debug message: value=%d", value);

// Temporarily change tag level at runtime
esp_log_level_set("garage_door", ESP_LOG_VERBOSE);
```

### GDB Debugging

```bash
# Build with debugging symbols
idf.py build

# Start GDB server
idf.py gdb

# In GDB:
(gdb) target remote :3333
(gdb) break app_main
(gdb) continue
```

### JTAG Debugging

If you have JTAG debugger:

```bash
# Connect JTAG to ESP32-H2
# OpenOCD configuration for ESP32-H2

idf.py openocd
# In another terminal:
idf.py gdb
```

### Serial Monitor with Timestamps

```bash
# Add timestamps to serial output for timing analysis
idf.py monitor | ts '%H:%M:%S'
```

## Common Error Messages

### "Guru Meditation Error"

**Cause**: Unhandled exception or illegal memory access.

**Solutions**:
1. Check for NULL pointer dereferences
2. Verify array bounds
3. Check stack overflow (increase stack size)
4. Use `esp_backtrace_print()` to trace crash location

### "Brownout"

**Cause**: Power supply voltage dropped below brownout threshold (~2.3V).

**Solutions**:
1. Increase power supply current rating
2. Add decoupling capacitors
3. Check for short circuits
4. Reduce power consumption (disable unnecessary peripherals)

### "Watchdog Timer Panic"

**Cause**: Task took too long to yield CPU.

**Solutions**:
1. Break long operations into smaller chunks
2. Add `vTaskDelay()` in loops
3. Increase watchdog timeout (not recommended as fix)
4. Check for high-priority tasks starving watchdog

### "Heap alloc failed"

**Cause**: Not enough free memory for allocation.

**Solutions**:
1. Reduce buffer sizes
2. Free unused memory
3. Use PSRAM if available (ESP32-H2 has 320KB internal SRAM)
4. Enable heap tracing to find leaks

## Getting Help

If issues persist:

1. **Check logs**: Always include serial monitor output in bug reports
2. **Describe setup**: Hardware version, ESP-IDF version, ESP-Matter version
3. **Minimal reproducible**: Can you reproduce the issue reliably?
4. **Isolate components**: Does issue occur with one component (reed switch, relay)?

Resources:
- [ESP32.com Forum](https://esp32.com/)
- [ESP-IDF Issues](https://github.com/espressif/esp-idf/issues)
- [ESP-Matter Issues](https://github.com/espressif/esp-matter/issues)