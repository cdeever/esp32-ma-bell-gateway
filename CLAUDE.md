# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

The Ma Bell Gateway is an ESP32-based system that emulates a Central Office (CO) telephone line, enabling vintage rotary phones to make and receive calls via Bluetooth Hands-Free Profile (HFP) connection to a mobile phone. The project handles call signaling, audio transport, dial tone generation, and classic telephony features.

## Build System & Development Commands

This project uses ESP-IDF (Espressif IoT Development Framework) v5.0 or later.

### Essential Commands

```bash
# Set target (required first time)
idf.py set-target esp32

# Build the project
idf.py build

# Flash and monitor (replace /dev/ttyUSB0 with your serial port)
idf.py -p /dev/ttyUSB0 flash monitor

# Build only (no flash)
idf.py build

# Clean build artifacts
idf.py fullclean

# Monitor serial output only
idf.py -p /dev/ttyUSB0 monitor

# Open configuration menu
idf.py menuconfig
```

### Documentation

Documentation is built using Sphinx:

```bash
cd docs
make html
```

The built documentation is available at `docs/build/html/index.html`.

## Architecture Overview

### Code Organization

The project follows a modular architecture with clear separation of concerns and subsystem encapsulation:

```
main/
├── main.c                    # Application entry point (clean 70-line initialization)
├── config/                   # Centralized configuration headers
│   ├── bluetooth_config.h    # BT device name, PIN, task configs
│   ├── wifi_config.h         # WiFi credentials, timeouts
│   ├── web_config.h          # HTTP server settings
│   ├── audio_config.h        # I2S and audio parameters
│   └── system_config.h       # System-wide settings
├── app/                      # Application-level logic
│   ├── bluetooth/            # HFP message handling & command processing
│   ├── state/                # Centralized state management system
│   ├── web/                  # HTTP web interface for status monitoring
│   ├── events/               # Event system for subsystem communication
│   ├── tones.c               # Telephone tone generation (dial, busy, ringback, etc.)
│   └── pin_assignments.h     # Hardware pin definitions (PCM, GPIO, UART)
├── bluetooth/                # Bluetooth subsystem
│   ├── bt_init.c             # Complete BT initialization (fixes critical bug)
│   ├── bt_connection_manager.c # Connection & reconnection handling
│   ├── bt_app_core.c         # Work dispatcher & task management
│   └── bt_app_hf.c           # HFP client event handling
├── hardware/                 # Hardware abstraction layer
│   ├── hardware_init.c       # Hardware initialization wrapper
│   └── gpio_pcm_config.c     # PCM/I2S GPIO configuration
├── network/                  # Network connectivity
│   ├── wifi/                 # WiFi subsystem
│   │   ├── wifi_init.c       # WiFi initialization wrapper
│   │   └── wifi.c            # WiFi connection management
│   └── mqtt/                 # MQTT client (if enabled)
└── storage/                  # NVS (Non-Volatile Storage) abstraction
```

### Key Architectural Patterns

#### 1. Centralized Configuration (`main/config/`)

All configuration constants are consolidated into subsystem-specific headers:
- **bluetooth_config.h**: Device name ("MA BELL"), PIN code, task stack sizes, reconnection interval
- **wifi_config.h**: WiFi credentials, connection timeouts, retry counts
- **web_config.h**: HTTP server port, stack size, handler limits, timeouts
- **audio_config.h**: I2S port, sample rate, buffer sizes
- **system_config.h**: System-wide settings and initialization order documentation

This eliminates scattered #defines across 10+ files and makes configuration changes straightforward.

#### 2. Subsystem Initialization Pattern

Each major subsystem provides a clean initialization API:
```c
esp_err_t bluetooth_init(void);      // Initializes entire BT subsystem
esp_err_t wifi_init_and_connect(void); // Initializes WiFi and connects
esp_err_t hardware_init(void);       // Initializes all hardware peripherals
esp_err_t event_system_init(void);   // Initializes event pub/sub system
```

All initialization functions:
- Return `esp_err_t` for consistent error handling
- Log their progress clearly
- Handle internal dependencies automatically
- Encapsulate subsystem-specific complexity

**main.c is now a clean "table of contents"** (70 lines vs original 199):
```c
void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(event_system_init());
    ESP_ERROR_CHECK(ma_bell_state_init());
    ESP_ERROR_CHECK(storage_init());
    ESP_ERROR_CHECK(hardware_init());
    ESP_ERROR_CHECK(bluetooth_init());     // CRITICAL FIX inside
    ESP_ERROR_CHECK(wifi_init_and_connect());
    ESP_ERROR_CHECK(web_interface_init());
}
```

