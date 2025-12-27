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

Application Circuit
-------------------

The following shows the complete application circuit for the HC-5504B SLIC, including all external components, power connections, and interface to the ESP32.

Basic SLIC Connections
^^^^^^^^^^^^^^^^^^^^^^^

::

    Telephone Line (TIP/RING)
            |
            v
    ┌───[ Protection Circuit ]───┐
    │   (See Protection section) │
    └──────────┬─────────────────┘
               │
               ├── TIP ──────────────────────┐
               │                              │
               └── RING ────────────────────┐ │
                                            │ │
                      ┌─────────────────────┼─┼───────────┐
                      │   HC-5504B SLIC     │ │           │
                      │                     │ │           │
    Power Supplies:   │  Pin 1: TIP ────────┘ │           │
                      │  Pin 2: RING ──────────┘           │
    +12V ─────────┬───┤  Pin 4: VB+ (+12V)                 │
                  │   │  Pin 11: VB- (-48V) ───────── -48V │
    Decoupling:   │   │  Pin 12: BG (Battery GND)          │
    100nF + 10µF  │   │  Pin 6: DG (Digital GND)           │
                  │   │  Pin 23: AG (Analog GND) ──┬───────┤
                  │   │                             │       │
    External      │   │  Pin 9: TF ──┬── 150Ω ─────┤       │
    Components:   │   │  Pin 10: RF ─┴── 150Ω ─────┤       │
                  │   │  Pin 5: C3 ──── 4.7µF ─────┤       │
                  │   │  Pin 17: C2 ─── 2.2µF ─────┤       │
                  │   │  Pin 22: C4 ─── 0.1µF ─────┤       │
                  │   │                             │       │
    Signals:      │   │  Pin 13: SHD ───────────── ESP32   │
                  │   │  Pin 8: RD ────────────── GPIO 32  │
                  │   │  Pin 16: RC ───────────── GPIO 33  │
                  │   │                         GPIO 13    │
    Audio:        │   │  Pin 24: TX ───────┐               │
                  │   │  Pin 21: RX ───────┤── Codec       │
                  │   │                     │   I2S         │
                  │   └─────────────────────┴───────────────┘
                  │
                  └─ Star Ground Point (AG=DG=BG)


External Component Values
^^^^^^^^^^^^^^^^^^^^^^^^^^

Based on HC-5504B datasheet recommendations:

- **R_TIP, R_RING** (Pins 9, 10): 150Ω, 1% tolerance, 1/4W

  - Feed resistors for tip and ring
  - Critical for proper loop current limiting

- **C3** (Pin 5): 4.7µF electrolytic or ceramic

  - Loop current timing capacitor
  - Voltage rating: min 16V

- **C2** (Pin 17): 2.2µF

  - Ring trip and ground key filter
  - Voltage rating: min 16V

- **C4** (Pin 22): 0.1µF ceramic

  - Longitudinal balance capacitor
  - Improves common-mode noise rejection

- **Decoupling on VB+**: 100nF ceramic (close to IC) + 10µF electrolytic

  - Place ceramic capacitor within 0.5" of pin 4
  - Electrolytic for bulk supply stabilization

**Note:**
The SLIC requires external protection (bridge rectifier, MOV, fuses) for robust field operation. See the "Line Protection" section for complete protection circuit design.

ESP32 GPIO Interface
--------------------

The HC-5504B SLIC interfaces with the ESP32 microcontroller through several GPIO pins for control and status monitoring. Audio signals flow through an external codec connected via I2S.

GPIO Pin Mapping
^^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 8 20 12 12 38

   * - SLIC Pin
     - SLIC Function
     - ESP32 GPIO
     - Direction
     - Description
   * - 13
     - SHD (Switch Hook Detect)
     - GPIO 32
     - Input
     - Off-hook detection - Goes LOW when phone is off-hook
   * - 8
     - RD (Ring Relay Driver)
     - GPIO 33
     - Input
     - Ring relay status monitoring - Active LOW when relay engaged
   * - 16
     - RC (Ring Command)
     - **GPIO 13**
     - **Output**
     - **Trigger ring sequence - Drive LOW to activate ringing**
   * - 15
     - PD (Power Denial)
     - Not Connected
     - (Output)
     - Optional power control - Not used in this design
   * - 14
     - GKD (Ground Key Detect)
     - Not Connected
     - (Input)
     - Optional ground key detection - Not used in this design
   * - 24
     - TX (Transmit Audio)
     - Via Audio Codec
     - Input to SLIC
     - Audio from ESP32 → Codec → SLIC → Phone
   * - 21
     - RX (Receive Audio)
     - Via Audio Codec
     - Output from SLIC
     - Audio from Phone → SLIC → Codec → ESP32

Signal Level Notes
^^^^^^^^^^^^^^^^^^

