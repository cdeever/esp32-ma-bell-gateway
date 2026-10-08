Line Interface: AG1171 SLIC Module (Prototyping)
================================================

The **AG1171 SLIC module** (commonly sold as the **KS0835F**, advertised "AG1171 / AG1170
compatible") is the **recommended line interface for prototyping** the Ma Bell Gateway.
It is a complete single-channel Subscriber Line Interface Circuit in a 14-pin SIL package
with an *integral DC/DC converter*, so it provides the full BORSCHT line interface — real
48V battery feed, 2-wire ↔ 4-wire hybrid, and ringing — from a **single +3.3V to +5V
supply, with no high-voltage hardware**.

.. note::

   This page documents the AG1171 module as the easy, low-voltage **prototyping** line
   interface. The :doc:`HC-5504B <line-interface-hc5504b>` remains the project's
   **production / from-scratch reference design**. The two are interchangeable at the
   ESP32 interface (with the polarity and ringing differences noted below). For the
   hands-on bring-up procedure, see ``impl/slic-module-build-guide.md``.

Overview
--------

- Single supply, **+3.3V to +5V** — the on-board DC/DC converter generates the ~48V
  battery feed and the ~65 Vrms / 20 Hz ringing voltage internally.
- Provides a constant ~30 mA loop feed and 600Ω line impedance (no programming resistors).
- Real 2-wire to 4-wire hybrid with 0 dB gain to ground-referenced ``VIN`` / ``VOUT``.
- Rings a real bell: ≥40 Vrms into **2 mechanical bells** (or 3 tone ringers).
- Off-hook and loop-disconnect (pulse) dialing reported on a single ``SHK`` output.
- Minimal external parts: supply caps, two audio coupling caps, ESD diodes.
- **No −48V, +12V, or 90V AC rails, and no external ring generator or relay.**

Pinout Summary (14-pin SIL)
---------------------------

.. list-table::
   :header-rows: 1
   :widths: 8 14 12 40

   * - Pin
     - Name
     - Direction
     - Description
   * - 1
     - RING (B)
     - --
     - Subscriber line Ring
   * - 2
     - TIP (A)
     - --
     - Subscriber line Tip
   * - 3
     - F/R
     - Input
     - Forward/Reverse polarity. Toggle at 20–25 Hz to ring; hold HIGH for normal calls
   * - 4
     - RM
     - Input
     - Ring Mode. HIGH during ringing, LOW otherwise (≤33% duty cycle)
   * - 5
     - SHK
     - Output
     - Switch Hook. HIGH = off-hook; pulses during pulse dialing. Internal 3.3kΩ pull-up
   * - 6–8
     - NC
     - --
     - No connection
   * - 9
     - VIN
     - Input
     - Audio in from codec (DAC → line). Couple via 10nF
   * - 10
     - VOUT
     - Output
     - Audio out to codec (line → ADC). Couple via 100nF
   * - 11
     - NC
     - --
     - No connection
   * - 12
     - GNDPWR
     - --
     - DC/DC ground
   * - 13
     - +VPWR
     - --
     - +3.3V to +5V supply input
   * - 14
     - PD
     - Input
     - Power-down. Leave OPEN for free-run; logic-L powers down. **Never drive HIGH**

Application Circuit
-------------------

The AG1171 needs only a handful of external parts (Silvertel datasheet Figure 4):

- ``C1`` 100nF and ``C2`` 470µF/16V across ``+VPWR`` / ``GNDPWR`` (supply bypass + bulk).
- ``C3`` 100nF in series with ``VOUT`` and ``C4`` 10nF in series with ``VIN`` (audio
  coupling to the codec).
- ESD protection diodes on TIP/RING for on-premise use (``D1`` BZT03C82/P6KE82, ``D2–D5``
  1N4004/MB4S bridge), and ``D6`` 1N4148 on ``PD`` to guarantee it is never pulled high.

Many KS0835F carrier boards already populate the supply caps and protection diodes; the
audio coupling caps usually must be added externally.

::

    +3.3V to +5V ──┬── C1(100nF) ──┐
                   └── C2(470µF) ──┴── GNDPWR
                   │
                   +VPWR
            ┌──────┴───────────────────────────┐
    TIP ────┤2 TIP        AG1171 / KS0835F   F/R├3──── ESP32 (ring oscillation)
    RING ───┤1 RING                          RM ├4──── ESP32 GPIO 13
            │                                SHK├5──── ESP32 GPIO 32 / 34
            │              VOUT 10├──C3(100nF)──────── PCM1808 VINL (ADC)
            │              VIN   9├──C4(10nF)───────── PCM5100 OUTL (DAC)
            │                 PD 14├ (open)            │
            └──────────────────────────────────────────┘
                   │
                  GNDPWR ── common ground

ESP32 GPIO Interface
--------------------

The AG1171 maps onto the firmware's existing SLIC GPIOs (defined in
``main/config/pin_assignments.h``):