#### 3. Event System (`main/app/events/event_system.h`)

A lightweight publish/subscribe system enables decoupled subsystem communication:

**Events:**
- Bluetooth: `BT_EVENT_CONNECTED`, `BT_EVENT_DISCONNECTED`, `BT_EVENT_CALL_STARTED`, `BT_EVENT_CALL_ENDED`
- Phone: `PHONE_EVENT_OFF_HOOK`, `PHONE_EVENT_ON_HOOK`, `PHONE_EVENT_RINGING_START`
- Network: `WIFI_EVENT_CONNECTED_EV`, `WIFI_EVENT_DISCONNECTED_EV`, `WIFI_EVENT_IP_ACQUIRED_EV`
- System: `SYS_EVENT_ERROR`, `SYS_EVENT_LOW_BATTERY`

**Usage:**
```c
// Publishers (in subsystems)
event_publish(BT_EVENT_CONNECTED, NULL);

// Subscribers (in other subsystems or app code)
void my_callback(event_type_t event, void* user_data) {
    if (event & BT_EVENT_CONNECTED) {
        ESP_LOGI("APP", "Bluetooth connected!");
    }
}
event_subscribe(BT_EVENT_CONNECTED | BT_EVENT_DISCONNECTED, my_callback, NULL);
```

The event system supplements (not replaces) the state management system, allowing subsystems to react to changes without tight coupling.

#### 4. Centralized State Management (`main/app/state/ma_bell_state.h`)

The state management system provides a unified way to track system state across four domains:
- **Phone state**: Off-hook, ringing, dialing, tone playback
- **Bluetooth state**: Connection status, call state, audio state
- **Network state**: WiFi connection, IP acquisition, web server status
- **System state**: Initialization, errors, battery level

States are managed using bitmasks for efficient representation. The system supports task notifications for state change awareness, enabling FreeRTOS tasks to wait for specific state transitions.

**Usage pattern:**
```c
// Check if Bluetooth is connected
if (ma_bell_state_bluetooth_bits_set(BT_STATE_CONNECTED)) {
    // Connected logic
}

// Update state bits
ma_bell_state_update_phone_bits(PHONE_STATE_OFF_HOOK, 0);

// Wait for state changes
uint32_t notif = ma_bell_state_wait_for_notification(NOTIFY_BT_STATE_CHANGED, 1000);
```

#### 5. Bluetooth Subsystem Architecture

**Critical Bug Fix:** The original code never called `bt_app_task_start_up()`, causing BT message queue failures. This is now fixed in `bluetooth_init()` (bt_init.c:60).

**Bluetooth Work Dispatcher Pattern** (`main/bluetooth/bt_app_core.c`):
The Bluetooth stack uses a work dispatcher pattern to handle events asynchronously. Events from the ESP-IDF Bluetooth stack are dispatched to a dedicated FreeRTOS task via a message queue, ensuring proper context switching and avoiding callback execution in ISR context.

**Connection Manager** (`main/bluetooth/bt_connection_manager.c`):
Handles GAP events, device discovery, pairing, and automatic reconnection. Extracted from main.c (150+ lines) for better separation of concerns.

**Initialization Order** (critical - enforced in bt_init.c):
1. BT controller init
2. Bluedroid init
3. **bt_app_task_start_up()** ← MUST be called before callbacks
4. Device name/PIN setup
5. HFP client init
6. Callback registration
7. Connection manager init

#### 3. Storage Abstraction Layer (`main/storage/storage.h`)

NVS (Non-Volatile Storage) is abstracted with namespaces:
- `wifi`: SSID, password
- `bt`: Device name, paired device info
- `sys`: Volume, ring volume

This abstraction simplifies storing/retrieving configuration across the system.

#### 4. Telephone Tone Generation (`main/app/tones.c`)

The system generates authentic North American telephone tones using I2S/PCM:
- Dial tone (350Hz + 440Hz continuous)
- Ringback (440Hz + 480Hz, 2s on / 4s off)
- Busy signal (480Hz + 620Hz, 0.5s on/off)
- Reorder tone (fast busy: 480Hz + 620Hz, 0.25s on/off)
- Off-hook warning, call waiting, SIT tones

Each tone is defined with dual frequencies and on/off cadence patterns.

### Hardware Interface

#### PCM/I2S Audio (defined in `main/app/pin_assignments.h`)
- `PIN_PCM_FSYNC` (GPIO25): Word select / frame sync
- `PIN_PCM_CLK_OUT` (GPIO5): Bit clock
- `PIN_PCM_DOUT` (GPIO26): Audio output to phone line
- `PIN_PCM_DIN` (GPIO35): Audio input from phone