- **Logic Levels:** All SLIC digital outputs (SHD, RD, GKD) operate at 3.3V logic levels, directly compatible with ESP32 GPIOs
- **Input Protection:** No level shifting required - SLIC pins 13, 14, 8 can directly drive ESP32 inputs
- **Output Drive:** ESP32 GPIO 13 can directly drive SLIC pin 16 (RC) for ring control
- **Pull-ups:** Internal ESP32 pull-ups may be enabled on input pins (SHD, RD) for noise immunity

Key Interface Functions
^^^^^^^^^^^^^^^^^^^^^^^

**Off-Hook Detection (SHD → GPIO 32):**
The SHD pin goes LOW when the telephone handset is lifted (off-hook condition). The ESP32 monitors this pin to detect when a call is initiated or answered.

**Ring Control (GPIO 13 → RC):**
To ring the telephone, the ESP32 drives GPIO 13 LOW. This signals the SLIC to engage the ring relay and apply ringing voltage to the line. A typical ring pattern is 2 seconds ON, 4 seconds OFF.

**Ring Status Monitor (RD → GPIO 33):**
The RD pin reflects the state of the internal ring relay driver. When active (LOW), the ring relay is engaged. This allows the ESP32 to confirm ringing is occurring.

**Audio Path:**
Audio does not connect directly to ESP32. Instead, the SLIC's TX and RX pins connect to an external audio codec, which performs analog-to-digital and digital-to-analog conversion. The codec then interfaces with ESP32 via I2S (see Audio Signal Path section).

Audio Signal Path
-----------------

The SLIC's analog audio outputs (TX and RX pins) require an external audio codec to interface with the ESP32's digital I2S bus. The codec performs ADC (analog-to-digital) and DAC (digital-to-analog) conversion.

Signal Flow Diagram
^^^^^^^^^^^^^^^^^^^

::

    ┌──────────┐        ┌───────────┐       ┌─────────────┐       ┌───────────┐
    │          │ Analog │           │ I2S   │             │  BT   │           │
    │  Phone   ├────────┤   Audio   ├───────┤    ESP32    ├───────┤ Bluetooth │
    │  (SLIC)  │        │   Codec   │       │             │       │  Device   │
    │          ├────────┤           │       │             │       │           │
    └──────────┘        └───────────┘       └─────────────┘       └───────────┘
     TX    RX            DAC   ADC          I2S Pins:
     Pin24 Pin21                            - GPIO 25 (LRCLK/WS)
                                            - GPIO 5  (BCLK)
                                            - GPIO 26 (DOUT to codec)
                                            - GPIO 35 (DIN from codec)

**Transmit Path (ESP32 → Phone):**
ESP32 → I2S → Codec DAC → SLIC RX (Pin 21) → Telephone handset speaker

**Receive Path (Phone → ESP32):**
Telephone handset microphone → SLIC TX (Pin 24) → Codec ADC → I2S → ESP32

Selected Audio Codec Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Ma Bell Gateway uses the **PCM5100 (DAC) + PCM1808 (ADC)** from Texas Instruments for production-quality audio in Bluetooth telephony applications.

