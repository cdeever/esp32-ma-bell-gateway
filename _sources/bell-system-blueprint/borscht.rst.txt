The BORSCHT Model
=================

What Is a Subscriber Line?
--------------------------

A subscriber line refers to the physical two-wire connection between a customer's telephone and the central office. This pair—known as **tip and ring**—carries everything: DC power, ringing signals, supervision, and full-duplex audio. It's a deceptively simple interface with astonishing versatility.

To manage all this on just two wires, the Bell System broke the problem into discrete responsibilities—each handled by part of the line card circuit. That's where **BORSCHT** comes in.

The Seven Functions
-------------------

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
---------------------------

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
-----------------------

By organizing this model around **BORSCHT**, Bell System engineers distilled decades of evolving requirements into a modular, scalable standard. Whether servicing a rural home or a city high-rise, the network could be expanded one line card at a time, each implementing the exact same set of functions.

That consistency, adaptability, and deep engineering discipline made the subscriber line interface one of the most quietly brilliant systems in 20th-century electronics.
