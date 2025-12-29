# Commissioning Guide

This guide explains how to commission (onboard) your ESP32-H2 smart garage door controller to a Matter-compatible smart home platform.

## Prerequisites

1. **Hardware Setup**: Complete [HARDWARE_SETUP.md](HARDWARE_SETUP.md)
2. **Flash Firmware**: Build and flash firmware to ESP32-H2
3. **Thread Border Router**: Active Thread network with border router
   - Google Nest Hub (2nd gen)
   - Apple HomePod mini
   - Amazon Echo (4th gen)
   - Dedicated Thread border router

## Commissioning Overview

The ESP32-H2 uses BLE (Bluetooth Low Energy) for initial onboarding, then switches to Thread for ongoing communication. The commissioning process:

1. Device powers on and starts BLE advertising
2. Scan QR code or use chip-tool to discover device
3. Perform PASE (Password Authenticated Session Establishment)
4. Provision Thread network credentials
5. Device joins Thread network
6. BLE advertising stops (device now uses Thread only)

## Method 1: Google Home (Android)

### Step 1: Prepare Google Home App

1. Open Google Home app on Android device
2. Ensure your phone is on the same Wi-Fi network as your Thread border router
3. Bluetooth must be enabled on your phone

### Step 2: Add New Device

1. Tap **+** (plus icon) → **Set up device**
2. Select **New device**
3. Grant location permissions (required for BLE scanning)

### Step 3: Scan QR Code

1. Point your phone's camera at the QR code displayed in the serial monitor:
   ```bash
   idf.py monitor
   ```
   
   You'll see output like:
   ```
   I (1234) matter_device: QR Code: MT:YK90-5349-8810-8420-5986-4118
   I (1245) matter_device: Manual pairing code: 34970112345
   ```

2. Wait for Google Home to detect the device (may take 5-10 seconds)
3. Tap "Smart Garage Door" when it appears

### Step 4: Complete Setup

1. Follow the on-screen prompts
2. The app will automatically provision Thread credentials
3. Wait for device to join Thread network (LED stops slow blinking)
4. Name your device (e.g., "Garage Door")
5. Add to a room (e.g., "Garage")

### Step 5: Test Control

1. In Google Home app, tap the "Garage Door" tile
2. Tap the door icon to open/close
3. Verify the physical garage door responds
4. Check the status indicator matches actual door position

## Method 2: Apple Home (iOS)

### Step 1: Prepare Home App

1. Open Home app on iPhone or iPad
2. Ensure your device is on the same Wi-Fi network as your Thread border router
3. Bluetooth must be enabled

### Step 2: Add Accessory

1. Tap **+** (plus icon) → **Add Accessory**
2. Select "I don't have a Code or Can't Scan"

### Step 3: Scan QR Code

1. Open camera app on your iPhone
2. Point at the QR code from serial monitor output
3. Tap the notification that appears: "Matter Accessory"

4. **Alternative** - If QR code scanning fails:
   - In Home app, tap **Enter Code Manually**
   - Enter the 11-digit pairing code: `349-701-23456`

### Step 4: Complete Setup

1. Follow the on-screen prompts
2. Apple Home will automatically provision Thread credentials
3. Wait for device to join Thread network
4. Name your device (e.g., "Garage Door")
5. Add to a room

### Step 5: Test Control

1. In Home app, tap the "Garage Door" tile
2. Tap the door icon to open/close
3. Verify physical door responds
4. Check status indicator

## Method 3: chip-tool (CLI)

This method is for developers and advanced users.

### Step 1: Build chip-tool

```bash
# Assuming ESP-Matter SDK is set up
cd $ESP_MATTER/connectedhomeip/connectedhomeip
./scripts/examples/gn_build_example.sh examples/chip-tool out/host
```

### Step 2: Discover Device

```bash
# Start BLE commissioning
./out/host/chip-tool pairing ble-thread <node-id> hex:<thread-dataset> <pin-code> <discriminator>

# Example with default dataset (from serial monitor output):
./out/host/chip-tool pairing ble-thread 1 hex:0e080000000000010000000300000b35060004001fffe00208111111112222222222030c4f70656e5468726561640410275311c050810111820030d5a1a1ffbc0405e000b03f50624201000000000000000000 20202021 34970112
```

