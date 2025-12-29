Power Supply Design
===================

Overview
--------

Legacy analog telephone systems operated across multiple voltage domains, each serving a distinct role—subscriber loop voltage, AC ringing, lamp power, and logic control. In recreating this behavior, the Ma Bell Gateway must provide safe and isolated supplies that replicate historical behavior while remaining compatible with modern digital components.

For historical context on Bell System standard voltages, see :doc:`/historical-perspective`.

.. note::
   **Safety First:** Voltages above 48V can be hazardous. High-voltage domains must be clearly labeled, well-isolated, and treated with care. Never connect unisolated high-voltage rails directly to logic circuits or USB-connected equipment.

Power Domains
-------------

This project replicates the classic telecom supply scheme using five distinct voltage sources:

1. **-48V DC** – Simulated Central Office (CO) Subscriber Loop
2. **+12V DC** – SLIC Analog Power Supply
3. **90V AC (or pulsed DC)** – Mechanical Ringer Power
4. **9V AC** – Rotary Dial Illumination
5. **3.3V DC** – Control Logic and Audio Codecs

Each serves a unique function and must be properly isolated.

-48V DC – Subscriber Loop Voltage
---------------------------------

Simulates the idle line voltage applied between **tip and ring**, with **tip at ground and ring at -48V DC**. This voltage powers the loop when a phone goes off-hook and serves as the baseline for tone injection and call supervision.

.. note::
   **Why is it negative?**  
   The Bell System standardized on **-48V DC** to reduce corrosion.  
   With **tip grounded** and **ring at -48V**, stray currents flow **into** exposed metal, inhibiting oxidation.  
   This helped preserve outdoor connectors and relay contacts across millions of subscriber lines.

**Design Options:**

- A **regulated bench power supply** can be used to provide -48V DC directly, with the positive terminal connected to ground (tip) and the negative terminal to ring. This is stable and authentic but bulky for embedded systems.

- A **DC-DC boost converter** may be used to generate -48V from a lower voltage source. This allows compact integration and automation but requires careful filtering and isolation.

- In some embedded designs, a **SLIC (Subscriber Line Interface Circuit)** can internally generate the loop voltage. These are ideal for VoIP integration but may not source enough current for electromechanical ringers.

+12V DC – SLIC Analog Power Supply
-----------------------------------

Modern Subscriber Line Interface Circuits (SLICs) require a positive DC voltage to power their internal analog circuitry. This is separate from the -48V loop voltage and the low-voltage digital logic supply.

For the HC-5504B SLIC used in this design, a **+12V DC** supply powers the analog front-end, including the hybrid transformer, audio path amplifiers, and line feed circuitry. This voltage appears on the VB+ pin of the SLIC and must be stable and well-regulated.

.. note::
   **Why 12V for SLIC Power?**
   The 12V supply for the SLIC allows it to handle the voltage swings needed for audio signals and loop current regulation while remaining within safe limits for integrated circuits. This voltage is lower than the -48V loop voltage, but higher than the 3.3V logic supply, providing the necessary headroom for analog signal processing.

**Design Options:**

- A **regulated 12V power supply** (linear or switching) can directly provide the SLIC with clean, stable power. This is simple and reliable for bench testing.

- A **DC-DC buck converter** can step down from a higher voltage (e.g., 48V main supply) to provide 12V efficiently. This is ideal for integrated designs where a single main supply feeds all voltage rails.

- A **linear regulator (LDO)** may be used if stepping down from a moderate voltage (e.g., 24V → 12V), though efficiency will be lower than a switching converter. This can provide very clean power with minimal switching noise.

**Current Requirements:**

The SLIC's 12V supply typically draws 50-150mA depending on operating mode (idle vs. active call). Design for at least 200mA capacity to ensure adequate margin.

Ringer Power (~90V AC)
----------------------

Classic electromechanical ringers require a 20Hz, 90V RMS sine wave. In modern designs, this can be approximated using transformer-based oscillators or boosted Class-D drivers.

**Design Options:**

