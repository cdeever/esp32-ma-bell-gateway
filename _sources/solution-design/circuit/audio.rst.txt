====================
Voice and Tone Audio
====================

Overview
========

This page outlines the analog circuitry needed to support both **voice** and **tone** audio signals in the Ma Bell Gateway project. These signals must be audible on a traditional telephone headset and comply with historical North American telco audio standards.

The circuit must support:

- **Listening**: Receiving voice and tones to the earpiece.
- **Speaking**: Capturing voice from the microphone.
- **Nominal Line Characteristics**:

  - **Impedance**: 600 ohms (balanced)
  - **Voice Bandwidth**: 300 Hz – 3400 Hz
  - **Signal Level**: ~0 dBm transmitted, ~-10 dB received

Circuit Roles
=============

Two distinct signal paths must be designed:

- **Earpiece Path (Receiving)**
- **Microphone Path (Transmitting)**

Each must interface with a high-impedance, low-voltage digital system on one end and 600-ohm analog audio hardware on the other.

Earpiece Path (Receiving)
=========================

This circuit drives the headset’s earpiece using audio or tone signals generated elsewhere in the system.

Circuit Requirements:
---------------------

- Match 600-ohm impedance to the earpiece.
- Drive enough current for audibility (~0 dBm).
- Isolate the ESP32 system from the analog line.
- Filter to restrict frequency to 300–3400 Hz.

Design Options:
---------------

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - **Option**
     - **Description**
     - **Trade-Offs**
   * - Transformer-Coupled Output
     - Audio signal passed through a 600-ohm transformer to the earpiece.
     - Provides excellent isolation and historical authenticity. Requires an additional power amplifier stage to drive the transformer effectively.
   * - Differential Amplifier Stage
     - Uses an op-amp or Class-D driver with impedance-matching resistors.
     - Compact design and easy gain control. However, it lacks historical fidelity and requires careful PCB layout to avoid noise.
   * - Capacitive-Coupled Audio
     - Routes signal through a DC-blocking capacitor with a resistive load.
     - Very simple and low component count. Limited isolation and more vulnerable to noise and ground loops.

Microphone Path (Transmitting)
==============================

This circuit accepts input from a traditional carbon or dynamic microphone and prepares it for digitization.

Circuit Requirements:
---------------------

- Present 600-ohm input impedance.
- Capture voice between 300–3400 Hz.
- Provide gain to bring microphone output to usable level.
- Block DC where required.
- Optional: support single-ended or differential input.

Design Options:
---------------

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - **Option**
     - **Description**
     - **Trade-Offs**
   * - Transformer-Coupled Input
     - Audio from the mic is passed through a 600-ohm transformer before amplification.
     - Provides strong electrical isolation and authentic signal characteristics. Requires additional gain circuitry after the transformer.
   * - Preamp with Resistor Matching
     - FET or op-amp preamp with input resistors selected to match microphone impedance.
     - Small and easily tunable for different mic types. Susceptible to mismatches and DC biasing issues.
   * - Capacitive-Coupled Mic Bias
     - Powers a carbon mic with DC, AC-couples the signal to an amplifier or ADC.
     - Simulates original mic behavior well, but may require fine-tuned biasing and introduce DC offset risks.

Tone Injection Path
===================

In addition to voice, tones (e.g., dial tone, ringback, busy signal) must be injected into the earpiece path.

Circuit Requirements:
---------------------

- Inject tone signals without disturbing the voice path.
- Match impedance and signal level to earpiece driver circuit.
- Support switching or summing strategies.

Design Options:
---------------

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - **Option**
     - **Description**
     - **Trade-Offs**
   * - Analog Summing Network
     - Mix tone and voice signals using passive or active summing network before final driver.
     - Simple and always-on. Can introduce signal bleed-through or require careful gain balancing.
   * - Relay or Analog Switch
     - Uses a mechanical relay or analog mux to select between tone or voice input.
     - Clear signal isolation and full control over path selection. Adds complexity and latency to switching.
   * - Dedicated Tone Driver
     - Tone signal goes through its own amplifier or transformer and joins the earpiece path later.
     - Clean isolation of tone source and adjustable path. Higher component count and more tuning required.

Echo Control Consideration
==========================

- Acoustic echo cancellation (AEC) or analog hybrid circuits may be used to prevent feedback between speaker and microphone.
- These typically require a four-wire interface or active cancellation circuits, often simulated with transformers or op-amp networks.

