Ring Generator
==============

This page documents the ring generator implementation for the Ma Bell Gateway, which produces the 90V AC 20Hz signal required to ring vintage electromechanical telephone bells.

Overview
--------

**Requirements:**

- **Voltage:** 90V RMS (approximately 127V peak, 254V peak-to-peak)
- **Frequency:** 20 Hz (North American standard)
- **Cadence:** 2 seconds ON, 4 seconds OFF (controlled by ESP32)
- **Current:** 20-40 mA RMS per ringer
- **Power:** 2-4W when active

**Design Choice:**

The Ma Bell Gateway uses a **commercial telecom ring generator module** rather than a custom oscillator/transformer design. This matches the overall design philosophy of using dedicated ICs for each function (like the HC-5504B SLIC), providing reliable operation without custom analog circuit design.

For alternative approaches (555 timer, ESP32 PWM, etc.), see the design documentation at :doc:`/solution-design/circuit/ringer`.

How Ringing Works
-----------------

The ringing system involves coordination between the ESP32, SLIC, ring generator module, and relay:

::

    ┌─────────────────────────────────────────────────────────────────────────────┐
    │                           Ring Signal Flow                                   │
    └─────────────────────────────────────────────────────────────────────────────┘

                                                    Normal State (On-Hook)
                                                    ┌─────────────────────┐
                                                    │  -48V Battery Feed  │
                                                    │  (from SLIC)        │
                                                    └──────────┬──────────┘
                                                               │
    ┌──────────┐    GPIO 13     ┌──────────┐    RD Pin    ┌────┴────┐
    │  ESP32   ├───────────────►│  SLIC    ├─────────────►│  Ring   │
    │          │    (RC Pin)    │ HC-5504B │    (Relay    │  Relay  │
    │          │                │          │    Driver)   │  DPDT   │
    └──────────┘                └──────────┘              └────┬────┘
                                                               │
                                                    Ring State │
                                                    ┌──────────┴──────────┐
    ┌──────────┐                                    │                     │
    │   48V    ├───────────────►┌──────────┐       │    ┌───────────┐    │
    │  Supply  │                │  Ring    ├───────┴───►│ Telephone │    │
    │          │                │  Gen     │  90V AC    │  Ringer   │    │
    └──────────┘                │  Module  │  20Hz      └───────────┘    │
                                └──────────┘                             │
                                                                         │
                                              Tip/Ring (green/red) ◄─────┘


**Sequence:**

1. **Incoming call detected** - ESP32 receives ring notification via Bluetooth HFP
2. **Ring command** - ESP32 drives GPIO 13 LOW (SLIC RC pin)
3. **Relay activation** - SLIC RD pin goes LOW, activating external ring relay
4. **Voltage switch** - Relay disconnects -48V battery and connects 90V AC generator
5. **Bell rings** - 20Hz AC signal rings the electromechanical bell
6. **Cadence control** - ESP32 toggles GPIO 13: 2s ON, 4s OFF
7. **Ring trip** - If phone goes off-hook during ring, SLIC detects it automatically

SLIC Ring Control Interface
---------------------------

The HC-5504B SLIC provides ring **control**, not ring **generation**:

.. list-table::
   :header-rows: 1
   :widths: 15 15 15 55

   * - SLIC Pin
     - Name
     - Direction
     - Function
   * - 16
     - RC
     - Input
     - Ring Command - Drive LOW to activate ring mode
   * - 8
     - RD
     - Output
     - Ring Relay Driver - Goes LOW when ring active (drives relay)
   * - 3
     - RFS
     - Input
     - Ring Feed Sense - For ring trip detection

**Important:** The SLIC does NOT generate the 90V AC signal. It only:

- Accepts the ring command from ESP32 (RC pin)
- Drives an external relay to switch the line (RD pin)
- Detects ring trip (off-hook during ringing)

Ring Generator Module
---------------------

**Selected Component: Commercial Telecom Ring Generator**

For the Ma Bell Gateway, we recommend a commercial ring generator module designed for telecom applications:

**Specifications to look for:**

.. list-table::
   :header-rows: 1
   :widths: 25 35 40

   * - Parameter
     - Requirement
     - Notes
   * - Input Voltage
     - 48V DC (or 24-60V range)
     - Must work with our 48V supply
   * - Output Voltage
     - 85-90V RMS (120-254V p-p)
     - Standard telephone ring level
   * - Output Frequency
     - 20 Hz (±1 Hz)
     - North American standard
   * - Output Waveform
     - Sine wave preferred
     - Some modules output trapezoidal
   * - Output Current
     - 40 mA minimum
     - Supports 1-2 ringers
   * - Control Input
     - Enable/disable pin
     - For relay or direct control

**Example Modules:**

- **Beta Dyne RG3000 series** - Professional telecom ring generator, 48V input, configurable output
- **Custom telecom DC-AC inverter modules** - Available from specialty suppliers
- **12V modules with boost** - Some 12V ring generators can be adapted with a 48V→12V stage

