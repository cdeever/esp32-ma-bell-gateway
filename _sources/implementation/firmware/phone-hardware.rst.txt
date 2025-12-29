SLIC Interface Monitoring
=========================

The SLIC interface module (``main/hardware/slic_interface.c``) provides real-time detection of telephone line events from the HC-5504B SLIC, including off-hook detection, ring status, and (future) dial pulse detection.

Overview
--------

The module interfaces with the SLIC's status output pins to monitor the physical telephone state and update the system state accordingly. This enables the ESP32 to respond to user actions on the telephone handset.

**Monitored Signals:**

- **Off-Hook Detection** (GPIO 32) - Detects when the telephone handset is lifted
- **Ring Status** (GPIO 33) - Monitors ring relay activation (future)
- **Dial Pulses** (GPIO 34) - Detects rotary dial pulses (future)

Off-Hook Detection
------------------

The primary function of the phone hardware module is detecting when the telephone handset is lifted (off-hook) or replaced (on-hook).

Hardware Connection
^^^^^^^^^^^^^^^^^^^

::

    HC-5504B SLIC                    ESP32-WROVER
    ┌────────────┐                  ┌─────────────┐
    │            │                  │             │
    │  Pin 13    ├──────────────────┤ GPIO 32     │
    │  (SHD)     │   Active LOW     │ (Input)     │
    │            │                  │             │
    └────────────┘                  └─────────────┘

- **SLIC Pin 13 (SHD):** Switch Hook Detect output
- **Signal Logic:** Active LOW (0V = off-hook, 3.3V = on-hook)
- **ESP32 GPIO 32:** Configured as input with internal pull-up resistor

GPIO Configuration
^^^^^^^^^^^^^^^^^^

The off-hook detect pin is configured during ``slic_interface_init()``:

.. code-block:: c

   gpio_config_t io_conf = {
       .pin_bit_mask = (1ULL << PIN_OFF_HOOK_DETECT),  // GPIO 32
       .mode = GPIO_MODE_INPUT,
       .pull_up_en = GPIO_PULLUP_ENABLE,   // Pull-up for safe default
       .pull_down_en = GPIO_PULLDOWN_DISABLE,
       .intr_type = GPIO_INTR_DISABLE      // Polling-based detection
   };

**Why pull-up resistor?**
The internal pull-up ensures the pin reads HIGH (on-hook) when the SLIC is not connected, preventing false off-hook detection during development or when the SLIC circuit is unpowered.

Polling Implementation
^^^^^^^^^^^^^^^^^^^^^^

The module uses a FreeRTOS task to poll the GPIO at regular intervals:

.. code-block:: c

   static void slic_monitor_task(void *arg)
   {
       while (1) {
           // Read GPIO level
           int gpio_level = gpio_get_level(PIN_OFF_HOOK_DETECT);
           bool current_hook_state = (gpio_level == 1);  // true = on-hook

           // Debounce and detect state changes
           // ... (see debouncing section)

           vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));  // 20ms
       }
   }

**Polling Parameters:**

- **Poll Interval:** 20ms (50 Hz update rate)
- **Task Priority:** 5 (moderate priority)
- **Stack Size:** 2048 bytes

**Alternative: Interrupt-Based Detection**

The current implementation uses polling for simplicity. For more efficient CPU usage, the module could be enhanced to use GPIO interrupts (``GPIO_INTR_ANYEDGE``) to detect state changes only when they occur.

Debouncing
^^^^^^^^^^

Mechanical telephone switches produce contact bounce, causing brief voltage fluctuations during state transitions. The module implements software debouncing to filter these transients:

.. code-block:: c

   #define HOOK_DEBOUNCE_MS 50

   if (current_hook_state != last_hook_state) {
       TickType_t now = xTaskGetTickCount();
       TickType_t elapsed_ms = pdTICKS_TO_MS(now - last_hook_change_time);

       if (elapsed_ms > HOOK_DEBOUNCE_MS) {
           // State change confirmed - update last_hook_state
           last_hook_change_time = now;
           // ... update system state
       }
   }

**Debounce Logic:**

1. Detect pin state change
2. Start debounce timer
3. Only accept change if pin remains stable for >50ms
4. Update state and log transition

This prevents spurious state changes from mechanical bounce or electrical noise.

State Updates
^^^^^^^^^^^^^

When a valid state change is detected, the module updates the global phone state:

