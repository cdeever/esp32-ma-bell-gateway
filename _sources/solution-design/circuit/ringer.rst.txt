Ringer Signaling
================

Legacy analog telephone ringers—especially those from the Bell System—were designed to respond to a **90 V RMS, 20 Hz sine wave**, typically delivered in a 2-seconds-on, 4-seconds-off cadence. This waveform was applied between the **tip and ring** conductors of the subscriber loop. While modern systems may use approximations or lower-voltage signals, classic electromechanical ringers operate best when driven with this original waveform profile.

.. figure:: /_images/ideal_co_ring_signal.png
   :alt: Ideal CO ring signal
   :width: 600px
   :align: center

   **Ideal Central Office Ring Signal:** 20 Hz sine wave applied for 2 seconds, followed by 4 seconds off.

Cadence Behavior
----------------

Standard North American ringing cadence is:

- **2 seconds ON** – Apply 20 Hz sine wave at high voltage (~90 V RMS)
- **4 seconds OFF** – Silence (open or high-impedance)

This 6-second pattern repeats until the call is answered or canceled. Cadence generation is separate from waveform generation. Modern controllers (e.g., microcontrollers or line cards) typically gate the ringer oscillator circuit to enforce this timing.

Historical Evolution
--------------------

Ringing signal generation evolved significantly during the 20th century, from simple electromechanical systems to precise solid-state circuits. Below is a historical progression of how the ringing waveform was created in central office and PBX systems.

**Magneto Generators (1890s–1920s):**
- Hand-cranked magnetos created high-voltage AC directly.
- Common in manual systems or rural subscriber lines.
- Produces variable-frequency ringing depending on crank speed.

**Motor-Generator Sets (1920s–1950s):**
- Electromechanical generator powered by line current or mains.
- Produced consistent 20 Hz sine wave for CO applications.
- Often part of early automatic exchanges.

**Synchronous Vibrators / Interrupters (1950s–1960s):**
- Used mechanical contacts and magnetic coils to chop DC into alternating current.
- Stepped up with transformers to ~90 V AC.
- Still relied on analog components and precise tuning.

**Oscillator-Driven Transformers (1970s–1990s):**
- Replaced mechanical elements with transistor-based oscillators (e.g., Wien bridge, multivibrators).
- 20 Hz waveform amplified and fed into a step-up transformer.
- Galvanic isolation and voltage swing achieved via transformer.
- Dominant method in later analog PBX systems.

**Line Interface ICs (1990s–Present):**
- VoIP ATAs and digital PBXs use **Subscriber Line Interface Circuits (SLICs)** with integrated ringing generators.
- Some SLICs produce true 20 Hz sine waves internally, while others use waveform tables and DACs.
- Voltage often limited (~50–75 V RMS), sufficient for modern phones but marginal for older bells.

Approaches to Emulating Ring Signal
-----------------------------------

Several methods are used today to generate ring signals in embedded or hobbyist systems:

**1. Square Wave with H-Bridge Switching**
- Alternates polarity of DC power (e.g., ±48 V) using an H-bridge.
- Simple and effective for some ringers, but waveform is harsh.
- May fail to trigger high-impedance or frequency-sensitive bells.

**2. PWM-Filtered Sine Simulation**
- Uses high-speed PWM to approximate a sine wave, then filters it.
- Can generate low-frequency AC with minimal hardware.
- Requires tuning and cannot easily achieve high voltages.

**3. Transformer-Based Oscillator**
- Transistor or IC-based oscillator (e.g., using a 555 timer) drives a push-pull or half-bridge stage.
- Output is passed through a step-up transformer to produce ~90 V RMS at 20 Hz.
- Provides galvanic isolation and authentic signal shape.
- Closely mirrors late-era Bell System and PBX designs.

.. note::
   Vintage ringers perform best when driven with 90 V RMS. Some units may ring at 60–75 V, but higher voltage ensures reliable activation and correct mechanical response.

System Integration
------------------

In modern designs, a controller (e.g., ESP32, PBX CPU, or SLIC logic) manages ring timing and gating. This is usually implemented by toggling the oscillator or enabling a driver stage, with provisions for:

- **Cadence control:** 2s ON / 4s OFF timing
- **Ring trip detection:** Disable ring immediately when off-hook is detected
- **Timeout:** Ringing disabled after a maximum time (e.g., 60s) to prevent overheating or user fatigue

.. code-block:: none

    [Controller] ---> [Oscillator or DAC] ---> [Amplifier or Transformer] ---> [Phone Line / Ringer]

Signal Flow Summary:

- **Timing logic** creates the cadence.
- **Oscillator** produces 20 Hz waveform.
- **Driver + Transformer** amplifies and isolates signal to reach 90 V RMS.
- **Ringer coil** responds by physically striking the bell.

Safety Considerations
---------------------

- Ring signal circuits must be **fully isolated** from low-voltage logic and user-accessible surfaces.
- Use **enclosed transformers, insulated wiring**, and appropriate creepage/clearance rules.
- Follow applicable safety codes for high-voltage AC systems.

Summary
-------

- Target waveform: **20 Hz sine wave**, ~90 V RMS, **2s ON / 4s OFF** cadence
- Historical generation: From magnetos to transformer-based oscillators to SLICs
- Transformer-based circuits remain the **closest modern analog** to Bell System designs
- Ring signal must cease immediately on **ring trip** (off-hook detection)
- Include timeout and **fail-safe logic** to avoid continuous ringing in fault conditions