.. list-table::
   :header-rows: 1
   :widths: 10 18 14 12 36

   * - Module Pin
     - Function
     - ESP32 GPIO
     - Direction
     - Description
   * - 5
     - SHK (Switch Hook)
     - GPIO 32 + GPIO 34
     - Input
     - Off-hook detect **and** pulse dialing — one output feeds both inputs
   * - 4
     - RM (Ring Mode)
     - **GPIO 13**
     - **Output**
     - Ring-mode enable + 2s/4s cadence
   * - 3
     - F/R (Forward/Reverse)
     - To be assigned
     - Output
     - 20–25 Hz ring oscillation; HIGH for normal calls
   * - 9
     - VIN (Audio In)
     - Via Audio Codec
     - Input to module
     - Audio from ESP32 → DAC → module → phone
   * - 10
     - VOUT (Audio Out)
     - Via Audio Codec
     - Output from module
     - Audio from phone → module → ADC → ESP32
   * - 14
     - PD (Power Down)
     - Not Connected
     - --
     - Leave open for free-run

Signal Level and Firmware Notes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The module's logic is 3.3V-compatible and connects directly to the ESP32. Two differences
from the HC-5504B affect firmware:

- **SHK polarity is active-HIGH** (HIGH = off-hook), whereas the firmware's
  ``PIN_OFF_HOOK_DETECT`` and the HC-5504B SHD are active-LOW. Invert the hook/pulse read
  in firmware for this build, or add a single hardware inverter on SHK.
