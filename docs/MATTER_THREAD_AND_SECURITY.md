# Thread Network & BLE Commissioning

Configuration and setup procedures for Thread networking and BLE commissioning for Smart Garage Door.

## Prerequisites

### Hardware Requirements
- ESP32-H2 with IEEE 802.15.4 radio (for Thread)
- ESP32-H2 or ESP32-S3 with BLE 5.0 (for commissioning)
- Thread Border Router (for Thread network formation and internet access)

### Software Requirements
- ESP-Matter SDK with Thread support
- chip-tool (Matter controller)
- ESP-IDF v5.1+

## Thread Network Setup

### 1. Thread Configuration Parameters

#### Operational Dataset
```
Active Operational Dataset:
- Network Name: Smart Garage Thread
- PAN ID: 0x1234567890abcdef (16 hex digits)
- Network Key: 00112233445566778899aabbccddeeff0 (32 hex digits)
- Channel: 15 (Channel 11-26 for 2.4 GHz)
- Mesh Local Prefix: fd00:0db8:0000:00ab:0:00
- Extended PAN ID: 0000db8000000000 (8 hex digits)
- PSKc: 0d7a9108220a920c1b2e8a9e07a9a (32 hex digits)
- Security Policy: 6800 (Thresh MAC filtering on)
- On Mesh Prefix: 0x00 (disabled)
- Leader Weight: 64 (default)
```

#### Border Router Configuration
```
Thread Border Router Settings:
- Radio Mode: Border Router (BR)
- Upstream Connection: Wi-Fi (for internet access)
- Border Router Role: SSED (Single SSED device allowed)
- Network ID: Smart-Garage-Mesh
- DHCPv6-PD: Disabled (not a DHCPv6 PD server)
```

### 2. Thread Network Initialization

#### Step 1: Initialize Thread Stack
```c
/* Thread stack initialization */
esp_err_t thread_stack_init(void) {
    /* ESP-Matter will initialize OpenThread stack automatically */
    /* When fully implemented, this will be handled by ESP-Matter SDK */

    /* For now, we can configure basic parameters */
    ESP_LOGI(TAG, "Thread stack initialization placeholder");
    return ESP_OK;
}
```

#### Step 2: Configure Operational Dataset
```c
/* Set operational dataset for Thread */
esp_err_t thread_set_operational_dataset(const char *dataset) {
    /* Parse hex-encoded dataset */
    /* Configure OpenThread with network credentials */
    ESP_LOGI(TAG, "Setting operational dataset");

    /* ESP-Matter will handle this when fully configured */
    return ESP_OK;
}
```

#### Step 3: Start Thread Network
```c
/* Start Thread device role */
esp_err_t thread_start(void) {
    /* Start Thread as border router or end device */
    ESP_LOGI(TAG, "Starting Thread network");

    /* Wait for Thread mesh formation */
    /* Monitor network connectivity */

    return ESP_OK;
}
```

### 3. Thread Network Commands

#### Console Commands
```
Thread Commands (via serial console):

matter esp thread start          - Start Thread network
matter esp thread stop           - Stop Thread network
matter esp thread dump             - Dump Thread status and metrics
matter esp thread ping <addr>     - Ping Thread device
matter esp thread diag-reset      - Reset Thread diagnostics
matter esp thread get <param>     - Get Thread parameter
matter esp thread set <param> <val> - Set Thread parameter
```

#### Thread Parameters
- `channel`: Radio channel (11-26)
- `panid`: Personal Area Network ID
- `networkkey`: Thread network key
- `networkname`: Network name
- `extpanid`: Extended PAN ID
- `meshprefix`: IPv6 mesh local prefix
- `keyindex`: Network key index

## BLE Commissioning Setup

### 1. BLE Configuration Parameters

#### Commissioning Data
```
Commissioning Configuration:
- Setup Passcode: 20202021 (default)
- Discriminator: 3840 (0x0F00)
- Vendor ID: 0xFFF1 (test vendor)
- Product ID: 0x8001
- Device Type: 0x0006 (Window Covering Device)
- Commissioning Flow: 0 (standard BLE-Wi-Fi)
- Discovery Capability: 0x02 (BLE enabled)
```

#### QR Code Format
```
QR Code Format: MT:Y.K9042C00KA0648G00

Decoding:
- MT: Matter protocol prefix
- Version: 0
- Vendor ID: 0xFFF1 (65521 decimal)
- Product ID: 0x8001 (32769 decimal)
- Custom Flow: 0 (standard commissioning)
- Discovery: 0x02 (BLE)
- Discriminator: 3840 (12 bits)
- Passcode: 20202021 (27 bits)
```