Parameters:
- `<node-id>`: Unique ID for this device (e.g., 1)
- `<thread-dataset>`: Thread network credentials (use border router's dataset)
- `<pin-code>`: Default 20202021 (or from serial monitor)
- `<discriminator>`: From serial monitor (e.g., 3497)

### Step 3: Get Thread Dataset from Border Router

**Google Nest Hub**:
```bash
# Access hub's Thread diagnostic page
# Thread Operational Dataset is listed there
```

**HomePod mini**:
```bash
# Home app → Home → HomeKit Thread Border Router → Thread Network Details
```

### Step 4: Verify Commissioning

```bash
# Read basic device info
./out/host/chip-tool basicinfo read 0 0 1

# Check door state (Window Covering cluster)
./out/host/chip-tool windowcovering read current-position-lift-percent-100ths 1 0
```

### Step 5: Control Door

```bash
# Open door (UpOrOpen command)
./out/host/chip-tool windowcovering up-or-open 1 0

# Close door (DownOrClose command)
./out/host/chip-tool windowcovering down-or-close 1 0

# Stop door (StopMotion command)
./out/host/chip-tool windowcovering stop-motion 1 0
```

## Commissioning Troubleshooting

### Device Not Found

**BLE advertising not starting:**
```bash
# Check serial monitor for errors
idf.py monitor

# Look for:
# - BLE initialization errors
# - Matter stack failures
# - GPIO configuration issues
```

**QR code not scanning:**
- Increase phone screen brightness
- Ensure QR code is clearly visible
- Check that phone's Bluetooth is on
- Try alternative: enter pairing code manually

**chip-tool can't discover:**
- Ensure chip-tool was built with BLE support
- Check that your computer has Bluetooth
- Reduce distance to ESP32-H2 (within 3 meters)
- Restart the ESP32-H2 (power cycle)

### Thread Network Issues

**Device won't join Thread network:**
- Verify Thread border router is powered on
- Check that border router has active Thread network
- Ensure border router is on same Wi-Fi as commissioning device
- Try reducing distance between ESP32-H2 and border router

**Provisioning fails:**
- Verify Thread dataset is correct (from border router)
- Check that Thread network supports device joining
- Ensure border router isn't at maximum device capacity
- Try factory reset: `storage_factory_reset()`

### Authentication Failures

**Invalid pin code:**
- Default is 20202021
- Check serial monitor for actual pin code
- Reset device if pin code was changed

**PASE handshake timeout:**
- Move commissioning device closer to ESP32-H2
- Reduce Wi-Fi interference (avoid 2.4GHz congestion)
- Restart commissioning process

## Factory Reset

If commissioning fails or you want to re-commission to a different network:

### Method 1: Button Factory Reset (Future)

Press and hold the factory reset button for 5 seconds (GPIO 0, to be implemented).

### Method 2: Command Factory Reset

```bash
# Connect via chip-tool and send factory reset command
./out/host/chip-tool generalcommissioning arm-fail-safe 120 1 0
./out/host/chip-tool generalcommissioning commissioning-complete 1 0
```

### Method 3: Erase NVS

```bash
# Erase all stored configuration
idf.py erase-flash
idf.py flash
```

## Multi-Controller Support

The device supports multiple Matter controllers simultaneously (e.g., Google Home + Apple Home):

### Commission to Google Home
1. Follow Google Home commissioning steps
2. Device is now controlled by Google Home

### Add Apple Home
1. Open Apple Home app
2. Add accessory
3. Scan QR code (same QR code works)
4. Device is now controlled by both controllers

**Note**: Both controllers will see the same device and can control it independently.

## Post-Commissioning Testing

After successful commissioning:

### 1. Verify Door State Reporting

```bash
# In Google Home app
# Open "Garage Door" tile → Status should match physical door

# In Apple Home app  
# Tap accessory → Status indicator
```

### 2. Test Remote Control

- Open door from app → Verify door opens
- Close door from app → Verify door closes
- Stop door from app → Verify door stops mid-operation

### 3. Test Automation

**Google Home Routines**:
1. Create routine: "When I arrive home" → "Open garage door"
2. Set location trigger to your home address
3. Test by driving away and returning

**Apple Home Automations**:
1. Create automation: "When someone arrives" → "Open garage door"
2. Set geofence to your home location
3. Test geofencing

### 4. Test Voice Control

**Google Assistant**:
```
"Hey Google, open the garage door"
"Hey Google, is the garage door open?"
```

**Siri**:
```
"Hey Siri, open the garage door"
"Is the garage door open?"
```

## Commissioning Status LEDs

The status LED indicates commissioning state:

| LED Pattern | Meaning |
|-------------|---------|
| Slow blink (1s) | Not commissioned, BLE advertising |
| Solid | Commissioned, Thread connected, door stationary |
| Fast blink (250ms) | Door moving (opening/closing) |
| Off | Error state or STOPPED |

## Next Steps

After commissioning:

1. [Test door operation](HARDWARE_SETUP.md#testing-hardware)
2. [Configure automations](#post-commissioning-testing)
3. [Set up monitoring](#post-commissioning-testing)
4. Review [Troubleshooting Guide](TROUBLESHOOTING.md)

## Additional Resources

- [Matter Commissioning Spec](https://csa-iot.org/developer-resource/specifications-download/)
- [Google Home Matter Support](https://support.google.com/googlenest/answer/10284125)
- [Apple HomeKit Thread Support](https://support.apple.com/guide/home/secure-your-home-connection-hmd49c3b3af4/ios)