Why This Configuration
"""""""""""""""""""""""

- **Excellent Audio Quality:** 16-bit+ audio with high SNR (106 dB DAC, 98 dB ADC)
- **No I2C Configuration:** Hardware-configured via pin strapping (simpler firmware)
- **Flexible Development:** Can implement DAC first (playback), then add ADC (recording)
- **Reliable:** Hardware configuration eliminates software bugs related to codec initialization
- **Production-Ready:** Professional telephony audio quality

PCM5100 (DAC) - Digital to Analog Converter
""""""""""""""""""""""""""""""""""""""""""""

**Key Specifications:**

- 32-bit, 384 kHz stereo audio DAC
- SNR: 106 dB (A-weighted)
- THD+N: -93 dB
- Hardware-configured (no I2C/SPI needed)
- Integrated PLL for clock flexibility
- 2.5V - 3.6V supply voltage

**Pin Connections:**

.. list-table::
   :header-rows: 1
   :widths: 20 20 50

   * - PCM5100 Pin
     - ESP32 Connection
     - Notes
   * - LRCK (WS)
     - GPIO 25
     - I2S word select / frame sync
   * - BCK (SCK)
     - GPIO 5
     - I2S bit clock
   * - DIN
     - GPIO 26
     - I2S data from ESP32
   * - XSMT
     - 3.3V (via 10kΩ)
     - System mute (HIGH = unmuted)
   * - FMT
     - GND
     - I2S format selection (see datasheet)
   * - VDD
     - 3.3V
     - Power supply (+decoupling)
   * - GND
     - GND
     - Ground
   * - OUTL/OUTR
     - AC coupling to SLIC RX
     - Analog output (use OUTL for mono)

PCM1808 (ADC) - Analog to Digital Converter
""""""""""""""""""""""""""""""""""""""""""""

**Key Specifications:**

- 24-bit, 96 kHz stereo audio ADC
- SNR: 98 dB (A-weighted)
- THD+N: -85 dB
- Hardware-configured via pin strapping
- Single-ended or differential input
- 2.7V - 5.5V supply voltage

**Pin Connections:**

.. list-table::
   :header-rows: 1
   :widths: 20 20 50

   * - PCM1808 Pin
     - ESP32 Connection
     - Notes
   * - LRCK
     - GPIO 25
     - I2S word select / frame sync (shared with DAC)
   * - BCK
     - GPIO 5
     - I2S bit clock (shared with DAC)
   * - DOUT
     - GPIO 35
     - I2S data to ESP32
   * - FMT0
     - GND
     - I2S format selection (00 = I2S, 24-bit)
   * - FMT1
     - GND
     - I2S format selection
   * - MD0
     - 3.3V
     - Mode selection: slave mode
   * - MD1
     - GND
     - Mode selection
   * - VDD
     - 3.3V
     - Power supply (+decoupling)
   * - GND
     - GND
     - Ground
   * - VINL
     - AC coupling from SLIC TX
     - Analog input (use VINL for mono)

Signal Path Diagram
"""""""""""""""""""

::

    Transmit Path (Phone → ESP32 → Bluetooth):
    ┌───────────┐                           ┌────────────┐              ┌────────────┐
    │  Handset  │     TX Pin 24             │  PCM1808   │     I2S      │   ESP32    │
    │  (Mic)    ├───────────────┬─[10µF]───┤ VINL  DOUT ├──────────────┤ GPIO 35    │
    └───────────┘               │           └────────────┘              └────────────┘
                    SLIC        └─[10kΩ]─┬─GND
                                          └─AGND (Pin 23)

    Receive Path (Bluetooth → ESP32 → Phone):
    ┌───────────┐              ┌────────────┐                           ┌────────────┐
    │   ESP32   │     I2S      │  PCM5100   │     RX Pin 21             │  Handset   │
    │ GPIO 26   ├──────────────┤ DIN  OUTL  ├───[10µF]──────────────────┤ (Speaker)  │
    └───────────┘              └────────────┘                           └────────────┘
                                                         SLIC

    Common I2S Clock Signals (Shared):
    ESP32 GPIO 25 ────┬───► PCM5100 LRCK
                      └───► PCM1808 LRCK

    ESP32 GPIO 5  ────┬───► PCM5100 BCK
                      └───► PCM1808 BCK

Bill of Materials
"""""""""""""""""

.. list-table::
   :header-rows: 1
   :widths: 30 15 15 30

   * - Component
     - Part Number
     - Qty
     - Notes / Supplier
   * - DAC
     - PCM5100PWR
     - 1
     - Texas Instruments, TSSOP-20
   * - ADC
     - PCM1808PWR
     - 1
     - Texas Instruments, TSSOP-20
   * - AC Coupling Caps
     - 10µF film/MLCC
     - 2
     - Non-polarized, 16V+, X7R/C0G
   * - Decoupling Caps (DAC)
     - 0.1µF + 10µF
     - 2
     - Ceramic + tantalum/MLCC
   * - Decoupling Caps (ADC)
     - 0.1µF + 10µF
     - 2
     - Ceramic + tantalum/MLCC
   * - Pull-up Resistor (XSMT)
     - 10kΩ
     - 1
     - 1% metal film
   * - Input Bias Resistor
     - 10kΩ
     - 1
     - For PCM1808 VINL to AGND

