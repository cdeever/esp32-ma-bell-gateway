Voice and Tone Audio
=====================

The analog telephone interface must support **bidirectional voice transmission** and **in-band signaling tones**, all delivered across just two wires: **tip and ring**. These audio paths must replicate the expectations of legacy Bell System telephony while remaining compatible with modern embedded systems and Bluetooth audio routing.

Target Parameters
-----------------

Historically, analog telephone audio adheres to the following standards:

- **Impedance:** 600 Ω balanced (nominal)
- **Voice Bandwidth:** 300 Hz to 3400 Hz (standard “voice band”)
- **Signal Level:**
  - Transmit (talk-out): ~0 dBm nominal
  - Receive (talk-in): –10 to –20 dBm typical
- **Isolation:** Galvanic isolation from line circuitry is essential
- **Tones:** In-band tones (dial tone, ringback, busy, etc.) occupy same audio path as speech

.. note::
   **What is Galvanic Isolation?**  
   Galvanic isolation means there is **no direct electrical connection** between two circuits—only signal transfer via magnetic, optical, or capacitive coupling. This prevents high voltages or ground loops from damaging sensitive electronics or endangering users.  
   Common methods include transformers and optocouplers.

Audio Functions
---------------

Two independent analog paths must be supported:

- **Earpiece / Receive Path (to headset):**
  - Delivers voice and tones from the system into the phone’s earpiece
- **Microphone / Transmit Path (from handset):**
  - Accepts voice from the microphone and routes it into the system for digitization or Bluetooth transmission

Both paths share the same tip-ring pair, requiring either a **hybrid** circuit (2-wire to 4-wire conversion) or **time-domain multiplexing** to avoid feedback or echo.

Historical Evolution
--------------------

The Bell System employed a variety of techniques over the decades to achieve clear, isolated, full-duplex audio across a two-wire line:

**Carbon Microphone & Induction Coil (1900s–1940s):**
- Carbon mics modulated DC current; an induction coil (hybrid transformer) separated mic and speaker paths.
- Tone signals were generated centrally and injected via coupling networks.
- Very robust, but audio fidelity was limited.

**Hybrid Transformers and Voice Frequency Signaling (1950s–1980s):**
- Voice signals and tones shared a 2-wire loop.
- Hybrids converted between 2-wire (loop) and 4-wire (switching equipment) to manage echo and gain.
- Sine wave tones (e.g., DTMF, dial tone) were injected and filtered per ITU-T Q.23 standards.

**Integrated Line Interface Circuits (1990s–Present):**
- SLICs include balanced drivers, hybrids, and filters internally.
- Audio is digitized near the line, with signal paths managed digitally.
- Provide built-in gain, echo cancellation, and tone injection in VoIP gear.

Receive Path: Earpiece Driver
-----------------------------

The earpiece must be driven with filtered voice-band signals at ~0 dBm nominal. This requires impedance matching and isolation from logic circuitry.

**Design Options:**

.. list-table::
   :header-rows: 1
   :widths: 25 40 35

   * - **Method**
     - **Description**
     - **Trade-Offs**
   * - **Transformer Output Stage**
     - Audio passed through a 600 Ω transformer to the earpiece coil.
     - Excellent isolation and historic fidelity. Requires amplification.
   * - **Differential Amplifier**
     - Op-amp or Class-D driver configured for 600 Ω output.
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
     - Mic signal passed through 600 Ω transformer to preamp.
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

**Tone Injection Strategies:**

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

Modern methods include:

- **Transformer hybrids** (inductive and matched for best rejection)
- **Op-amp hybrids** (active differential networks)
- **Digital echo cancellation** (in SLICs or DSP chips)

Summary
-------

- Voice and tone audio share the same physical pair: tip and ring
- Line audio is balanced 600 Ω, 300–3400 Hz bandwidth
- Earpiece path must drive ~0 dBm across 600 Ω
- Microphone path must present matching impedance and provide gain
- Tone signals must mix or switch into the receive path
- Use transformers or hybrids for best isolation and echo control
- SLICs now handle these functions in a single IC, but discrete circuits remain viable and educational

.. note::
   For audio experimentation, always use **isolated test setups** and appropriate impedance loads. Never connect ESP32 audio outputs directly to phone line wiring without proper coupling.