- **Ringing requires an F/R oscillation.** Set RM HIGH and toggle F/R at 20–25 Hz for the
  ring burst (the HC-5504B's RC pin enables ringing on its own). On off-hook during ring,
  SHK pulses (ring-trip): debounce 10 ms, drive F/R permanently HIGH, then drop RM LOW
  within >10 ms and <500 ms. Implement the F/R toggle on a spare GPIO, or use an external
  ~20 Hz oscillator gated by GPIO 13.

Audio Signal Path
-----------------

The module's hybrid presents ground-referenced audio at ``VIN`` (receive) and ``VOUT``
(transmit) at 0 dB gain, intended to drive a codec directly:

::

    PCM5100 OUTL ──┤10nF├── VIN  (pin 9)    receive: far-end / tones → earpiece
    PCM1808 VINL ──┤100nF├── VOUT (pin 10)   transmit: phone mic → far end

Because this is a true hybrid (not the v1.0 AC-coupled loop), the far-end echo of the
low-voltage rig is eliminated — no firmware AEC or transformer hybrid is required. The I2S
connections from the codecs to the ESP32 are unchanged from Sub-Assembly A (see
:doc:`pin-assignments`). If the ~2 Vrms DAC full-scale overdrives ``VIN``, reduce the codec
output level in firmware rather than adding hardware attenuation.

Power Supply Integration
------------------------

A single regulated **+3.3V to +5V** rail powers the module; the integral DC/DC converter
makes everything else. There is **no −48V, +12V, or 90V AC** rail in this design.

Supply current is significant and must be planned for (datasheet §8.3):

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Condition
     - Current
     - Notes
   * - On-hook (idle)
     - ~60–70 mA
     - CLI bias resistor not fitted
   * - Off-hook (active)
     - ~290–360 mA
     - 300Ω load
   * - Ringing
     - up to ~500 mA
     - 1 REN load

.. warning::

   Do **not** power the module from the ESP32's USB/3V3 rail — off-hook and ringing peaks
   will brown out the ESP32. Use a dedicated regulated supply rated for **≥1 A**, with its
   ground tied to the common ground. Keep ``+VPWR`` / ``GNDPWR`` tracks short and thick and
   place the 470µF + 100nF caps directly at the module. Absolute maximum supply is 7.0V —
   exceeding it, even briefly, will destroy the module.

Line Protection
---------------

The AG1171 is designed for short, **on-premise** loops (FCT/FWT/WLL/VoIP). Per the
datasheet, power-cross and lightning protection are *not* required for on-premise use; only
ESD protection is needed, via the low-cost diode network in the application circuit. This
is appropriate for bench prototyping. For field/off-premise deployment, the production
:doc:`HC-5504B <line-interface-hc5504b>` design with full MOV/PTC/bridge protection should
be used instead.

Component Selection & Bill of Materials
---------------------------------------

.. list-table::
   :header-rows: 1
   :widths: 20 24 20 36

   * - Ref
     - Component
     - Value / Part
     - Notes
   * - U2 (alt)
     - SLIC module
     - KS0835F (AG1171/AG1170 compatible)
     - Prototyping line interface; replaces HC-5504B + ring generator
   * - C1
     - Supply bypass
     - 100nF ceramic
     - +VPWR / GNDPWR
   * - C2
     - Supply bulk
     - 470µF, 16V electrolytic
     - +VPWR / GNDPWR
   * - C3
     - Audio coupling (VOUT)
     - 100nF ceramic
     - Module VOUT → ADC
   * - C4
     - Audio coupling (VIN)
     - 10nF ceramic
     - DAC → module VIN
   * - D1
     - ESD clamp
     - BZT03C82 or P6KE82
     - One per line (often on carrier board)
   * - D2–D5
     - Bridge / ESD
     - 1N4004 or MB4S
     - TIP/RING protection (often on carrier board)
   * - D6
     - PD protection
     - 1N4148 or BAS16
     - Ensures PD never pulled HIGH
   * - PS
     - Supply
     - Regulated 3.3–5V, ≥1A
     - Dedicated; not the ESP32 USB rail

Design Notes
------------

- This module makes the AG1171 named in the project risk register (``specs/risk-register.md``,
  RISK-H001) a validated, low-effort prototyping alternative to the HC-5504B.
- SHK carries both hook state and pulse-dial pulses — wire it to both ``PIN_OFF_HOOK_DETECT``
  (GPIO 32) and ``PIN_PULSE_DIAL_IN`` (GPIO 34), as the low-voltage rig does with its shared
  current-sense event.
- Ringing is host-paced: keep RM duty cycle ≤33% and switch ringing off (RM LOW) within
  500 ms of ring-trip to limit power dissipation.

References
----------

**Hardware Documentation:**

- `Silvertel Ag1171 Datasheet (V1.6) <https://silvertel.com/images/datasheets/Ag1171-datasheet-Low-cost-ringing-SLIC-with-single-supply.pdf>`_
- `Silvertel Ag1171 product page <https://silvertel.com/ag1171/>`_

**Build & Firmware Integration:**

- ``impl/slic-module-build-guide.md`` — step-by-step bring-up with this module
- :doc:`line-interface-hc5504b` — production / from-scratch SLIC reference design
- :doc:`../firmware/phone-hardware` — SLIC interface monitoring in firmware
- ``main/config/pin_assignments.h`` — GPIO pin definitions

.. note::

   To fully exercise this module the firmware needs two adjustments (active-HIGH hook read,
   and a 20–25 Hz F/R ring oscillation). Until then, every feature except ringing the
   bell works with the module as-is.