PCB Layout Recommendations
"""""""""""""""""""""""""""

- **Placement:** Keep codec ICs within 3-4 inches of ESP32 for clean I2S signals
- **Signal Integrity:**

  - Route I2S clock signals (LRCK, BCK) as matched-length pairs
  - Keep I2S data lines (DIN, DOUT) short and direct
  - Avoid routing I2S signals near high-frequency switching (e.g., power converters)

- **Power:**

  - Place 0.1µF decoupling caps within 5mm of each IC's VDD pin
  - Add 10µF bulk caps near codec power pins
  - Use low-ESR ceramic capacitors (X7R or better)

- **Ground:**

  - Connect codec grounds to AGND plane (SLIC Pin 23)
  - Avoid ground loops - use star grounding topology
  - Separate analog and digital ground planes if possible

- **Audio Coupling:**

  - Place AC coupling capacitors close to SLIC pins
  - Use film or high-quality MLCC capacitors for audio paths
  - Keep analog traces short to minimize noise pickup

Prototyping Option: ESP32 Internal DAC
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::
   **For Prototyping/Testing Only**

   The ESP32 has two 8-bit internal DACs (GPIO25, GPIO26) that can be used for transmit-only testing during early development:

   - **Use Case:** Quick audio output testing without external hardware
   - **Limitations:**

     - 8-bit resolution (vs. 16-bit+ for external codecs) = significantly lower quality
     - Transmit-only - no ADC capability for receiving audio from phone
     - Cannot support full-duplex Bluetooth HFP calls
     - Higher noise floor and distortion

   - **Connection:** ESP32 DAC pin → 10µF AC coupling capacitor → SLIC RX (Pin 21)
   - **Not suitable for production** - Use PCM5100 + PCM1808 for production telephony applications

Alternative Codec Options (Reference)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For reference, alternative codec configurations are possible but not recommended for the Ma Bell Gateway:

**WM8731 (Wolfson/Cirrus Logic) - Integrated Codec**

- Single-chip ADC+DAC solution, well-documented
- Requires I2C configuration (additional firmware complexity)
- Cost: ~$3-5
- *When to use:* If PCB space is extremely limited and single-chip solution is required

**UDA1334A (NXP) - DAC Only**

- Transmit-only (no ADC)
- Cost: ~$2
- *When to use:* If only ESP32→Phone audio is needed (no bidirectional calls)

Audio Coupling
^^^^^^^^^^^^^^

Between the codec outputs and SLIC RX/TX pins, AC coupling capacitors are recommended:

- **Value:** 10µF non-polarized (or 10µF electrolytic in series pair for bipolar)
- **Purpose:** Block any DC offset from codec outputs
- **Placement:** As close to SLIC pins as possible

::

    Codec Output ──┤├── 10µF ──┤├── 10µF ─── SLIC RX/TX Pin
                    (AC coupling, bipolar configuration)

Or use non-polarized film capacitors if available.

I2S Pin Connections (PCM5100 + PCM1808)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

ESP32 I2S interface connections to PCM5100 (DAC) and PCM1808 (ADC), as defined in pin_assignments.h:

**Shared Clock Signals:**

- **GPIO 25** - PCM_FSYNC (I2S Word Select / LRCLK / Frame Sync)

  - Connected to: PCM5100 LRCK + PCM1808 LRCK
  - Common frame sync for both DAC and ADC

- **GPIO 5** - PCM_CLK_OUT (I2S Bit Clock / BCLK)

  - Connected to: PCM5100 BCK + PCM1808 BCK
  - Common bit clock for both DAC and ADC

**Data Signals:**

- **GPIO 26** - PCM_DOUT (I2S Data Out from ESP32)

  - Connected to: PCM5100 DIN
  - Audio data to DAC (for playback/receive path)

- **GPIO 35** - PCM_DIN (I2S Data In to ESP32)

  - Connected to: PCM1808 DOUT
  - Audio data from ADC (for recording/transmit path)

**Connection Summary:**

::

    ESP32                       PCM5100 (DAC)         PCM1808 (ADC)
    ──────                      ─────────────         ─────────────
    GPIO 25 (LRCLK)  ──────┬──► LRCK          ┌────► LRCK
    GPIO 5  (BCLK)   ──────┼──► BCK           ├────► BCK
    GPIO 26 (DOUT)   ───────┘─► DIN           │
    GPIO 35 (DIN)    ◄─────────────────────────────── DOUT

All I2S signals should be routed as short, direct traces with matched lengths where practical.

Sample Rate and Format
^^^^^^^^^^^^^^^^^^^^^^

Typical audio configuration for telephone-quality audio:

- **Sample Rate:** 8 kHz or 16 kHz (8 kHz is standard for telephony)
- **Bit Depth:** 16-bit
- **Format:** I2S standard format
- **Channels:** Mono (though I2S is stereo-capable, phone audio is mono)

Power Supply Integration
-------------------------

The HC-5504B SLIC requires multiple voltage rails and careful grounding to function properly. All power rails are derived from the main 48V supply as documented in the Power Supply Architecture (see power-supply.rst).

Voltage Rail Requirements
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 15 15 20 40

   * - SLIC Pin
     - Rail
     - Voltage
     - Source/Notes
   * - Pin 4 (VB+)
     - Positive Supply
     - +12V DC
     - From 48V→12V buck converter. Powers SLIC analog circuits
   * - Pin 11 (VB-)
     - Negative Battery
     - -48V DC
     - From inverting DC-DC converter. Loop feed voltage for telephone line
   * - Pin 12 (BG)
     - Battery Ground
     - 0V (GND)
     - Battery/power supply ground reference
   * - Pin 6 (DG)
     - Digital Ground
     - 0V (GND)
     - Digital logic ground
   * - Pin 23 (AG)
     - Analog Ground
     - 0V (GND)
     - Analog/audio ground

Power Rail Connections
^^^^^^^^^^^^^^^^^^^^^^

::

    From Power Supply System:

    48V Main Supply
         │
         ├─► Buck Converter ──► +12V ──┬──► VB+ (Pin 4)
         │                              │
         │                     Decoupling: 100nF + 10µF
         │                              │
         └─► Inverting DC-DC ─► -48V ──┴──► VB- (Pin 11)


                                  Star Ground Point ◄─┬─ AG (Pin 23)
                                       (GND)          ├─ DG (Pin 6)
                                                      └─ BG (Pin 12)

Grounding Strategy
^^^^^^^^^^^^^^^^^^

The SLIC has three separate ground pins that must be connected using a **star grounding** topology:

1. **AG (Analog Ground, Pin 23)** - For audio and analog signals
2. **DG (Digital Ground, Pin 6)** - For digital logic signals
3. **BG (Battery Ground, Pin 12)** - For power supply return

**Star Ground Implementation:**

All three ground pins should connect to a single point as close to the SLIC as possible. This minimizes ground loops and prevents noise coupling between analog, digital, and power domains.

::

    SLIC Pins:                           Common Star
                                         Ground Point
         AG (23) ─────────────────────────┐
                                          ├───► System GND
         DG (6)  ─────────────────────────┤
                                          │
         BG (12) ─────────────────────────┘

On PCB: Use a copper pour or wide trace for the star ground point, with short, direct connections from each SLIC ground pin.

Power Supply Decoupling
^^^^^^^^^^^^^^^^^^^^^^^^

Critical for stable SLIC operation:

**On VB+ (+12V, Pin 4):**

- **100nF ceramic capacitor** - Place within 0.5 inches (12mm) of pin 4
- **10µF electrolytic or ceramic** - Place within 1 inch (25mm) of pin 4
- Purpose: Filter switching noise from buck converter, stabilize analog circuits

**On VB- (-48V, Pin 11):**

- **10µF electrolytic, 63V rated** - Place near pin 11
- Purpose: Stabilize negative supply, handle transients during ringing

Example PCB Layout
^^^^^^^^^^^^^^^^^^

::

    ┌──────────────────────────────────────┐
    │         HC-5504B SLIC Package        │
    │                                      │
    │   Pin 4 (VB+) ─┬─ 100nF (ceramic)   │
    │     +12V       └─ 10µF (electro/cer) │
    │                                      │
    │   Pin 11 (VB-) ─── 10µF, 63V (electro)
    │      -48V                            │
    │                                      │
    │   Pin 23 (AG) ──┐                   │
    │   Pin 6  (DG) ──┤                   │
    │   Pin 12 (BG) ──┴─► ★ Star GND      │
    │                                      │
    └──────────────────────────────────────┘

Cross-Reference
^^^^^^^^^^^^^^^

For complete power supply system design including:

- 48V main supply specifications
- Buck converter selection (+12V rail)
- Inverting DC-DC design (-48V rail)
- Ring generator (90V AC, 20Hz)

See: **docs/source/implementation/circuit/power-supply.rst**

Line Protection
---------------

The telephone line interface must include protection circuitry to safeguard the SLIC and connected electronics from overvoltage, reverse polarity, and fault conditions. The HC-5504B provides some internal protection, but external components are essential for robust field operation.

Protection Circuit Overview
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    Telephone                                          To SLIC
    Line Jack                                       TIP/RING Pins
         │                                                │
         ├─ TIP ──┬─[ Fuse 0.5A ]──┬─[ Bridge Rect ]──┬──► TIP (Pin 1)
         │        │                 │    DB107 or      │
         │        │                 │    equiv.        │
         └─ RING ─┴─[ Fuse 0.5A ]──┴──────┬───────────┴──► RING (Pin 2)
                  │                        │
                  │                      [ MOV ]
                  │                    V150LA10A
                  │                    (150V rating)
                  │                        │
                  └────────────────────────┴──► GND


    Protection Elements:
    1. Fuses - Overcurrent protection (0.5A slow-blow)
    2. Bridge Rectifier - Reverse polarity protection
    3. MOV - Overvoltage/surge protection

Protection Components
^^^^^^^^^^^^^^^^^^^^^

**1. Overcurrent Protection - Fuses**

- **Type:** 0.5A slow-blow (time-delay) fuse
- **Purpose:** Protect against sustained overcurrent or short circuit
- **Placement:** One fuse in series with TIP, one with RING
- **Rating:** 250V AC minimum
- **Part Example:** Littelfuse 0217.500 or equivalent

**Why slow-blow?** Telephone lines experience brief current surges during ringing. A fast-blow fuse would trip unnecessarily. Slow-blow fuses tolerate brief surges while still protecting against sustained faults.

**2. Reverse Polarity Protection - Bridge Rectifier**

- **Type:** Full-wave bridge rectifier
- **Purpose:** Protect SLIC from reversed TIP/RING connections
- **Rating:** 1A, 200V minimum
- **Part Example:** DB107 (1A, 1000V) or Vishay DF10S (1A, 1000V)

The bridge rectifier ensures the SLIC always sees correct polarity regardless of how the telephone line is connected. This is critical because telephone line polarity can vary in field installations.

::

    Bridge Rectifier Operation:

    TIP/RING ──┬─►|├──┬──► To SLIC TIP (always positive)
               │  BR  │
               └─►|├──┴──► To SLIC RING (always negative)

**3. Overvoltage Protection - Metal Oxide Varistor (MOV)**

- **Type:** MOV (Metal Oxide Varistor)
- **Purpose:** Clamp transient overvoltages (lightning, power line cross, inductive kicks)
- **Rating:** 150V RMS / 200V DC
- **Part Example:** Littelfuse V150LA10A or Bourns MOV-14D151K
- **Placement:** Across TIP and RING, after fuses

The MOV acts as a voltage-dependent resistor. Under normal voltage (<150V), it has very high resistance. During a surge (>150V), it clamps to ground, shunting excess energy away from the SLIC.

**Optional: Gas Discharge Tube (GDT)**

For enhanced lightning protection in areas with high lightning activity:

- **Type:** Gas discharge tube (GDT)
- **Rating:** 90V - 230V breakdown
- **Placement:** Parallel to MOV, or as first stage before MOV
- **Purpose:** Handle very high energy transients (direct lightning strikes)

GDTs can handle much higher energy than MOVs but have slower response time. Using both (GDT as first stage, MOV as second stage) provides optimal protection.

Complete Protection Schematic
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    Phone Line Jack
         │
    TIP ─┼─[ F1: 0.5A ]──┬────────┬───────┐
         │               │        │       │
         │             [ BR1 ]  [ MOV ] [GDT]   ← Protection
         │               │      150V   90-230V    Components
         │               │        │       │
    RING─┴─[ F2: 0.5A ]──┴────────┴───────┘
                                  │
                                 GND
                                  │
                     ┌────────────┴────────────┐
                     │                         │
              Rectified & Protected     To HC-5504B
              TIP/RING Output           Pins 1 & 2
                     │                         │
                     └─────────────────────────┘

Placement and Routing
^^^^^^^^^^^^^^^^^^^^^

- **Fuses:** Place immediately after phone line connector
- **Bridge Rectifier:** Place after fuses, before SLIC
- **MOV:** Place between TIP/RING and ground, as close to fuses as practical
- **GDT (if used):** Place in parallel with MOV, or as first protection stage

**PCB Considerations:**

- Use wide traces (>20 mil) for TIP/RING to handle surge currents
- Keep protection components physically close to phone jack
- Route protected signals to SLIC with minimal trace length
- Maintain adequate creepage/clearance for high voltage (>2mm for 150V)

Component Selection Notes
^^^^^^^^^^^^^^^^^^^^^^^^^^

- **Fuse holders:** Use panel-mount or PCB-mount holders for easy replacement
- **Bridge:** Higher voltage rating (e.g., 1000V) provides extra margin
- **MOV clamping voltage:** Choose 130-150V range for -48V telephone lines
- **All components:** Ensure automotive/telecom grade for reliability

Safety Notes
^^^^^^^^^^^^

- This protection is for normal telephone line faults and surges
- For direct lightning strikes, additional building-level protection (whole-house surge suppressors) is recommended
- Always test protection circuits with controlled overvoltage sources before field deployment
- Replace MOVs after major surge events (they degrade with use)

Component Selection & Bill of Materials
----------------------------------------

This section provides a complete BOM for the SLIC circuit, including all passive components, protection devices, and recommended part numbers.

SLIC External Components
^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 20 15 20 35

   * - Component
     - Value/Type
     - Part Number/Example
     - Notes
   * - R_TIP (Pin 9 feed)
     - 150Ω, 1%, 1/4W
     - Generic metal film
     - Tip feed resistor, precision critical
   * - R_RING (Pin 10 feed)
     - 150Ω, 1%, 1/4W
     - Generic metal film
     - Ring feed resistor, precision critical
   * - C3 (Pin 5)
     - 4.7µF, 16V+
     - Generic electrolytic/ceramic
     - Loop current timing capacitor
   * - C2 (Pin 17)
     - 2.2µF, 16V+
     - Generic electrolytic/ceramic
     - Ring trip/ground key filter
   * - C4 (Pin 22)
     - 0.1µF (100nF)
     - Generic ceramic
     - Longitudinal balance
   * - C_VB+ (Pin 4 decoupling)
     - 100nF ceramic
     - Generic X7R/X5R
     - Place <0.5" from pin 4
   * - C_VB+ (Pin 4 bulk)
     - 10µF, 16V+
     - Generic electrolytic/ceramic
     - Place <1" from pin 4
   * - C_VB- (Pin 11)
     - 10µF, 63V
     - Generic electrolytic
     - -48V supply decoupling

Protection Circuit Components
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 20 15 20 35

   * - Component
     - Value/Type
     - Part Number/Example
     - Notes
   * - F1, F2 (Fuses)
     - 0.5A slow-blow, 250V
     - Littelfuse 0217.500
     - Overcurrent protection on TIP/RING
   * - BR1 (Bridge)
     - 1A, 200V+ bridge
     - DB107 (1A, 1000V)
     - Reverse polarity protection
   * - MOV1
     - 150V RMS MOV
     - Littelfuse V150LA10A
     - Overvoltage/surge protection
   * - GDT1 (Optional)
     - 90-230V GDT
     - Bourns 2036-15-SM
     - Enhanced lightning protection

Audio Coupling Components
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 20 15 20 35

   * - Component
     - Value/Type
     - Part Number/Example
     - Notes
   * - C_TX, C_RX
     - 10µF non-polar or film
     - Generic film or dual electrolytic
     - AC couple codec to SLIC TX/RX

Ring Relay (if external ringing used)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 20 15 20 35

   * - Component
     - Value/Type
     - Part Number/Example
     - Notes
   * - RLY1
     - DPDT, 12V coil
     - Omron G5V-2 or equiv
     - Switches 90V AC ring voltage
   * - D1 (Flyback diode)
     - 1N4148 or 1N4001
     - Generic signal/power diode
     - Protect relay driver from inductive kick

Main IC
^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 20 15 20 35

   * - Component
     - Value/Type
     - Part Number/Example
     - Notes
   * - U1 (SLIC)
     - HC-5504B-5 DIP-24
     - HC-5504B-5 (Renesas/Intersil)
     - Main SLIC IC

Audio Codec (Choose One Option)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Option 1: Integrated Codec**

.. list-table::
   :widths: 20 35 35

   * - WM8731 Codec
     - Wolfson/Cirrus WM8731SEDS/V
     - Stereo codec, I2S + I2C interface

**Option 2: Separate DAC/ADC**

.. list-table::
   :widths: 20 35 35

   * - PCM5100 DAC
     - Texas Instruments PCM5100PW
     - DAC for ESP32 → Phone path
   * - PCM1808 ADC
     - Texas Instruments PCM1808PWR
     - ADC for Phone → ESP32 path

**Option 3: Simple DAC (TX only)**

.. list-table::
   :widths: 20 35 35

   * - UDA1334A DAC
     - NXP UDA1334ATS
     - Simple I2S DAC, TX path only

Connector & Miscellaneous
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 20 35 35

   * - Component
     - Description
     - Notes
   * - J1
     - RJ11/RJ45 phone jack
     - Standard 6P4C or 8P8C modular jack for telephone line
   * - Fuse holders
     - Panel/PCB mount holders
     - For F1, F2 (facilitates replacement)

Sourcing Notes
^^^^^^^^^^^^^^

- Most passive components (resistors, capacitors) are generic and available from Digi-Key, Mouser, LCSC, or similar
- SLIC IC (HC-5504B-5) may have limited availability; check Renesas, Digi-Key, or surplus distributors
- Audio codecs (WM8731, PCM5100, PCM1808) are widely available
- Use automotive/industrial-grade components where possible for reliability

Total Estimated Cost
^^^^^^^^^^^^^^^^^^^^

Approximate component costs (USD, quantity 1, retail pricing):

- SLIC IC (HC-5504B-5): $8-15 (if available)
- Passives (resistors, caps): $2-5
- Protection (fuses, bridge, MOV): $3-5
- Audio codec: $3-6
- Ring relay (if used): $3-5
- Misc (connectors, PCB): $5-10

**Total:** ~$25-45 USD for SLIC circuit components (excluding ESP32 and power supply)

PCB Design Guidelines
---------------------

Proper PCB layout is critical for the SLIC circuit to function reliably and meet noise/EMI requirements. Follow these guidelines during board design.

General Layout Strategy
^^^^^^^^^^^^^^^^^^^^^^^

1. **Functional Zones:**

   - Protection zone (fuses, bridge, MOV) near phone jack
   - SLIC zone (IC and immediate passives) in center
   - Audio zone (codec and coupling) near SLIC
   - Power supply zone (buck converters, decoupling) separate from audio

2. **Signal Flow:**

   ::

       Phone Jack ──► Protection ──► SLIC ──► Audio Codec ──► ESP32
                                      │
                                   Power Supply

   Arrange components to follow this left-to-right signal flow on PCB.

Grounding
^^^^^^^^^

**Star Ground Topology:**

The three SLIC ground pins (AG, DG, BG) must connect at a single star ground point as close to the SLIC as possible.

::

    ┌─────────────────────────────────┐
    │        SLIC IC (Top View)        │
    │                                  │
    │   Pin 23 (AG) ──┐               │
    │   Pin 6  (DG) ──┼───► ★ Star    │
    │   Pin 12 (BG) ──┘      GND Pad  │
    │                        (Copper)  │
    └─────────────────────────────────┘
             │
             └──► System Ground Plane

- Use wide traces (>50 mil) or copper pour for star ground connection
- Avoid running return currents from other circuits through SLIC ground
- Connect star ground to main PCB ground plane at single point

Power Supply Decoupling
^^^^^^^^^^^^^^^^^^^^^^^^

- **Ceramic capacitors (100nF):** Place within 0.5" (12mm) of VB+ pin
- **Bulk capacitors (10µF):** Place within 1" (25mm) of VB+ and VB- pins
- **Via placement:** Use multiple vias (2-4) for power/ground connections to reduce inductance

High Voltage Clearances
^^^^^^^^^^^^^^^^^^^^^^^^

For traces carrying -48V, +12V, or ring voltage (90V AC):

- **Trace spacing:** Minimum 2mm (80 mil) between high voltage and low voltage traces
- **Creepage:** Maintain 2.5mm creepage distance for 150V rating
- **Clearance:** Air gap ≥2mm for 150V
- **Solder mask:** Use solder mask as additional barrier but don't rely on it alone for isolation

Audio Path Layout
^^^^^^^^^^^^^^^^^

Audio traces (SLIC TX/RX to codec) should be:

- **Short and direct:** Minimize trace length (<3 inches ideal)
- **Differential routing:** Route TX+ and TX- as differential pair (if applicable)
- **Guard traces:** Surround audio traces with ground guard traces
- **Ground plane:** Provide solid ground plane under audio signals
- **Separation:** Keep audio traces away from digital signals (I2S clock, data) and switching power supplies

Digital Signal Integrity
^^^^^^^^^^^^^^^^^^^^^^^^^

For ESP32 GPIO and I2S signals:

- **Trace impedance:** Target 50Ω for high-speed signals (I2S BCLK, LRCLK)
- **Length matching:** Match I2S data and clock trace lengths within 0.5"
- **Termination:** Series 22-33Ω resistors on I2S signals near ESP32 if trace length >3"
- **Crosstalk:** Separate parallel traces by 3x trace width minimum

Protection Circuit Placement
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Place protection components (fuses, bridge, MOV) as close to the phone jack as possible:

::

    Phone Jack ──[ <0.5" ]──► Fuses ──[ <1" ]──► Bridge/MOV ──► SLIC

- Minimizes unprotected trace length
- Reduces chances of surge coupling to other circuits
- Use wide traces (>20 mil / 0.5mm) for TIP/RING to handle fault currents

Layer Stackup (for multi-layer boards)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Recommended 4-layer stackup:

::

    Layer 1: Signal + Components (top)
    Layer 2: Ground Plane
    Layer 3: Power Plane (+12V, +3.3V, -48V zones)
    Layer 4: Signal + Components (bottom)

For 2-layer boards:

- Top: Signals and components
- Bottom: Ground pour with power routing

Thermal Considerations
^^^^^^^^^^^^^^^^^^^^^^

- SLIC IC can dissipate 1-2W during ringing
- Provide thermal relief with copper pour connected to ground pins
- If using DIP package, consider adding heat sink or thermal pad

Component Placement Summary
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    ┌──────────────────────────────────────────────────┐
    │  Phone Jack (J1)                                  │
    │       │                                           │
    │   [Protection Zone]                               │
    │    F1, F2, BR1, MOV                               │
    │       │                                           │
    │    ┌──────────────────┐                          │
    │    │   SLIC IC (U1)   │   [Audio Zone]          │
    │    │   HC-5504B-5     │    Codec, coupling       │
    │    │                  ├──────────┐               │
    │    │  Passives around │          │               │
    │    │  (C2, C3, C4,    │      Codec IC            │
    │    │   R_TIP, R_RING) │          │               │
    │    └──────────────────┘          │               │
    │       │                           │               │
    │    [Power Zone]              I2S signals          │
    │    Buck converters           to ESP32 ───────────►│
    │    Decoupling caps                                │
    │                                                   │
    └───────────────────────────────────────────────────┘

Testing & Validation
^^^^^^^^^^^^^^^^^^^^

After PCB assembly:

1. Visual inspection for solder bridges, polarity
2. Continuity check: Star ground, power rails
3. Power-on test: Measure +12V, -48V at SLIC pins
4. Off-hook test: Connect phone, lift handset, verify SHD signal
5. Audio loopback: Test TX/RX paths with signal generator/scope
6. Protection test: Apply controlled overvoltage to verify MOV clamping

Design Notes
------------

- Carefully manage ground domains: AG, DG, and BG should be star-connected as per best practices.
- VB- (-48V) and VB+ (+12V) supplies must be isolated and protected.
- See datasheet for specific application examples and protection recommendations.

References
----------

**Hardware Documentation:**

- `HC-5504B Datasheet (Intersil/Renesas) <https://www.renesas.com/us/en/document/dst/hc-5504b-datasheet>`_
- Application notes and reference designs from datasheet

**Firmware Integration:**

- :doc:`../firmware/phone-hardware` - Off-hook detection and phone hardware monitoring implementation
- ``main/hardware/phone_hardware.c`` - Source code for SLIC interface monitoring
- ``main/app/pin_assignments.h`` - GPIO pin definitions

.. note::

   This documentation describes the complete SLIC circuit design. For firmware implementation details on how the ESP32 interfaces with the SLIC hardware (off-hook detection, ring control), see the firmware documentation linked above.

