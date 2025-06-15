Pin Assignments
===============

ESP32-WROVER-IE Pin Configuration 
---------------------------------

The following tables list the GPIO pin assignments for the ESP32-WROVER-IE, grouped by function.

Phone Line & Audio Interface
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :widths: 12 18 50
   :header-rows: 1

   * - GPIO
     - Function
     - Description
   * - 25
     - PCM_FSYNC
     - I2S/PCM word select (frame sync, "LRCLK")
   * - 5
     - PCM_CLK_OUT
     - I2S/PCM bit clock ("BCLK")
   * - 26
     - PCM_DOUT
     - I2S/PCM audio out to phone line interface
   * - 35
     - PCM_DIN
     - I2S/PCM audio in from phone (if bidirectional)
   * - 32
     - Off-Hook Detect
     - Input from off-hook detection circuit
   * - 33
     - Ring Detect
     - Input from ring detection circuit
   * - 27
     - Dial Lamp Control
     - Output to dial lamp relay/MOSFET
   * - 34
     - Pulse Dial Input
     - Input from rotary pulse detection
   * - 39
     - DTMF Input
     - Input from DTMF decoder IC (if used)

Bluetooth Module
^^^^^^^^^^^^^^^^

.. list-table::
   :widths: 12 18 50
   :header-rows: 1

   * - GPIO
     - Function
     - Description
   * - 16
     - BT Module Reset
     - Output: Reset line for Bluetooth module
   * - 17
     - BT Power Control
     - Output: Power enable for BT module
   * - 1
     - UART0 TX
     - Debug serial output (do not use for app I/O)
   * - 3
     - UART0 RX
     - Debug serial input

User Interface
^^^^^^^^^^^^^^

.. list-table::
   :widths: 12 18 50
   :header-rows: 1

   * - GPIO
     - Function
     - Description
   * - 2
     - Status LED
     - Output: General status indication
   * - 4
     - User Button
     - Input: User pushbutton (pairing, reset, etc)
   * - 18
     - UI LED
     - Output: Additional status LED
   * - 21
     - I2C SDA (optional)
     - Data line for I2C display/expander (if used)
   * - 22
     - I2C SCL (optional)
     - Clock line for I2C display/expander

Power and Ground Pins
---------------------

**Power Input:**

- The Ma Bell Gateway is powered by a regulated **3.3 V DC supply**, provided to the ESP32-WROVER-IE module’s `3V3` pin.
- **Do not exceed 3.3 V** on this pin; the ESP32 is not 5V-tolerant.
- Power for the dial lamp, ringers, or other peripherals requiring higher voltages (such as 9V or 12V) should be supplied separately and isolated from ESP32 logic.  
  Use opto-isolators or level shifters to interface with the ESP32 where needed.

**Ground Reference:**

- All signals above are referenced to the ESP32’s **GND** pins.
- The ESP32-WROVER-IE module includes multiple GND pads—any may be used to connect the common ground for all circuits, including analog phone interface and Bluetooth module.
- Ensure all circuits within the Ma Bell Gateway share a common ground with the ESP32 for reliable operation.

Notes
-----

- GPIO6–11 are used internally for flash and must not be used for I/O.
- GPIO0, 12, and 15 are used for boot strapping; avoid using them for critical input or output.
- GPIO34–39 are input-only and are used for phone detection circuits.
- All grounds must be common for correct operation.
- All features and pin assignments listed here are specific to the Ma Bell Gateway project.

