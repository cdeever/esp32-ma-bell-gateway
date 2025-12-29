The Bell System Blueprint
=========================

Before the rise of digital switching and VoIP, the **Bell System** established a rigorous engineering model for delivering reliable analog phone service across millions of subscriber lines. At the heart of this model was the **subscriber line interface**—the circuit that connected each individual phone to the central office (CO).

To standardize the functions of these circuits, Bell Labs engineers coined the acronym **BORSCHT**, which neatly summarizes the core services every line interface must provide. This framework guided the design of **line cards** like the Western Electric 400E and its descendants—modular units that delivered service to one subscriber each and operated faithfully for decades inside CO switching racks.

The BORSCHT Model
-----------------

What Is a Subscriber Line?
^^^^^^^^^^^^^^^^^^^^^^^^^^

A subscriber line refers to the physical two-wire connection between a customer's telephone and the central office. This pair—known as **tip and ring**—carries everything: DC power, ringing signals, supervision, and full-duplex audio. It's a deceptively simple interface with astonishing versatility.

To manage all this on just two wires, the Bell System broke the problem into discrete responsibilities—each handled by part of the line card circuit. That's where **BORSCHT** comes in.

The Seven Functions
^^^^^^^^^^^^^^^^^^^

The BORSCHT model defines the seven core functions every subscriber line interface must provide. Each responsibility is essential for delivering power, signaling, audio, and diagnostic capabilities over a simple two-wire phone line.


.. list-table::
   :header-rows: 1
   :widths: 12 88

   * - **Letter**
     - **Function**
   * - **B** - Battery Feed
     - Supplies loop current to power the telephone, typically –48 V DC.
   * - **O** - Overvoltage Protection
     - Protects equipment from lightning strikes, surges, and accidental shorts.
   * - **R** - Ringing
     - Applies an AC ringing voltage (~90 V RMS at 20 Hz) to alert the subscriber of incoming calls.
   * - **S** - Supervision
     - Monitors line status—detecting off-hook, on-hook, pulse dialing, and ring trip events.
   * - **C** - Codec
     - Converts analog voice signals into digital form (and back), typically via μ-law or A-law encoding.
   * - **H** - Hybrid
     - Splits the 2-wire line into separate transmit and receive paths, enabling full-duplex audio.
   * - **T** - Test
     - Supports remote testing of the subscriber line from the central office.

Design Details with Purpose
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Each element of BORSCHT wasn't arbitrary—it was engineered with care to balance **functionality, safety, serviceability, and longevity**. Here are just a few of the clever design decisions behind the acronym:

- **–48 V DC (Battery Feed)**
  This negative voltage helps prevent electrolytic corrosion at wiring joints. With the central office (CO) negative relative to earth ground, stray currents flow into the ground instead of pulling oxygen into copper connections—prolonging cable life across thousands of miles.

- **Gas Tube Arrestors and Carbon Blocks (Overvoltage Protection)**
  Subscriber loops often span outdoor lines, prone to lightning strikes. Telcos added surge protection at both ends—sometimes with audible "pops" when a strike occurred—preserving line cards and phones alike.

- **90 V RMS, 20 Hz Ringing (Ringing)**
  This frequency and voltage combo was optimized to mechanically resonate electromechanical bell clappers while avoiding interference with voice frequencies or excessive current draw.

- **Loop Current Detection (Supervision)**
  A simple current sensor could detect if a handset was lifted, if dialing was in progress (via pulses), or if a call had been answered—all without extra wiring.

- **μ-law and A-law Companding (Codec)**
  These logarithmic encoding schemes compressed dynamic range efficiently for long-distance trunk lines, ensuring quiet voices weren't drowned out and loud signals didn't overload.

- **Transformer Hybrids (Hybrid)**
  Audio transformers or active circuits cleverly separated transmit and receive signals on the same pair of wires—enabling full-duplex conversations on just two conductors.

- **Central Office Line Testing (Test)**
  Technicians could remotely inject tones, measure resistance, or verify loop status without ever visiting the subscriber's home—a remarkable feat in the pre-digital era.

What BORSCHT Represents
^^^^^^^^^^^^^^^^^^^^^^^

By organizing this model around **BORSCHT**, Bell System engineers distilled decades of evolving requirements into a modular, scalable standard. Whether servicing a rural home or a city high-rise, the network could be expanded one line card at a time, each implementing the exact same set of functions.

That consistency, adaptability, and deep engineering discipline made the subscriber line interface one of the most quietly brilliant systems in 20th-century electronics.

Voice and Tone Audio
--------------------

The analog telephone interface must support **bidirectional voice transmission** and **in-band signaling tones**, all delivered across just two wires: **tip and ring**. These audio paths must replicate the expectations of legacy Bell System telephony while remaining compatible with modern embedded systems and Bluetooth audio routing.

Target Parameters
^^^^^^^^^^^^^^^^^

Historically, analog telephone audio adheres to the following standards:

- **Impedance:** 600 Ω balanced (nominal)
- **Voice Bandwidth:** 300 Hz to 3400 Hz (standard "voice band")
- **Signal Level:**
  - Transmit (talk-out): ~0 dBm nominal
  - Receive (talk-in): –10 to –20 dBm typical
- **Isolation:** Galvanic isolation from line circuitry is essential
- **Tones:** In-band tones (dial tone, ringback, busy, etc.) occupy same audio path as speech

.. note::
   **What is Galvanic Isolation?**
   Galvanic isolation means there is **no direct electrical connection** between two circuits—only signal transfer via magnetic, optical, or capacitive coupling. This prevents high voltages or ground loops from damaging sensitive electronics or endangering users.
   Common methods include transformers and optocouplers.

Audio Functions
^^^^^^^^^^^^^^^

Two independent analog paths must be supported:

- **Earpiece / Receive Path (to headset):**
  - Delivers voice and tones from the system into the phone's earpiece
- **Microphone / Transmit Path (from handset):**
  - Accepts voice from the microphone and routes it into the system for digitization or Bluetooth transmission

Both paths share the same tip-ring pair, requiring either a **hybrid** circuit (2-wire to 4-wire conversion) or **time-domain multiplexing** to avoid feedback or echo.

Historical Evolution
^^^^^^^^^^^^^^^^^^^^

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

Power Domains
-------------

The Bell System established standard voltages for line and subscriber equipment by the early 20th century:

- **Subscriber Loop (-48V DC):** Established for consistent talk battery voltage and to minimize corrosion at outdoor wiring connections
- **Ringing Signal (~90V AC @ 20Hz):** Used to activate electromechanical bells
- **Lamp Circuits (6–12V AC or DC):** Powered dial illumination and panel indicators
- **Logic Circuits (varied):** Central office and PBX logic voltages often used -24V or +5V DC depending on the technology

As systems evolved, switchmode regulators, SLICs, and integrated isolation became common in embedded and VoIP equipment. However, traditional analog phones still require these distinct voltages to function authentically.
