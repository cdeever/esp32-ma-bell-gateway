=================
Ringer Signaling 
=================

Most legacy Bell System ringers were designed to respond to approximately 90V RMS sine wave at 20 Hz. While modern designs may attempt to simulate this with lower voltages or simpler signals, the original sound and behavior is best reproduced with a true sine wave.

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

Recommended Direction
---------------------

For this project, the transformer-based oscillator approach (Option 3) is preferred. It provides the most authentic ring waveform and adheres closely to historical Bell System practices. While more complex than simple switching, it offers better compatibility with a wider range of legacy ringers and is ideal for a faithful recreation of vintage telephone behavior.

The final implementation will allow the ESP32 to control the ring cadence by enabling or disabling the oscillator circuit as needed.