### 2. BLE Advertising

#### Advertisement Data
```
BLE Advertisement Data:
- Service UUID: 0000FDC-000F-4A23-1234-56789-ABCDE (Matter service)
- Device Type: 0x0006 (Window Covering Device)
- Service Data: Commissioning data
- Advertisement Interval: 100ms
- Tx Power: 0dBm

Matter Service Data Structure:
- Version: 0x00
- Device Identifier: Random 64-bit value
- Commissioner ID: Setup passcode
- Vendor ID: 0xFFF1
- Product ID: 0x8001
- Discriminator: 3840
```

#### BLE Advertisement Control
```c
/* Start BLE advertising for commissioning */
esp_err_t ble_start_advertising(void) {
    ESP_LOGI(TAG, "Starting BLE advertising");

    /* Advertise with Matter service UUID */
    /* Include commissioning data */

    /* ESP-Matter will handle this when configured */
    return ESP_OK;
}

/* Stop BLE advertising */
esp_err_t ble_stop_advertising(void) {
    ESP_LOGI(TAG, "Stopping BLE advertising");

    /* Stop BLE advertisement */

    return ESP_OK;
}
```

### 3. PASE Session

#### PAKE (Password Authenticated Key Exchange)
```
PASE Process:
1. Commissioner sends PAKE initiation request
2. Device responds with PASE setup
3. PAKE establishes shared secret
4. SPAKE2+ completes authentication
5. Session keys exchanged

Parameters:
- Iteration Count: 1000 (default)
- Salt: U1BBS0UyUCBLZXK (base64 encoded)
- Verifier: Random 256-bit value
```

### 4. Commissioning Workflow

#### Step-by-Step Process

```
Commissioning Flow:

1. Discovery (BLE)
   ├─ Device advertises on BLE
   ├─ Commissioner scans for Matter devices
   └─ Device identified by discriminator (3840)

2. PAKE Handshake
   ├─ Commissioner reads QR code or manual code
   ├─ Commissioner initiates PASE with device
   ├─ SPAKE2+ authentication completes
   └─ Secure session established

3. Credential Exchange
   ├─ Device provides operational dataset
   └─ Commissioner receives Thread credentials

4. Network Join
   ├─ Device receives Thread credentials
   ├─ Device initializes Thread stack
   └─ Device joins Thread network

5. Configuration
   ├─ Device obtains IPv6 address
   ├─ Device registers with Matter controller
   ├─ Access Control Lists (ACLs) configured
   └─ Commissioning complete

Total Time Target: < 3 minutes
```

#### Commissioning Commands
```bash
# BLE commissioning with manual code
chip-tool pairing code-thread <node-id> <ssid> <password> <manual-code>

# BLE commissioning with passcode
chip-tool pairing ble-thread <node-id> <operational-dataset> 20202021 3840

# Scan and commission (discover device first)
chip-tool pairing ble-thread <node-id> <ssid> <password> --discriminator 3840 --setup-code 20202021
```

## Security and Diagnostics

### 1. Device Attestation

#### PAA and PAI Certificates
```
Attestation Certificate Chain:
- PAA (Product Attestation Authority)
  └─ Issued by: Espressif (0xFFF1)
  └─ Trust Anchor: Built into Matter SDK

- PAI (Product Attestation Intermediate)
  └─ Issued by: Espressif for specific product line
  └─ Product ID: 0x8001

- DAC (Device Attestation Certificate)
  └─ Issued by: Manufacturer for specific device
  └─ Contains: Device public key + certificates

- CD (Certification Declaration)
  └─ Declares: Vendor ID, Product ID, software version
  └─ Required for: Production devices
```

#### Attestation Workflow
```
Device Attestation Process:

1. Certificate Generation
   ├─ Manufacturer generates public/private key pair
   ├─ CSR (Certificate Signing Request) created
   ├─ CSR signed by PAI
   ├─ DAC certificate issued
   └─ Device ID assigned

2. Certificate Installation
   ├─ DAC, PAI, PAA stored in secure storage
   ├─ CD configured for device
   └─ Ready for commissioning

3. Attestation During Commissioning
   ├─ Commissioner validates certificate chain
   ├─ PAI validates against PAA
   └─ DAC presented to commissioner
```

### 2. Access Control (ACL)