.. code-block:: c

   if (current_hook_state) {
       // On-hook (handset replaced)
       ESP_LOGI(TAG, "Phone on-hook detected");
       ma_bell_state_update_phone_bits(0, PHONE_STATE_OFF_HOOK);
   } else {
       // Off-hook (handset lifted)
       ESP_LOGI(TAG, "Phone off-hook detected");
       ma_bell_state_update_phone_bits(PHONE_STATE_OFF_HOOK, 0);
   }

The ``ma_bell_state_update_phone_bits()`` function:

- Sets or clears the ``PHONE_STATE_OFF_HOOK`` bit in the phone state bitmask
- Logs the state change
- Sends notifications to registered tasks (via FreeRTOS task notifications)

For details on the state management system, see :doc:`state-management`.

Initialization
--------------

The SLIC interface module is initialized during the hardware subsystem startup:

**Call Hierarchy:**

::

   main()
     └─ hardware_init()          (main/hardware/hardware_init.c)
          └─ slic_interface_init()  (main/hardware/slic_interface.c)

**Initialization Steps:**

1. Configure GPIO 32 as input with pull-up
2. Read initial hook state
3. Initialize debounce state variables
4. Create monitoring FreeRTOS task
5. Log initialization status

**Error Handling:**

If GPIO configuration or task creation fails, ``slic_interface_init()`` returns an error code and logs the failure. The main initialization routine handles this with ``ESP_ERROR_CHECK()``, which will halt the system if SLIC interface initialization fails.

Diagnostic Output
-----------------

The module provides detailed logging for troubleshooting:

**Initialization Logs:**

.. code-block:: text

   I (1234) slic_if: Initializing SLIC interface monitoring
   I (1235) slic_if: GPIO 32 configured for off-hook detection
   I (1236) slic_if: Initial hook state: on-hook
   I (1237) slic_if: SLIC monitor task started
   I (1238) slic_if: SLIC interface monitoring started successfully

**Runtime Logs:**

.. code-block:: text

   I (5432) slic_if: Phone off-hook detected
   I (5434) ma_bell_state: Phone state changed: 0x00 -> 0x01

   I (12345) slic_if: Phone on-hook detected
   I (12346) ma_bell_state: Phone state changed: 0x01 -> 0x00

Web API Integration
-------------------

The off-hook state is exposed through the web interface at ``/status``:

.. code-block:: json

   {
     "phone": {
       "status": {
         "hook": "Off-hook",     // or "On-hook"
         "ringing": false,
         "ring_count": 0,
         "dialing": false,
         "last_digit": "None"
       }
     }
   }

The ``hook`` field updates in real-time based on the monitored GPIO state.

Testing
-------

Hardware Testing
^^^^^^^^^^^^^^^^

**Without SLIC (Development):**

1. Build and flash firmware
2. Monitor serial output
3. Short GPIO 32 to GND → Should log "Phone off-hook detected"
4. Release GPIO 32 → Should log "Phone on-hook detected"
5. Check ``/status`` endpoint → ``hook`` field should reflect current state

**With SLIC Connected:**

1. Connect telephone handset to SLIC
2. Lift handset → Should log "Phone off-hook detected"
3. Replace handset → Should log "Phone on-hook detected"
4. Verify no false triggers from line noise or ringing voltage

Software Testing
^^^^^^^^^^^^^^^^

**State Verification:**

.. code-block:: c

   const ma_bell_state_t* state = ma_bell_state_get();

   if (ma_bell_state_phone_bits_set(PHONE_STATE_OFF_HOOK)) {
       // Phone is currently off-hook
   }

**State Change Notifications:**

Tasks can register to receive notifications when phone state changes:

.. code-block:: c

   // Register for phone state notifications
   ma_bell_state_register_for_notifications(NOTIFY_PHONE_STATE_CHANGED);

   // Wait for notification
   uint32_t bits = ma_bell_state_wait_for_notification(
       NOTIFY_PHONE_STATE_CHANGED,
       5000  // 5 second timeout
   );

Ring Control
------------

The ESP32 controls the ringing cadence by driving the SLIC RC (Ring Command) pin. The SLIC does NOT generate the 90V AC ring signal—it only controls the relay that switches between normal battery feed and the external ring generator.

For hardware details on the ring generator circuit, see :doc:`../circuit/ring-generator`.

Hardware Connection
^^^^^^^^^^^^^^^^^^^