**Why Commercial Module:**

- Matches SLIC-based design philosophy (dedicated ICs)
- No custom transformer winding or oscillator tuning
- Known specifications and reliability
- Faster development time

Complete Ring Circuit
---------------------

**Block Diagram:**

::

    48V Supply ────┬──────────────────────────────────────────────────────►  To SLIC VB+
                   │
                   │     ┌─────────────────────┐
                   └────►│  Ring Generator     │
                         │  Module             │
                         │                     │
                         │  48V DC → 90V AC    │
                         │          20Hz       │
                         └──────────┬──────────┘
                                    │
                                    │ 90V AC Output
                                    │
                                    ▼
                         ┌─────────────────────┐
                         │    Ring Relay       │
                         │      (DPDT)         │◄────── SLIC RD Pin (via driver)
                         │                     │
                         │  NC: -48V (normal)  │
                         │  NO: 90V AC (ring)  │
                         └──────────┬──────────┘
                                    │
                                    │ Switched Output
                                    ▼
                              To SLIC Tip/Ring
                              (Phone Line)


**Relay Driver Circuit:**

The SLIC RD pin can sink current but may need a driver transistor for larger relays:

::

                                    +12V (or relay coil voltage)
                                      │
                                      │
                                    ┌─┴─┐
                                    │   │ Relay Coil
                                    │   │ (DPDT, 12V)
                                    └─┬─┘
                                      │
                    D1 (1N4148)       │
                   ┌──────┤◄├─────────┤  Flyback diode
                   │                  │
                   │                ┌─┴─┐
                   │                │ C │
                   │          ┌─────┤   │ Q1 (2N2222 or similar)
                   │          │     │ E │
                   │          │     └─┬─┘
                   │          │       │
    SLIC RD ───────┴──[R1]────┴───────┴───── GND
    (Pin 8)           1kΩ


**Note:** If using a small relay (< 50mA coil), the SLIC RD pin may drive it directly. Check the HC-5504B datasheet for RD pin current capability.

**Relay Selection:**

- **Type:** DPDT (Double Pole, Double Throw)
- **Coil Voltage:** 12V DC (matches SLIC VB+ rail)
- **Contact Rating:** 1A @ 125V AC minimum
- **Example:** Omron G5V-2, Panasonic TQ2-12V

Cadence Control
---------------

The ESP32 firmware controls ring cadence by toggling GPIO 13 (SLIC RC pin):

**Standard North American Cadence:**

- 2 seconds ring ON
- 4 seconds ring OFF
- Repeat until answered or caller hangs up

The firmware implementation handles cadence timing, ring trip detection (when user answers during ring), and integration with the Bluetooth HFP call state.

For firmware implementation details including GPIO configuration and sample code, see :doc:`../firmware/phone-hardware`.

Safety Considerations
---------------------

.. warning::

   **90V AC is hazardous!** The ring voltage can cause painful shock and injury. Follow proper safety practices when working with this circuit.

**Electrical Safety:**

- Use properly rated components (125V AC minimum for relay contacts)
- Ensure adequate creepage/clearance on PCB (2mm minimum for 90V)
- Include fusing on the ring generator output (100mA fast-blow)
- Never touch tip/ring connections when ring voltage may be present

**Isolation:**

- The ring generator module should provide galvanic isolation between 48V input and 90V output
- The SLIC provides isolation between high-voltage line circuits and low-voltage ESP32 logic

**Protection:**

- MOV across relay contacts to suppress switching transients
- Flyback diode across relay coil (required)
- Current limiting in ring generator module

Bill of Materials
-----------------

.. list-table::
   :header-rows: 1
   :widths: 25 20 20 35

   * - Component
     - Value/Type
     - Qty
     - Notes
   * - Ring Generator Module
     - 48V→90VAC 20Hz
     - 1
     - Commercial telecom module
   * - Ring Relay
     - DPDT, 12V coil
     - 1
     - Omron G5V-2 or equivalent
   * - Flyback Diode
     - 1N4148
     - 1
     - Across relay coil
   * - Driver Transistor
     - 2N2222 or 2N3904
     - 1
     - If needed for relay drive
   * - Base Resistor
     - 1kΩ
     - 1
     - For transistor driver
   * - Fuse (output)
     - 100mA fast-blow
     - 1
     - Ring circuit protection
   * - MOV (optional)
     - 150V
     - 1
     - Relay contact protection

References
----------

- :doc:`/solution-design/circuit/ringer` - Ring signal design alternatives
- :doc:`power-supply` - 48V power source for ring generator
- :doc:`line-interface-hc5504b` - SLIC pinout and ring control interface
- `Analog Devices Design Note 134 <https://www.analog.com/en/resources/design-notes/telephone-ring-tone-generation.html>`_ - Telephone ring tone generation
- `Beta Dyne Ring Generator Application Note <http://www.betadynepower.com/wp-content/uploads/2017/11/RG3000_UNDERSTANDING-RING-GENERATORS.pdf>`_ - Understanding ring generators
