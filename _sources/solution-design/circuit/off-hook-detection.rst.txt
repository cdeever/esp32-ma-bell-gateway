Off-Hook Detection
==================

An **off-hook detection circuit** is required to sense when a telephone receiver is lifted—completing the subscriber loop between tip and ring—so that the system can recognize an attempt to place or answer a call.

.. note::
   **Safety:** The subscriber loop may carry up to 48V DC and 90V AC ringing. All detection methods must provide robust isolation to protect the ESP32 and the user. Never connect microcontroller logic directly to line voltage.

System Behavior
---------------

When the phone goes **off-hook** (receiver lifted), the Ma Bell Gateway:

- Senses the closed loop and increased current via the off-hook detection circuit
- Activates dial illumination
- Starts dial tone generation
- Prepares the Bluetooth subsystem for call placement or to answer incoming calls

When the phone returns **on-hook** (receiver replaced), the system:

- Stops dial illumination and dial tone
- Ends or cancels any active Bluetooth call or call setup

Electrical Detection Principle
------------------------------

Off-hook is detected as a significant increase in loop current (typically 20–60mA), as the subscriber loop is closed by lifting the receiver. The selected detection circuit senses this current or voltage change and provides a safe, isolated logic signal to the ESP32.

Historical Framing
------------------

Telephone systems—whether classic analog or modern digital—have always required reliable methods to detect when a subscriber lifts the receiver (“goes off-hook”). Over time, the technology for sensing this condition has evolved to meet new demands for safety, durability, integration, and compatibility with advancing switching systems. Each stage in this progression reflects broader shifts in both available technology and network requirements.

- **1900s–1940s:** Mechanical relays sensed loop current and physically closed contacts to engage switching equipment.
- **1950s–1970s:** Solid-state transistor circuits began replacing relays in central office and PBX gear.
- **1970s–1990s:** Optocouplers introduced safe isolation between analog lines and digital switching logic.
- **2000s–present:** Solid-state relays (SSRs) and line interface ICs became common in VoIP and embedded systems.

Detection Approach Options
--------------------------

Several hardware approaches were considered for off-hook detection. Each method reflects a different era or technology, with distinct advantages and drawbacks:

- **Mechanical Relay:**  
  The classic approach—uses a relay coil in series with the phone line. When the handset is lifted, loop current flows, energizing the coil and closing relay contacts. This method provides satisfying audible/physical feedback (“click”), but relays are bulky, slow, and eventually wear out due to mechanical contacts.

- **Resistor + Transistor:**  
  An early solid-state solution—places a current-sensing resistor in the line. When enough current flows, it creates a voltage drop that turns on a transistor, which then signals the logic input. Simple and cheap, but offers no isolation and exposes sensitive electronics to high voltages.

- **Optocoupler:**  
  The modern, best-practice method. The loop current drives an LED inside the optocoupler, which activates an isolated phototransistor connected to the ESP32. Provides safe galvanic isolation, is compact and reliable, and integrates easily with microcontrollers.

- **Solid-State Relay (SSR):**  
  Uses an internal opto-isolated switch or MOSFET pair. SSRs offer “plug and play” operation, are fully silent, have no moving parts, and provide isolation. They are sometimes more expensive and may leak a small current even when “off.”

Behavior & Performance
----------------------

.. list-table::
   :header-rows: 1
   :widths: 22 19 19 19 19

   * - **Feature**
     - **Mechanical Relay**
     - **Resistor + Transistor**
     - **Optocoupler**
     - **SSR**
   * - **Sound / Feel**
     - ✅ Clunky click
     - ❌ Silent
     - ❌ Silent
     - ❌ Silent
   * - **Speed**
     - ❌ Slower
     - ✅ Fast
     - ✅ Fast
     - ✅ Fast
   * - **Wear**
     - ❌ Wears out
     - ✅ No wear
     - ✅ No wear
     - ✅ No wear
   * - **Reliability**
     - ❌ Contact wear
     - ✅ Stable
     - ✅ Very stable
     - ✅ Some leakage

