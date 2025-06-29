Ringer Signaling
================

Legacy analog telephone ringers—especially those from the Bell System—were designed to respond to a **90 V RMS, 20 Hz sine wave**, typically delivered in a 2-seconds-on, 4-seconds-off cadence. This waveform was applied between the **tip and ring** conductors of the subscriber loop. While modern systems may use approximations or lower-voltage signals, classic electromechanical ringers operate best when driven with this original waveform profile.

Cadence Behavior
----------------

Standard North American ringing cadence:

- **2 seconds ON** – Apply 20 Hz sine wave at high voltage (~90 V RMS)
- **4 seconds OFF** – Silence (open or high-impedance)

.. figure:: /_images/ideal_co_ring_signal.png
   :alt: Ideal CO ring signal
   :width: 600px
   :align: center

This 6-second pattern repeats until the call is answered or canceled. Cadence generation is separate from waveform generation. Various controllers (electromechanical, analog, or digital) may gate the ring oscillator circuit to enforce this timing.

Historical Evolution
--------------------

Ringing signal generation has evolved over more than a century. Major methods include:

**Magneto Generators (1890s–1920s):**
- Hand-cranked magnetos produced high-voltage AC directly.
- Used mainly in manual or rural systems.
- Output frequency and voltage depended on cranking speed.

**Motor-Generator Sets (1920s–1950s):**
- Electromechanical generators powered by mains or battery.
- Provided a stable 20 Hz sine wave for central office use.

**Synchronous Vibrators / Interrupters (1950s–1960s):**
- Mechanical choppers or interrupters switched DC into AC, stepped up with transformers.
- Required tuning and maintenance.

**Oscillator-Driven Transformers (1970s–1990s):**
- Solid-state oscillators (e.g., Wien bridge, 555 timer) generated a 20 Hz waveform, amplified and stepped up using a transformer.
- Enabled compact, robust ringing circuits for PBX and intercom systems.

**Line Interface ICs (1990s–Present):**
- Subscriber Line Interface Circuits (SLICs) may include integrated ringing drivers.
- Some SLICs generate the waveform internally; others require an external signal.
- Voltage and frequency are optimized for compatibility with modern and legacy ringers.

Approaches to Emulating Ring Signal
-----------------------------------

Contemporary and hobbyist systems may employ a variety of approaches to generate ringing signals for analog phones:

**1. Square Wave with H-Bridge Switching**
- Alternates the polarity of a DC supply (e.g., ±48 V) using an H-bridge.
- Simple to implement, but the resulting waveform can be harsh and less effective on vintage high-impedance ringers.

**2. PWM-Filtered Sine Simulation**
- Uses high-frequency PWM output filtered to approximate a sine wave at 20 Hz.
- Allows waveform shaping with minimal hardware, though achieving sufficient voltage for classic ringers may require additional amplification or step-up circuitry.

**3. Transformer-Based Oscillator (Analog or MCU-Driven)**
- Employs a low-frequency oscillator (such as a 555 timer, microcontroller PWM, or DAC) to drive a power amplifier or H-bridge.
- The amplified signal feeds a step-up transformer, producing ~90 V RMS at 20 Hz for the ringer.
- This approach provides galvanic isolation, robust voltage swing, and a waveform profile **closest to authentic Bell System signals**.
- The oscillator can be implemented using analog circuits or digitally via a microcontroller, enabling flexible frequency, cadence, and pattern control.

.. important::
   **Authentic Bell System Ringing:**  
   For true compatibility with vintage electromechanical ringers, the signal must be a **20 Hz sine wave at approximately 90 V RMS**.  
   - Lower voltages, non-sinusoidal waveforms, or different frequencies may cause weak, erratic, or silent ringing.
   - Many telephone ringers—especially those made before the 1980s—are sensitive to both frequency and waveform shape.

Summary
-------

- The traditional ring signal for legacy telephones is a **20 Hz sine wave at ~90 V RMS**, typically delivered in a repeating cadence (2 s ON, 4 s OFF).
- A range of historical and modern circuits can generate this signal, from hand-cranked magnetos to transformer-based solid-state oscillators and advanced SLIC ICs.
- **Transformer-based oscillator designs**—whether driven by analog circuitry or a microcontroller—offer a practical and authentic solution for emulating central office ringing signals.
- Selection of design approach depends on system requirements, available components, and compatibility with the target telephone hardware.
