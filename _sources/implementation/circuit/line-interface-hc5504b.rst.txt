Line Interface: HC-5504B SLIC
=============================

The HC-5504B Subscriber Line Interface Circuit (SLIC) is the critical component that connects the Ma Bell Gateway to a standard analog telephone.  
It implements nearly all essential BORSCHT functions (Battery feed, Overvoltage protection, Ringing, Supervision, Codec hybrid, and Test), emulating the classic Bell System line card in a single chip.

Overview
--------

- Provides DC loop feed (-48V) and line supervision for the phone.
- Handles 2-wire to 4-wire hybrid audio conversion.
- Drives ringing voltage to the subscriber line via relay control.
- Detects off-hook, ring trip, and ground key conditions.
- Integrates protection features for robust operation.

Pinout Summary (DIP-24)
-----------------------

.. list-table::
   :header-rows: 1
   :widths: 8 18 24

   * - Pin
     - Name
     - Description
   * - 1
     - TIP
     - Analog subscriber loop (tip)
   * - 2
     - RING
     - Analog subscriber loop (ring)
   * - 3
     - RFS
     - Ring Feed Sense (supervision, ringing)
   * - 4
     - VB+
     - Positive supply (+5V or +12V)
   * - 5
     - C3
     - Loop current/timing capacitor
   * - 6
     - DG
     - Digital ground
   * - 7
     - RS
     - Ring synchronization input (tie to +5V if unused)
   * - 8
     - RD
     - Ring relay driver output (logic low = active)
   * - 9
     - TF
     - Tip Feed (to line via 150Ω resistor)
   * - 10
     - RF
     - Ring Feed (to line via 150Ω resistor)
   * - 11
     - VB-
     - -48V battery supply
   * - 12
     - BG
     - Battery ground
   * - 13
     - SHD
     - Switch hook detect (off-hook, logic output)
   * - 14
     - GKD
     - Ground key detect (logic output)
   * - 15
     - PD
     - Power denial (logic input)
   * - 16
     - RC
     - Ring command (logic input)
   * - 17
     - C2
     - Capacitor for ring trip/ground key filter
   * - 18
     - OUT
     - Uncommitted op-amp output (optional)
   * - 19
     - -IN
     - Op-amp inverting input
   * - 20
     - +IN
     - Op-amp non-inverting input
   * - 21
     - RX
     - Receive input (audio from codec/DAC)
   * - 22
     - C4
     - Capacitor for longitudinal balance/filter
   * - 23
     - AG
     - Analog ground
   * - 24
     - TX
     - Transmit output (audio to codec/ADC)

Typical Application Circuit
---------------------------

#.. figure:: /images/hc5504b-app-circuit.png
#   :alt: Typical application circuit for HC-5504B SLIC

   Typical application circuit for the HC-5504B SLIC (see datasheet for details).

**Note:**  
- Requires external protection (bridge rectifier, MOV, fuses) for robust field operation.
- Uses precision resistors (typically 150Ω) and filter capacitors as per the datasheet.
- Relay or MOSFET driver may be needed for high-voltage ringing.

Interfacing with ESP32
----------------------

- **SHD (Switch Hook Detect, Pin 13):** Connects to ESP32 input GPIO for off-hook status.
- **GKD (Ground Key Detect, Pin 14):** Optional; connects to ESP32 input if ground key features are needed.
- **RD (Ring Relay Driver, Pin 8):** Can be monitored by ESP32 for relay status or used to trigger ringing logic.
- **RC (Ring Command, Pin 16):** Controlled by ESP32 output to trigger ringing sequence.
- **PD (Power Denial, Pin 15):** Controlled by ESP32 output if selective power shutdown is desired.
- **TX (Pin 24), RX (Pin 21):** Connect to audio codec, then to ESP32 I2S interface for digital audio in/out.

Design Notes
------------

- Carefully manage ground domains: AG, DG, and BG should be star-connected as per best practices.
- VB- (-48V) and VB+ (+5V/+12V) supplies must be isolated and protected.
- See datasheet for specific application examples and protection recommendations.

References
----------

- `HC-5504B Datasheet (Intersil/Renesas) <https://www.renesas.com/us/en/document/dst/hc-5504b-datasheet>`_

.. note::

   Update this page with your final schematic snippet, board pin numbers, and any specific integration notes as you build and test.

