Firmware Architecture
=====================

Design Principles
-----------------

- **Separation of Concerns:** Each module focuses on a single responsibility—hardware, protocol, state, business logic, or UI.
- **Explicit APIs:** Modules communicate through clear interfaces.
- **Scalability:** Structure supports adding new features (web endpoints, hardware interfaces, state transitions) without creating spaghetti code.
- **Testability:** Isolated logic is testable without hardware.

Directory Layout
----------------

The Ma Bell Gateway firmware is organized into logical modules and directories:

.. code-block:: none

   main/
     app/              # Application logic and event coordination
       state/          # Centralized state management (ma_bell_state.c)
       bluetooth/      # HFP message handling (app_hf_msg_set.c)
       web/            # HTTP web interface (web_interface.c)
       events/         # Event publish/subscribe system (event_system.c)
     audio/            # Audio subsystem
       audio_output.c  # I2S TX/RX, tone generation, audio write API
       audio_bridge.c  # BT↔Phone ring buffers and bridging tasks
       tones.c         # Telephone tone definitions
     bluetooth/        # Bluetooth stack integration
       bt_init.c       # BT subsystem initialization
       bt_app_core.c   # Work dispatcher and task management
       bt_app_hf.c     # HFP client event handling, audio callbacks
       bt_connection_manager.c  # GAP, pairing, reconnection
     config/           # Centralized configuration
       audio_config.h  # I2S and audio parameters
       bluetooth_config.h  # BT device name, PIN, timeouts
       pin_assignments.h   # GPIO pin definitions
       wifi_config.h   # WiFi credentials
     hardware/         # Hardware abstraction
       hardware_init.c     # Hardware initialization wrapper
       gpio_pcm_config.c   # PCM/I2S GPIO configuration
       slic_interface.c    # SLIC monitoring (off-hook, etc.)
     network/          # Network connectivity
       wifi/           # WiFi subsystem
       mqtt/           # MQTT client (optional)
     storage/          # NVS abstraction
     main.c            # Application entry point
     CMakeLists.txt

Module Responsibilities
-----------------------

**app/**
  - Contains the application's core logic, including the state machine, event system, and all "business logic."
  - ``state/`` - Centralized state management with bitmask-based state tracking
  - ``events/`` - Lightweight publish/subscribe event system
  - ``web/`` - HTTP web interface for status monitoring
  - Coordinates between hardware, Bluetooth, network, and user interfaces.

**audio/**
  - Manages bidirectional audio between phone handset and Bluetooth:
    - ``audio_output.c`` - I2S TX/RX initialization, tone generation task, audio write API
    - ``audio_bridge.c`` - Ring buffer management, BT↔Phone bridging tasks
    - ``tones.c`` - Telephone tone definitions (frequencies, cadences)
  - Uses HCI audio path for software control over Bluetooth audio
  - See :doc:`audio-subsystem` for detailed documentation.

**bluetooth/**
  - Implements all Bluetooth functionality:
    - ``bt_init.c`` - Complete BT subsystem initialization
    - ``bt_app_hf.c`` - HFP client event handling and audio data callbacks
    - ``bt_connection_manager.c`` - GAP events, pairing, auto-reconnection
    - ``bt_app_core.c`` - Work dispatcher pattern for async event handling
  - Provides a clear API for application modules to initiate or respond to Bluetooth events.

**config/**
  - Centralized configuration headers:
    - ``audio_config.h`` - I2S port, sample rate, buffer sizes
    - ``bluetooth_config.h`` - Device name, PIN, task configs
    - ``pin_assignments.h`` - All GPIO pin definitions
    - ``wifi_config.h`` - WiFi credentials and timeouts

**hardware/**
  - Manages interaction with the physical hardware:
    - ``hardware_init.c`` - Hardware subsystem initialization wrapper
    - ``gpio_pcm_config.c`` - PCM/I2S GPIO matrix configuration
    - ``slic_interface.c`` - SLIC monitoring (off-hook detection)
  - Abstracts hardware details from application logic.
  - See :doc:`phone-hardware` for details on SLIC interface monitoring.

**network/**
  - Responsible for network connectivity and device management:
    - Wi-Fi provisioning, status, and reconnection
    - Web server and REST API endpoints for status monitoring, configuration, and control

**storage/**
  - Provides persistent storage for configuration data (such as paired device info and user settings)  
  - Abstracts the ESP32’s NVS (non-volatile storage) details behind a simple interface

**platform/**
  - Contains project entry points and ESP32/RTOS glue:
    - Main firmware boot/init
    - Task registration and synchronization
    - Component registration (build system integration)

**include/**
  - Optionally holds public header files for modules that require global access  
  - Helps decouple modules and simplify dependency management

Development Guidelines
----------------------

- **New features** should be added in the most appropriate module or directory—never spread logic across unrelated areas.
- **APIs** between modules must be explicit and documented in header files.
- **Hardware and protocol details** should be abstracted away from core application logic.
- **All project documentation and this architecture page should be updated as you refactor and extend the codebase.**

By following this structure, the Ma Bell Gateway firmware will be maintainable, testable, and easy for new contributors to understand.