#### ACL Configuration
```
ACL Structure:
{
  "fabricIndex": 1,
  "privilege": 5,           // Admin privilege
  "authMode": 2,            // CASE (Certificate Authenticated Session)
  "subjects": [112233],      // Node ID of commissioner
  "targets": null              // All endpoints accessible
}
```

#### Access Levels
```
Privilege Levels:
- 0: View (read-only access)
- 1: Operator (can control device)
- 2: Manage (can configure device)
- 3: Admin (full access, can add/remove fabrics)
- 4: Provision (can commission new devices)
- 5: Root (unrestricted access)
```

#### ACL Management
```c
/* Add ACL entry */
esp_err_t add_acl_entry(uint16_t fabric_index, uint64_t node_id) {
    /* Configure access control for Matter controller */
    ESP_LOGI(TAG, "Adding ACL for fabric %u, node 0x%" PRIx64,
             fabric_index, node_id);

    /* ESP-Matter will handle this when configured */
    return ESP_OK;
}

/* Remove ACL entry */
esp_err_t remove_acl_entry(uint16_t fabric_index, uint64_t node_id) {
    /* Remove access control for Matter controller */
    ESP_LOGI(TAG, "Removing ACL for fabric %u, node 0x%" PRIx64,
             fabric_index, node_id);

    return ESP_OK;
}
```

### 3. Matter Event Logging

#### Event Types
```
Matter Event Types:
- Access Control Entry
- Access Control Exit
- Credential Revoked
- Credential Provisioned
- Start Up
- Shut Down
- Operational State Changed
- Door State Changed
```

#### Event Logging
```c
/* Log Matter event */
esp_err_t log_matter_event(uint16_t event_id, uint8_t *event_data) {
    /* Log event to diagnostics buffer */
    ESP_LOGI(TAG, "Matter event %u logged", event_id);

    /* ESP-Matter will handle this when configured */
    return ESP_OK;
}
```

### 4. Fail-Safe Timer

#### Configuration
```
Fail-Safe Settings:
- Timeout: 60 seconds
- Trigger: Door movement without reaching end state
- Action: Stop door immediately
- Reset: Manual command or new operation resets timer
```

#### Fail-Safe Implementation
```c
/* Reset fail-safe timer */
void reset_failsafe_timer(void) {
    /* Start 60-second timer on door movement */
    ESP_LOGI(TAG, "Fail-safe timer started");
}

/* Cancel fail-safe timer */
void cancel_failsafe_timer(void) {
    /* Stop timer when door completes */
    ESP_LOGI(TAG, "Fail-safe timer stopped");
}
```

### 5. Thread Diagnostics

#### Diagnostic Metrics
```
Thread Metrics:
- IPv6 Addresses: Global, Link-Local, Mesh-Local
- RLOC (Routing Locator): Thread mesh address
- Parent RSSI: Link quality to parent
- Parent LQI: Link quality indicator
- Child Count: Number of child devices (for routers)
- Route Table: Thread mesh routing entries
- Network Uptime: Time since Thread started
- Partition ID: Current Thread partition ID
- Channel: Current radio channel
```

#### Diagnostics Commands
```bash
# Get Thread diagnostics
matter esp thread dump

# Get Thread metrics
chip-tool basic read <node-id> 0x001F 0x0040 0x0042

# Monitor Thread sessions
chip-tool read <node-id> <endpoint-id> 0x003F 0x0028
```

## Configuration Files

### Thread Configuration (sdkconfig)
```
Thread Network Config:
CONFIG_OPENTHREAD_DEVICE_TYPE=y          # End Device or Router
CONFIG_OPENTHREAD_SSED=y                   # Sleepy End Device
CONFIG_OPENTHREAD_MESH_LOCAL_PREFIX=y       # Enable IPv6 prefix
CONFIG_OPENTHREAD_DEFAULT_CHANNEL=15          # Default channel
CONFIG_OPENTHREAD_BORDER_ROUTER=y           # Border router mode
CONFIG_OPENTHREAD_UPSTREAM_WIFI=y          # Wi-Fi upstream
```

### Matter Configuration (sdkconfig)
```
Matter Config:
CONFIG_ENABLE_CHIP_SHELL=y                # Enable Matter console
CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER=y   # Device info provider
CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER=y    # Factory data provider
CONFIG_SEC_CERT_DAC_PROVIDER=y            # DAC provider
CONFIG_ENABLE_OTA_REQUESTOR=y               # OTA support
CONFIG_CHIP_ENABLE_LOGGING=y               # Enable logging
CONFIG_LOG_LEVEL_INFO=y                     # Log level
```

