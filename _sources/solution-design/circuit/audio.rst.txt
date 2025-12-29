Voice and Tone Audio Design
===========================

This section covers design alternatives for implementing voice and tone audio paths in the Ma Bell Gateway. For historical background on Bell System audio standards and evolution, see :doc:`/historical-perspective`.

The design must support bidirectional voice and in-band signaling tones across the tip/ring pair, with 600 Ω impedance matching and galvanic isolation.

Receive Path: Earpiece Driver
-----------------------------

The earpiece must be driven with filtered voice-band signals at ~0 dBm nominal. This requires impedance matching and isolation from logic circuitry.

**Design Options:**

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - **Method**
     - **Description**
     - **Trade-Offs**
   * - **Transformer Output Stage**
     - Audio passed through a 600 Ω transformer to the earpiece coil.
     - Excellent isolation and historic fidelity. Requires amplification.
   * - **Differential Amplifier**
     - Op-amp or Class-D driver configured for 600 Ω output.
     - Compact, tunable, but less authentic and requires filtering.
   * - **Capacitive Coupling**
     - Simple capacitor + resistive load to block DC.
     - Low component count but limited isolation and prone to noise.

Transmit Path: Microphone Input
-------------------------------

Carbon or dynamic microphones require impedance matching, gain, and DC biasing. Older mics modulate line current; newer ones output low-level AC signals.

**Design Options:**

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - **Method**
     - **Description**
     - **Trade-Offs**
   * - **Transformer Input Stage**
     - Mic signal passed through 600 Ω transformer to preamp.
     - High isolation and noise immunity. Requires post-gain.
   * - **Preamp with Matched Resistors**
     - Op-amp or FET with input resistor matched to mic impedance.
     - Compact, but more sensitive to DC offset and mismatch.
   * - **Capacitive-Biased Carbon Mic**
     - Bias voltage applied; AC coupled to gain stage.
     - Mimics original circuit behavior. Requires tuning and stability considerations.

Tone Injection
--------------

All tones used in legacy telephony (e.g., dial tone, ringback, busy, reorder) fall within the voice band and are **injected into the same path** as the receive audio.

**Design Options:**

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - **Method**
     - **Description**
     - **Trade-Offs**
   * - **Passive Analog Mixing**
     - Resistor-based summing network to blend tone + voice signals.
     - Simple, but fixed blend. Can cause bleed-through or level conflicts.
   * - **Active Summing Amplifier**
     - Op-amp mixer sums multiple audio sources.
     - Tunable gain and impedance. More complexity.
   * - **Relay or Analog Switch**
     - Physically switch between tone and voice sources.
     - Full isolation. May introduce switching noise or latency.

Hybrid and Echo Control
-----------------------

To prevent speaker signal from leaking into the mic path, legacy systems used **hybrid transformers** or **2-wire to 4-wire converters**. These circuits separate transmit and receive audio over the same line.

**Design Options:**

- **Transformer hybrids** (inductive and matched for best rejection)
- **Op-amp hybrids** (active differential networks)
- **Digital echo cancellation** (in SLICs or DSP chips)

Digital Audio Interface (ESP32 Side)
------------------------------------

Between the ESP32 microcontroller and the analog telephone line, digital-to-analog and analog-to-digital conversion is required. The ESP32 provides I2S (Inter-IC Sound) as its primary interface for high-quality audio.

Audio Output (DAC)
^^^^^^^^^^^^^^^^^^

The Ma Bell Gateway uses an **external DAC connected via I2S** for all audio output, including tones and Bluetooth-transmitted voice.

**Design Options:**

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - **Method**
     - **Description**
     - **Trade-Offs**
   * - **External I2S DAC**
     - Dedicated audio DAC (e.g., PCM5100) with 16-bit+ resolution
     - High audio quality, hardware-configured operation, production-ready
   * - **ESP32 Built-in DAC**
     - GPIO25/26 with 8-bit resolution
     - Insufficient for voice quality; suitable only for prototyping
   * - **PWM Audio (LEDC)**
     - Square-wave generation filtered to analog
     - Adequate for simple tones but introduces switching noise

Audio output is routed from the external DAC to the telephone earpiece via the SLIC line interface, with proper impedance matching and AC coupling.

Audio Input (ADC)
^^^^^^^^^^^^^^^^^

The Ma Bell Gateway uses an **external ADC connected via I2S** for capturing microphone audio from the telephone handset.

**Design Options:**

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - **Method**
     - **Description**
     - **Trade-Offs**
   * - **External I2S ADC**
     - Dedicated audio ADC (e.g., PCM1808) with high SNR
     - Clear voice capture, digital noise immunity, legacy mic compatible
   * - **ESP32 Built-in ADC**
     - GPIO36-39 designed for sensor readings
     - Insufficient sample rate and quality for telephony
   * - **I2S Digital Microphone**
     - MEMS microphone with digital output
     - Would require replacing vintage phone's microphone

The audio input path interfaces with the Bluetooth HFP stack to transmit voice from the telephone handset to the paired mobile device.

Summary
-------

When selecting an audio design approach for the Ma Bell Gateway:

- **Transformer-based stages** offer the best isolation and authentic Bell System behavior
- **Active circuits** provide flexibility and compact footprint
- **SLIC ICs** (like the HC-5504B) integrate hybrid, codec, and audio functions in one chip
- **External I2S DAC/ADC** provides high-quality digital audio interface to the ESP32

For the Ma Bell Gateway implementation using the HC-5504B SLIC and external audio codecs, see :doc:`/implementation/circuit/line-interface-hc5504b` and :doc:`/implementation/firmware/audio-subsystem`.

.. note::
   For audio experimentation, always use **isolated test setups** and appropriate impedance loads. Never connect ESP32 audio outputs directly to phone line wiring without proper coupling.
