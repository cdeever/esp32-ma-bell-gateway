Voice Transmission
==================

The analog telephone interface must support **bidirectional voice transmission** across just two wires: **tip and ring**. Achieving clear, full-duplex audio over a single pair of conductors was one of the Bell System's most elegant engineering accomplishments.

Target Parameters
-----------------

Historically, analog telephone voice circuits adhered to the following standards:

- **Impedance:** 600 Ω balanced (nominal)
- **Voice Bandwidth:** 300 Hz to 3400 Hz (standard "voice band")
- **Signal Level:**
  - Transmit (talk-out): ~0 dBm nominal
  - Receive (talk-in): –10 to –20 dBm typical
- **Isolation:** Galvanic isolation from line circuitry is essential

.. note::
   **What is Galvanic Isolation?**
   Galvanic isolation means there is **no direct electrical connection** between two circuits—only signal transfer via magnetic, optical, or capacitive coupling. This prevents high voltages or ground loops from damaging sensitive electronics or endangering users.
   Common methods include transformers and optocouplers.

Audio Paths
-----------

Two independent analog paths must be supported:

- **Earpiece / Receive Path (to headset):**
  - Delivers voice from the system into the phone's earpiece
- **Microphone / Transmit Path (from handset):**
  - Accepts voice from the microphone and routes it into the system for transmission

Both paths share the same tip-ring pair, requiring a **hybrid** circuit (2-wire to 4-wire conversion) to separate transmit and receive signals and avoid feedback or echo.

Historical Evolution
--------------------

The Bell System employed a variety of techniques over the decades to achieve clear, isolated, full-duplex audio across a two-wire line:

Carbon Microphone & Induction Coil (1900s–1940s)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Carbon microphones modulated DC current flowing through granular carbon
- An **induction coil** (hybrid transformer) separated microphone and speaker paths
- Very robust and loud, but audio fidelity was limited
- This design remained in use for decades due to its simplicity and reliability

Hybrid Transformers (1950s–1980s)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Voice signals traveled on a 2-wire loop to the subscriber
- **Hybrids** converted between 2-wire (subscriber loop) and 4-wire (switching equipment)
- Careful impedance matching minimized echo and maximized signal separation
- Transformers provided galvanic isolation between subscriber and central office

Integrated Line Interface Circuits (1990s–Present)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- **SLICs** (Subscriber Line Interface Circuits) include balanced drivers, hybrids, and filters internally
- Audio is often digitized near the line, with signal paths managed digitally
- Modern SLICs provide built-in gain control, echo cancellation, and impedance matching
- VoIP equipment uses DSP-based echo cancellation and codec conversion

The 600 Ohm Standard
--------------------

The 600 Ω impedance standard became universal in North American telephony. This impedance:

- Minimized reflections and echo on long subscriber loops
- Allowed standardized equipment to interconnect reliably
- Enabled predictable signal levels across the network

While modern digital systems no longer require 600 Ω matching, vintage telephones and SLIC-based designs still expect this impedance for proper operation.