- A **transformer-based oscillator** is the most authentic method, using a 20Hz waveform (from a 555 or digital timer) to drive a push-pull transistor circuit that steps up to 90V AC. This closely replicates historical CO equipment.

- A **PWM-based Class-D audio driver** combined with a step-up transformer can also produce the needed waveform. This approach is more digital but requires tuning and careful filtering.

- A **pulsed 48V DC signal** is a simple alternative. While not a sine wave, many ringers will respond to 2s ON / 4s OFF pulses at 48V. This compromises authenticity but is safer and easier to implement.

9V AC – Dial Illumination
-------------------------

Many rotary phones include a lamp behind the dial, powered by a low-voltage AC signal (often between 6–12V). This is typically delivered over the **black/yellow** pair on RJ11 jacks.

**Design Options:**

- A **9V wall adapter** (AC type) provides simple, isolated power for the dial lamp. This is straightforward and requires no internal transformer.

- A **low-voltage winding on a custom transformer** can provide built-in illumination supply within the main PSU enclosure. This is compact and authentic but requires thermal planning.

- If dynamic control is needed, an **electrically isolated relay or triac** can be used to switch the AC lamp circuit in sync with off-hook detection.

3.3V DC – Control and Audio Codecs
-----------------------------------

Used to power digital controllers (ESP32 microcontroller), audio DACs/ADCs (PCM5100, PCM1808), and other low-voltage peripherals.

.. note::
   **Audio Codec Power Quality:**
   Audio DACs and ADCs (e.g., PCM5100, PCM1808) are sensitive to power supply noise. Clean, low-noise 3.3V power is critical for high-quality audio reproduction. Ripple and switching noise can degrade signal-to-noise ratio and introduce audible artifacts. Use adequate decoupling (0.1µF ceramic + 10µF bulk capacitors) and consider using low-dropout (LDO) post-regulation if the primary buck converter has significant output ripple.

**Design Options:**

- A **USB power supply** combined with an LDO regulator is often the simplest way to deliver 3.3V. However, current is limited and this may not be suitable for peripherals with high demand.

- A **switchmode buck converter** can step down 12V or 24V to 3.3V or 5V with excellent efficiency. This is ideal for larger systems with audio amplifiers, displays, or relays. For audio codec power, choose buck converters with low output ripple (<50mV) or add LDO post-regulation.

- **Pre-built PSU modules** with dedicated outputs (e.g., 3.3V 1A) are compact and offer clean power, but may increase BOM cost. Verify ripple specifications if powering audio codecs directly.

Fuse & Surge Protection
-----------------------

To protect logic and user interfaces, each power domain should include:

- **Fast-blow or PTC fuses** on high-voltage inputs
- **TVS diodes** or clamping zeners for surge suppression
- **Flyback diodes** across relay coils
- **Snubber networks** for AC ringing signals

.. note::
   Design for **fault tolerance**. Ensure no fault in one domain can backfeed into others. Power rails should be fused or decoupled.

Power Routing and Isolation
---------------------------

To prevent accidental cross-domain exposure:

- Use **galvanic isolation** between power domains via:

  - Transformers  
  - Opto-isolators  
  - Relays

- Keep grounds isolated: **Logic GND ≠ -48V GND**
- Route each supply in its own section of PCB or enclosure
- Label all connectors with voltage and polarity

Summary
-------

The use of multiple, purpose-specific power supplies allows the Ma Bell Gateway to faithfully recreate the behavior of traditional phone equipment while protecting modern digital components.

- **-48V DC**: Simulates the CO loop voltage and drives current to the phone when off-hook
- **+12V DC**: Powers the SLIC's analog circuitry for audio processing and line interface functions
- **90V AC (or pulsed DC)**: Activates mechanical ringers in the authentic cadence
- **9V AC**: Powers vintage dial lamps through the outer RJ11 pair
- **3.3V DC**: Powers modern control logic (ESP32) and audio codecs (PCM5100 DAC, PCM1808 ADC)

Proper isolation, protection, and layout are essential for safe and functional operation across these mixed-voltage domains.