Safety, Power & Integration
---------------------------

All methods connect to an ESP32 using basic GPIO—no special peripherals needed.

.. list-table::
   :header-rows: 1
   :widths: 22 19 19 19 19

   * - **Feature**
     - **Mechanical Relay**
     - **Resistor + Transistor**
     - **Optocoupler**
     - **SSR**
   * - **Voltage Isolation**
     - ✅ Isolated
     - ❌ Direct risk
     - ✅ Isolated
     - ✅ Isolated
   * - **Current Draw**
     - 20–60mA
     - 20–60mA
     - 20–60mA
     - Very low
   * - **Power Needs**
     - ❌ Extra power
     - ✅ None
     - ✅ None
     - ✅ None

Complexity, Cost & Final Score
------------------------------

.. list-table::
   :header-rows: 1
   :widths: 22 19 19 19 19

   * - **Feature**
     - **Mechanical Relay**
     - **Resistor + Transistor**
     - **Optocoupler**
     - **SSR**
   * - **Simplicity**
     - ❌ Needs driver
     - ❌ Needs protection
     - ✅ Simple
     - ✅ PnP
   * - **Complexity**
     - ❌ Moderate
     - ✅ Low
     - ✅ Low
     - ✅ Low
   * - **Cost**
     - ❌ Moderate
     - ✅ Cheapest
     - ✅ Affordable
     - ❌ Higher
   * - **Pitfalls**
     - Debounce needed
     - Needs Zener
     - Vf matching
     - Leakage
   * - **🔥 Final Score**
     - **34**
     - **27**
     - **45**
     - **42**

Best Choice Based on Score
--------------------------

.. list-table::
   :header-rows: 1
   :widths: 50 20

   * - **Detection Method**
     - **Final Score (Max 50)**
   * - ✅ **Optocoupler (Modern Isolation)** → *Best Overall*
     - **45**
   * - ✅ **SSR (No Click)** → *PnP Option*
     - **42**
   * - ✅ **Mechanical Relay (Clicky CO Feel)**
     - **34**
   * - ✅ **Resistor + Transistor (Vintage Style)**
     - **27**

.. note::
   In modern commercial telephony and VoIP/FXS hardware, optocouplers and line interface ICs are the preferred method for safety, integration, and cost. SSRs are also viable, but may introduce leakage and are less common for low-current analog loops.

Debouncing and Safety
---------------------

To ensure reliable operation, the system **needs to implement software debouncing** on the optocoupler signal. This prevents false triggers caused by voltage spikes or rapid toggling. Additionally, a supervisory timeout should be included to detect and respond appropriately if the phone remains off-hook for an extended period (e.g., playing a reorder tone or releasing a call after a timeout).

Signal Flow Overview
--------------------

The following diagram illustrates the off-hook detection signal path, from the high-voltage line, through the detection circuit, to the ESP32 microcontroller. This ensures both isolation and accurate sensing of the phone’s status.

.. image:: /_images/off_hook_signal_flow.png
   :alt: Off-hook detection signal flow from phone line to ESP32
   :width: 400px
   :align: center


Final Thoughts
--------------

In this project, the **optocoupler-based circuit** will be used for off-hook detection. It provides galvanic isolation from the phone line and allows the ESP32 to monitor the line safely.

.. tip::
   **Before connecting to real phone voltages, always validate your optocoupler circuit with a scaled-down (low-voltage) DC source first.** This ensures reliable triggering and confirms isolation, helping protect both you and your hardware.

The implementation will require:

  - A **current-limiting resistor** on the optocoupler's input side  
  - Careful matching of the **forward voltage (Vf)** and current for reliable triggering  
  - A **pull-up resistor** or logic input to the ESP32 on the output side  
  - Optional use of a **Schmitt trigger or GPIO interrupt** for clean digital transitions

This approach reflects how later-generation COs managed line isolation while providing a practical and safe solution for modern embedded design.
