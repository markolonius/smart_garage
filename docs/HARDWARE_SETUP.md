# Hardware Setup Guide

This guide explains how to connect the ESP32-H2 smart garage door controller to your garage door opener system.

## Required Components

- ESP32-H2 development board (ESP32-H2-DevKitM-1 recommended)
- 2x Reed switches (normally open type with magnets)
- 1x Relay module (5V, dry contact)
- 5V DC power supply
- Jumper wires / Dupont cables
- Optional: Status LED (5mm)
- Optional: USB-to-serial adapter for flashing

## Wiring Diagram

### Reed Switches

Place reed switches on the garage door frame to detect when the door is fully open or fully closed.

| ESP32-H2 GPIO | Reed Switch | Connection |
|----------------|-------------|------------|
| GPIO 2 | Reed Switch (Closed) | Switch → GPIO 2, Ground → GND |
| GPIO 3 | Reed Switch (Open) | Switch → GPIO 3, Ground → GND |

**Notes**:
- Reed switches are active LOW (ground when magnet is near)
- Mount the "closed" reed switch at the bottom of the door track
- Mount the "open" reed switch at the top of the door track
- Place magnets on the door so they align with switches at each position
- Test switch operation by moving the door manually

### Relay

The relay provides a dry contact to trigger the garage door opener (simulating wall button press).

| ESP32-H2 GPIO | Relay | Connection |
|----------------|-------|------------|
| GPIO 4 | Relay IN | GPIO 4 → Relay IN |
| 5V | Relay VCC | 5V → Relay VCC |
| GND | Relay GND | GND → Relay GND |

**Relay NO/NC Connections to Genie Opener**:
- Connect relay **NO** (Normally Open) terminal to Genie opener button terminal 1
- Connect relay **COM** (Common) terminal to Genie opener button terminal 2
- This creates a momentary connection when relay is activated

**Important**: Most Genie openers use low-voltage (24V AC) for button inputs. Use a relay with appropriate voltage rating (typically 250V AC/30V DC).

### Power Supply

| Power Supply | ESP32-H2 | Relay |
|--------------|-----------|-------|
| 5V | 5V pin | 5V pin |
| GND | GND pin | GND pin |

**Notes**:
- Ensure power supply can provide at least 500mA (relay + ESP32-H2)
- If using external power for relay, connect grounds together

### Optional Status LED

| ESP32-H2 GPIO | LED | Connection |
|----------------|-----|------------|
| GPIO 8 | LED Anode (+) | GPIO 8 → Resistor (220Ω) → LED (+) |
| GND | LED Cathode (-) | LED (-) → GND |

**LED Behavior**:
- Solid: Door stationary (OPEN/CLOSED)
- Fast blink (250ms): Door moving (OPENING/CLOSING)
- Slow blink (1000ms): Not commissioned (BLE advertising)
- Off: Error state or STOPPED

## Complete Wiring Summary

```
ESP32-H2                    Components
--------                    ----------

GPIO 2  ──────┬────────── Reed Switch (Closed)
               │
               └─────── Magnet (on door)

GPIO 3  ──────┬────────── Reed Switch (Open)
               │
               └─────── Magnet (on door)

GPIO 4  ───────────────── Relay IN
5V       ───────────────── Relay VCC
GND      ───────────────── Relay GND
                          Relay NO/COM ── Genie Opener Button

GPIO 8  ─[220Ω]───────── LED (+) (optional)
GND      ───────────────── LED (-)

5V       ───────────────── Power Supply (+)
GND      ───────────────── Power Supply (-)
USB      ───────────────── Programming/Debug
```

## Genie Garage Door Opener Integration

### Finding Button Terminals

1. **Locate the wall button**: Find the wired button on your wall
2. **Trace wiring**: Follow wires from button to the opener motor unit
3. **Identify terminals**: Look for two terminals (usually labeled "1" and "2", or "button")
4. **Test with multimeter**: Measure voltage (should be ~24V AC)

### Connection Options

**Option 1: Parallel Connection (Recommended)**
- Connect relay in parallel with existing wall button
- Door can still be controlled via wall button
- Relay activation triggers door just like button press

**Option 2: Direct Connection**
- Disconnect wall button (if you want smart-only control)
- Connect relay directly to button terminals
- Not recommended - keeps wall button functional as backup

### Safety Precautions

⚠️ **IMPORTANT**: Garage doors can cause serious injury or property damage.

1. **Test door operation**: Before connecting relay, ensure door opens/closes smoothly
2. **Check safety sensors**: Verify photo-eye sensors are working (door reverses on obstruction)
3. **Test manually**: Use wall button to verify door operation
4. **Start with manual control**: After installation, test with wall button first
5. **Monitor first few cycles**: Watch door during first few smart control operations
6. **Keep wall button accessible**: Don't disable manual control option
7. **Install emergency release**: Ensure red emergency release cord is accessible

## Testing the Hardware

### Reed Switch Test

```bash
# Connect to serial monitor
idf.py monitor

# Move door manually and observe logs
# Should see reed switch state changes
```

Expected output:
```
I (1234) reed_switch: Initialized on pins 2 (closed), 3 (open)
I (2345) reed_switch: Position changed to 1 (CLOSED)
I (3456) reed_switch: Position changed to 2 (OPEN)
```

### Relay Test

```bash
# Use test program or manually activate via API
# Should hear relay click
# Use multimeter to verify NO/COM closure (should be ~500ms)
```

### Full System Test

1. Close door via wall button
2. Observe: reed switch reports CLOSED
3. Send open command via Matter or API
4. Observe: relay activates, door starts opening
5. Door reaches top, reed switch reports OPEN
6. Repeat for close command

## Troubleshooting

### Reed Switch Issues

**Switch not detecting door:**
- Check magnet alignment (should be within 10mm of switch)
- Verify switch is active LOW (connect to ground triggers detection)
- Test switch with multimeter (continuity check)

**False triggers:**
- Increase debounce time in code (default 50ms)
- Secure switch to prevent vibration
- Check for nearby magnetic interference

### Relay Issues

**Relay not activating:**
- Verify 5V power to relay module
- Check GPIO connection (multimeter test: should go HIGH when activating)
- Test relay with manual trigger (connect IN to 5V)

**Door not responding to relay:**
- Verify relay NO/COM connections to opener button terminals
- Check for loose wiring
- Test door with wall button to ensure opener is working
- Measure voltage at terminals (should be ~24V AC)

### Power Issues

**Device resets during operation:**
- Check power supply current rating (needs >500mA)
- Measure voltage at ESP32-H2 (should be 5V ±0.2V)
- Try shorter USB cable if powering via USB

**Brownout or unstable operation:**
- Add decoupling capacitors (100µF electrolytic + 0.1µF ceramic)
- Check for voltage drops when relay activates
- Consider separate power supplies for ESP32-H2 and relay

## Next Steps

After hardware setup is complete:
1. [Build and flash the firmware](../README.md#building)
2. [Commission with your smart home platform](COMMISSIONING.md)
3. Test end-to-end control (Matter command → door movement)

## Safety Certifications

⚠️ This is a DIY project. It is not UL certified and does not replace commercial garage door openers with built-in safety features.

Always:
- Keep door in sight when operating remotely
- Test emergency release regularly
- Disconnect power before working on wiring
- Follow local electrical codes
- Use proper wire gauge for voltage/current

## Additional Resources

- [ESP32-H2 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-h2_datasheet_en.pdf)
- [ESP32-H2 Pinout](https://docs.espressif.com/projects/esp-idf/en/latest/esp32h2/user-guides/boards.html)
- [Genie Opener Manual](https://www.geniecompany.com/support/)