### Factory Partition
```
Factory Data:
- Commissioning Dataset
- DAC Certificate
- PAI Certificate
- PAA Certificate (for validation)
- Certification Declaration (CD)
- Vendor Name: "Espressif"
- Product Name: "Smart Garage Door"
- Hardware Version: 1
- Serial Number: <device-specific>
```

## Commissioning Procedure

### Step-by-Step Guide

#### 1. Initial Setup
1. **Prepare Hardware**
   - Connect ESP32-H2 to USB/serial
   - Verify Thread antenna is connected
   - Ensure Wi-Fi access point is available

2. **Configure Border Router**
   - Set up Thread border router with Wi-Fi
   - Configure Thread network parameters
   - Verify router has internet connectivity

3. **Prepare Device**
   - Flash firmware to ESP32-H2
   - Factory reset NVS if needed
   - Power on device

#### 2. BLE Commissioning
1. **Generate QR Code**
   - Run: `matter esp onboardingcodes` on device console
   - Note: Manual code and QR code will be displayed
   - Verify code format is correct

2. **Scan for Device**
   - Use chip-tool: `chip-tool pairing ble-thread`
   - Or use Matter phone app to scan
   - Device should appear as "Matter Garage Door"

3. **Commission**
   - Scan QR code with Matter app
   - Select Thread network from app
   - Enter Wi-Fi credentials for border router
   - Commission device

#### 3. Verify Commissioning
1. **Check Device Logs**
   - Monitor serial output for commissioning success
   - Look for: "Commissioning complete"
   - Verify no errors occurred

2. **Verify Thread Network**
   - Check device joined Thread network
   - Verify IPv6 address is assigned
   - Check RLOC is registered

3. **Test Matter Control**
   - Use chip-tool: `chip-tool windowcovering go-to-percent <node-id> 1 100`
   - Verify door responds
   - Check state changes are reported

4. **Verify Subscriptions**
   - Subscribe to door position changes
   - Verify updates arrive in real-time
   - Check attribute values are correct

## Troubleshooting

### Common Issues

#### BLE Advertising Issues
**Problem**: Device not discoverable
- Check: BLE is started with `matter ble start`
- Verify: Service UUID is correct
- Check: Advertisement interval is reasonable (100-200ms)
- Solution: Restart BLE advertising

#### Thread Network Issues
**Problem**: Device won't join Thread network
- Check: Operational dataset is correct
- Check: Border router is powered on
- Verify: Thread channel is clear (no interference)
- Check: Network credentials match border router
- Solution: Reset Thread and retry commissioning

#### Certificate Issues
**Problem**: Attestation failure
- Check: DAC certificate is installed
- Check: CD is present
- Verify: Vendor ID and Product ID match certificates
- Solution: Flash proper factory partition

#### Fail-Safe Issues
**Problem**: Door stops unexpectedly
- Check: Timeout value is appropriate
- Verify: Timer is reset on completion
- Check: Door sensors are working correctly
- Solution: Adjust timeout or sensor sensitivity

## Verification Checklist

```
Thread Network Setup
□ Thread stack initializes correctly
□ Operational dataset configured
□ Thread device role set (End Device/Router)
□ Border router configured (if applicable)
□ Thread network joined
□ IPv6 address obtained
□ RLOC registered

BLE Commissioning
□ BLE advertising active
□ Service UUID correct
□ Discriminator accessible
□ QR code working
□ Manual code working
□ PASE handshake succeeds
□ Credentials exchanged
□ Commissioning complete

Security and Diagnostics
□ DAC certificate installed
□ PAI certificate valid
□ CD configured correctly
□ ACL configured for authorized controllers
□ Event logging enabled
□ Fail-safe timer configured (60s)
□ Diagnostics accessible

Device Certification
□ Vendor ID: 0xFFF1
□ Product ID: 0x8001
□ Software version: 0.1.0
□ Hardware version: 1
□ Production certificates ready
```

## References

- ESP-Matter Security Guide: https://docs.espressif.com/projects/esp-matter/en/latest/esp32/security.html
- Matter Security Specification: https://csa-iot.org/all-solutions/matter/
- Thread Specification: https://www.threadgroup.org/ThreadSpec
- OpenThread API Reference: https://openthread.io/documentation/group/openthread
