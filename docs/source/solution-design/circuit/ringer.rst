Ringer Signaling 
================

Most legacy Bell System ringers were designed to respond to approximately 90V RMS sine wave at 20 Hz. While modern designs may attempt to simulate this with lower voltages or simpler signals, the original sound and behavior is best reproduced with a true sine wave at the standard voltage.

Overview
--------

All ringer options are expected to follow the standard North American cadence of 2 seconds ON and 4 seconds OFF, repeating for the duration of the ring. The difference lies in how the "ON" segment is energized, and particularly, what waveform is used to drive the ringer coil.

Below is the target waveform used by historical central office equipment:

.. figure:: /_images/ideal_co_ring_signal.png
   :alt: Ideal CO ring signal
   :width: 600px
   :align: center

   **Ideal Central Office Ring Signal:** 20 Hz sine wave applied for 2 seconds, followed by 4 seconds off.

Each waveform generation method offers a different level of authenticity and complexity, as described below.

System Integration
------------------

The ESP32 monitors for incoming calls via Bluetooth. When a ring event is detected, it controls the ring cadence (2s ON / 4s OFF) by enabling or disabling the ringer oscillator. Ringing continues until the call is answered (off-hook detected) or terminated. The ESP32 must immediately disable the ring signal if the handset is lifted during ringing (“ring trip”), ensuring safe and correct system behavior.

Design Options
--------------

**Option 1: Square Wave Switching with H-Bridge**

- Alternates polarity of 48V DC at 20 Hz to create a square-wave signal.
- Mimics ringing with sufficient voltage swing to activate the bell.
- Easy to implement with an H-bridge and GPIO timing.
- **Drawback:** Not a true sine wave. Some vintage ringers may sound harsh or fail to trigger.

**Option 2: PWM-Based Pseudo-Sine with Filtering**

- Generates a 20 Hz sine wave using high-frequency PWM from the ESP32.
- Output is passed through a low-pass filter to smooth into a sine-like signal.
- May be amplified using a Class D topology.
- **Drawback:** More complex, requires tuning filter and may not deliver high enough voltage.

**Option 3: Transformer-Based Oscillator (Recommended)**

- Uses a low-frequency oscillator circuit (or 555 timer) to toggle a push-pull driver or half-bridge.
- Feeds a step-up transformer to produce ~90V RMS sine-like AC.
- Transformer provides natural galvanic isolation and impedance matching.
- **Advantage:** Closely replicates the way central offices originally generated ringing voltage.
- **Tradeoff:** Slightly more complex analog design; requires sourcing or winding a suitable transformer.

.. note::
   The transformer in this design provides both the required voltage boost and critical galvanic isolation, protecting both the ESP32 logic and the user from hazardous voltages.

   Most legacy ringers operate best at 90V RMS, but some can respond to lower voltages (60–75V). If using vintage equipment, target the higher end for maximum reliability and authentic sound.

Recommended Direction
---------------------

For this project, the transformer-based oscillator approach (Option 3) is preferred. It provides the most authentic ring waveform and adheres closely to historical Bell System practices. While more complex than simple switching, it offers better compatibility with a wider range of legacy ringers and is ideal for a faithful recreation of vintage telephone behavior.

The final implementation will allow the ESP32 to control the ring cadence by enabling or disabling the oscillator circuit as needed.

Ringing Timeout and Safety
--------------------------

The system needs to implement a **maximum ring timeout** (e.g., 60 seconds), after which ringing is forcibly disabled until reset. This prevents damage or excessive noise in the event of a fault, and avoids excessive wear or overheating of the ringer mechanism.

.. note::
   All high-voltage AC circuitry must be enclosed in a safe, insulated housing with proper clearance and isolation. Follow applicable electrical safety standards and local regulations for high-voltage design and construction.

Block Diagram
-------------

The following diagram shows the flow of the ring signal from the ESP32 through the oscillator and transformer, and into the phone line and ringer:

.. code-block:: none

    [ESP32] ---> [Oscillator] ---> [Transformer] ---> [Phone Line / Ringer]

The ringer signaling path proceeds as follows:

- **ESP32:** Generates timing and cadence signals for ringing events.
- **Oscillator:** Converts the digital control signals into a low-frequency AC waveform (typically 20 Hz).
- **Transformer:** Steps up the AC waveform to the required high voltage (~90V RMS) and provides galvanic isolation.
- **Phone Line / Ringer:** Receives the 20 Hz, 90V RMS AC signal, activating the mechanical bell in a legacy phone.


Summary
-------

   - Target waveform: 20 Hz, ~90V RMS sine wave, 2s ON / 4s OFF cadence
   - Use transformer-based oscillator for authenticity and isolation
   - Ring trip: disable signal immediately when phone is answered
   - Enforce maximum ring duration for safety
   - All high-voltage AC must be securely enclosed and labeled

