Power Supply Design
===================

Overview
--------

Legacy analog telephone systems operated across multiple voltage domains, each serving a distinct role—subscriber loop voltage, AC ringing, lamp power, and logic control. In recreating this behavior, the Ma Bell Gateway must provide safe and isolated supplies that replicate historical behavior while remaining compatible with modern digital components.

.. note::
   **Safety First:** Voltages above 48V can be hazardous. High-voltage domains must be clearly labeled, well-isolated, and treated with care. Never connect unisolated high-voltage rails directly to logic circuits or USB-connected equipment.

Historical Framing
------------------

The Bell System established standard voltages for line and subscriber equipment by the early 20th century:

- **Subscriber Loop (-48V DC):** Established for consistent talk battery voltage and to minimize corrosion (see below)
- **Ringing Signal (~90V AC @ 20Hz):** Used to activate electromechanical bells
- **Lamp Circuits (6–12V AC or DC):** Powered dial illumination and panel indicators
- **Logic Circuits (varied):** Central office and PBX logic voltages often used -24V or +5V DC depending on the technology

As systems evolved, switchmode regulators, SLICs, and integrated isolation became common in embedded and VoIP equipment. However, traditional analog phones still require these distinct voltages to function authentically.

Power Domains
-------------

This project replicates the classic telecom supply scheme using three distinct voltage sources:

1. **-48V DC** – Simulated Central Office (CO) Subscriber Loop
2. **90V AC (or pulsed DC)** – Mechanical Ringer Power
3. **9V AC** – Rotary Dial Illumination
4. **3.3V / 5V DC** – Control Logic and Audio

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

3.3V / 5V DC – Control and Audio
--------------------------------

Used to power digital controllers, audio DACs, sensors, relays, and other low-voltage peripherals.

**Design Options:**

- A **USB power supply** combined with an LDO regulator is often the simplest way to deliver 3.3V. However, current is limited and this may not be suitable for peripherals with high demand.

- A **switchmode buck converter** can step down 12V or 24V to 3.3V or 5V with excellent efficiency. This is ideal for larger systems with audio amplifiers, displays, or relays.

- **Pre-built PSU modules** with dedicated outputs (e.g., 3.3V 1A) are compact and offer clean power, but may increase BOM cost.

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
- **90V AC (or pulsed DC)**: Activates mechanical ringers in the authentic cadence
- **9V AC**: Powers vintage dial lamps through the outer RJ11 pair
- **3.3V / 5V DC**: Powers modern control logic and audio components

Proper isolation, protection, and layout are essential for safe and functional operation across these mixed-voltage domains.