::

    ESP32-WROVER                    HC-5504B SLIC
    ┌─────────────┐                ┌────────────┐
    │             │                │            │
    │  GPIO 13    ├────────────────┤ Pin 16     │
    │  (Output)   │   Active LOW   │ (RC)       │
    │             │                │            │
    └─────────────┘                └────────────┘

- **ESP32 GPIO 13:** Ring Command output
- **SLIC Pin 16 (RC):** Ring Command input
- **Signal Logic:** Active LOW (0V = ring, 3.3V = idle)

GPIO Configuration
^^^^^^^^^^^^^^^^^^

The ring command pin is configured during initialization:

.. code-block:: c

   #define PIN_RING_COMMAND    GPIO_NUM_13

   gpio_config_t io_conf = {
       .pin_bit_mask = (1ULL << PIN_RING_COMMAND),
       .mode = GPIO_MODE_OUTPUT,
       .pull_up_en = GPIO_PULLUP_DISABLE,
       .pull_down_en = GPIO_PULLDOWN_DISABLE,
       .intr_type = GPIO_INTR_DISABLE
   };
   gpio_config(&io_conf);
   gpio_set_level(PIN_RING_COMMAND, 1);  // Start in idle state (HIGH)

Cadence Generation
^^^^^^^^^^^^^^^^^^

Standard North American ringing uses a 2 seconds ON, 4 seconds OFF cadence. The firmware generates this pattern:

.. code-block:: c

   #define RING_ON_MS          2000    // 2 seconds
   #define RING_OFF_MS         4000    // 4 seconds

   void ring_telephone(void)
   {
       int ring_count = 0;

       while (incoming_call_active && !phone_off_hook) {
           // Ring ON
           gpio_set_level(PIN_RING_COMMAND, 0);  // LOW = ring
           vTaskDelay(pdMS_TO_TICKS(RING_ON_MS));

           // Check for ring trip (off-hook)
           if (phone_off_hook) break;

           // Ring OFF
           gpio_set_level(PIN_RING_COMMAND, 1);  // HIGH = idle
           vTaskDelay(pdMS_TO_TICKS(RING_OFF_MS));

           ring_count++;
       }

       // Ensure ring is off
       gpio_set_level(PIN_RING_COMMAND, 1);
   }

**Cadence Parameters:**

- **Ring ON:** 2000ms (2 seconds)
- **Ring OFF:** 4000ms (4 seconds)
- **Cycle:** 6 seconds total

Ring Trip Detection
^^^^^^^^^^^^^^^^^^^

The SLIC automatically detects if the phone goes off-hook during ringing (ring trip). The firmware monitors GPIO 32 (SHD pin) to detect this condition and stop the ring sequence. When ``phone_off_hook`` becomes true, the ring loop exits and the ring command is deactivated.

This prevents the ringer from sounding after the user has answered the call.

Future Enhancements
-------------------

The current implementation provides a foundation for expanded phone hardware monitoring:

**Planned Additions:**

1. **Ring Detection (GPIO 33):**

   - Monitor SLIC RD pin for ring relay activation
   - Count ring cycles
   - Detect ring cadence patterns

2. **Pulse Dial Detection (GPIO 34):**

   - Decode rotary dial pulses (10 PPS typical)
   - Accumulate digit values (1-9 pulses = 1-9, 10 pulses = 0)
   - Implement inter-digit timeout detection

4. **Interrupt-Based Detection:**

   - Replace polling with GPIO edge interrupts
   - Reduce CPU overhead
   - Improve response time

5. **Event Publishing:**

   - Publish hook state changes to event system
   - Enable decoupled event handling
   - Support multiple event subscribers

Module Files
------------

**Source Files:**

- ``main/hardware/slic_interface.c`` - Implementation
- ``main/hardware/slic_interface.h`` - Public API

**Dependencies:**

- ``main/config/pin_assignments.h`` - GPIO pin definitions
- ``main/app/state/ma_bell_state.h`` - State management API
- FreeRTOS - Task management and delays
- ESP-IDF GPIO driver

**Build Integration:**

The module is included in the main component's CMakeLists.txt:

.. code-block:: none

   SRCS "hardware/slic_interface.c"
        ...

References
----------

- :doc:`state-management` - State management system documentation
- :doc:`architecture` - Overall firmware architecture
- ``docs/source/implementation/circuit/line-interface-hc5504b.rst`` - SLIC hardware documentation
- ``docs/source/implementation/circuit/pin-assignments.rst`` - GPIO pin assignments