#### Phone Line Detection
- `PIN_OFF_HOOK_DETECT` (GPIO32): Off-hook detection circuit input
- `PIN_RING_DETECT` (GPIO33): Ring detection
- `PIN_DIAL_LAMP_CTRL` (GPIO27): Dial lamp control output
- `PIN_PULSE_DIAL_IN` (GPIO34): Rotary dial pulse detection

### Initialization Sequence (from `main/main.c`)

1. NVS flash initialization
2. Bluetooth controller initialization (Classic BT only, BLE disabled)
3. State management system initialization
4. GPIO and PCM configuration
5. WiFi station mode initialization
6. Web interface startup
7. HFP client profile registration
8. Bluetooth reconnection task creation

The reconnection task (`bt_reconnect_task`) continuously monitors Bluetooth connectivity and attempts to reconnect to the last paired device when disconnected.

### Bluetooth Pairing & Reconnection

- **Default PIN**: "0000" (configured in `main.c`)
- **Device Name**: "MA BELL"
- **Pairing storage**: Paired device info (address + name) is stored in NVS via `app_hf_store_paired_device()`
- **Auto-reconnection**: The system periodically scans for the last paired device and attempts reconnection every 10 seconds

### Configuration

Important build configurations are in `sdkconfig.defaults`:
- Classic Bluetooth enabled, BLE disabled
- Hands-Free Profile (HFP) client enabled
- Single synchronous connection supported

WiFi credentials are configured at compile time (see `main/network/wifi/wifi.h` for defaults) or can be stored in NVS.

### Web Interface

The system provides an HTTP web interface for monitoring status. It displays:
- Phone state (on-hook/off-hook, dialing, tones)
- Bluetooth connection status
- Network connectivity
- System health

The web server runs on the ESP32's WiFi interface when connected.

## Development Notes

### Adding New Subsystems

To add a new subsystem (e.g., MQTT, OTA updates, sensor integration):

1. **Create subsystem directory**: `main/network/mqtt/` or `main/sensors/`
2. **Create init module**: `mqtt_init.h` / `mqtt_init.c` with `mqtt_init()` function
3. **Add configuration**: Create `main/config/mqtt_config.h` with all constants
4. **Publish events**: Use `event_publish()` to announce subsystem state changes
5. **Update CMakeLists.txt**: Add new source files and include directories
6. **Call from main.c**: Add `ESP_ERROR_CHECK(mqtt_init())` in appropriate order
7. **Update CLAUDE.md**: Document the new subsystem

### Changing Configuration

All configuration is in `main/config/*.h` files:
- **Bluetooth settings**: Edit `bluetooth_config.h` (device name, PIN, timeouts)
- **WiFi credentials**: Edit `wifi_config.h` (SSID, password, retry count)
- **Web server**: Edit `web_config.h` (port, stack size, timeouts)
- **Audio parameters**: Edit `audio_config.h` (sample rate, buffer size)
- **Pin assignments**: Edit `main/app/pin_assignments.h` (GPIO numbers)

No need to search through multiple .c files - everything is centralized.

### Adding New Telephone Tones

1. Define the tone in `main/app/tones.h` enum
2. Add tone parameters (frequencies, on/off cadence) to the `tones[]` array in `main/app/tones.c`
3. Configuration like sample rate and buffer size are in `config/audio_config.h`

### Adding State Tracking

1. Define new state bitmasks in `main/app/state/ma_bell_state.h`
2. Update the relevant state category (phone/bluetooth/network/system)
3. Use `ma_bell_state_update_*_bits()` to modify state
4. Publish corresponding events with `event_publish()` for other subsystems to react
5. Tasks can register for notifications and wait for state changes

### Using the Event System

**To publish an event:**
```c
#include "app/events/event_system.h"

event_publish(BT_EVENT_CONNECTED, NULL);
```

**To subscribe to events:**
```c
void my_handler(event_type_t event, void* user_data) {
    ESP_LOGI("MYAPP", "Event received: 0x%x", event);
}

event_subscribe(BT_EVENT_CONNECTED | BT_EVENT_DISCONNECTED, my_handler, NULL);
```

Events are delivered in the publisher's context (simple, synchronous callbacks).

### ESP-IDF Version Compatibility

The project requires ESP-IDF v5.0 or later. Ensure the `IDF_PATH` environment variable is set and the ESP-IDF tools are activated:

```bash
. $IDF_PATH/export.sh
```

### Documentation Structure

The Sphinx documentation (`docs/source/`) includes:
- Historical perspective on telephony
- Solution design (circuit diagrams, enclosure)
- Implementation details (state management, signaling)
- Usage and standards

When modifying documentation, follow the existing reStructuredText format and rebuild with `make html`.
