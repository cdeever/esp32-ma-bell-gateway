The Bell System Blueprint
=========================

Before the rise of digital switching and VoIP, the **Bell System** established a rigorous engineering model for delivering reliable analog phone service across millions of subscriber lines. At the heart of this model was the **subscriber line interface**—the circuit that connected each individual phone to the central office (CO).

To standardize the functions of these circuits, Bell Labs engineers coined the acronym **BORSCHT**, which neatly summarizes the core services every line interface must provide. This framework guided the design of **line cards** like the Western Electric 400E and its descendants—modular units that delivered service to one subscriber each and operated faithfully for decades inside CO switching racks.

What Is a Subscriber Line?
--------------------------

A subscriber line refers to the physical two-wire connection between a customer's telephone and the central office. This pair—known as **tip and ring**—carries everything: DC power, ringing signals, supervision, and full-duplex audio. It's a deceptively simple interface with astonishing versatility.

To manage all this on just two wires, the Bell System broke the problem into discrete responsibilities—each handled by part of the line card circuit. That’s where **BORSCHT** comes in.

BORSCHT Explained
-----------------

.. list-table::
   :header-rows: 1
   :widths: 12 88

   * - **Letter**
     - **Function**
   * - **B** - Battery Feed  
     - Supplies loop current to power the telephone, typically –48 V DC.
   * - **O** - Overvoltage Protection  
     - Protects equipment from lightning strikes, surges, and accidental shorts.
   * - **R** - Ringing  
     - Applies an AC ringing voltage (~90 V RMS at 20 Hz) to alert the subscriber of incoming calls.
   * - **S** - Supervision  
     - Monitors line status—detecting off-hook, on-hook, pulse dialing, and ring trip events.
   * - **C** - Codec  
     - Converts analog voice signals into digital form (and back), typically via μ-law or A-law encoding.
   * - **H** - Hybrid  
     - Splits the 2-wire line into separate transmit and receive paths, enabling full-duplex audio.
   * - **T** - Test  
     - Supports remote testing of the subscriber line from the central office.

A Modern Tribute
----------------

This project—the **Ma Bell Gateway**—is a modern, microcontroller-based reimagining of the subscriber line card. But instead of switching a real call through copper trunks, it hands off audio to a Bluetooth headset. And instead of being mounted in a climate-controlled CO, it lives in your workshop or home lab.

Still, it honors the same **functional separation and design philosophy** that powered the public switched telephone network (PSTN) for over a hundred years.

Why It Matters
--------------

By organizing this project around the **Bell System’s BORSCHT model**, we’re not just recreating a piece of vintage hardware—we’re paying tribute to an engineering tradition that defined voice communication for over a century.

Every element of the Ma Bell Gateway, from loop supervision to ringing voltage, follows principles laid down by generations of telecom engineers who built systems to last decades, serve millions, and work flawlessly under all conditions.

This project embraces that legacy—not just for nostalgia’s sake, but to learn from its clarity, its modularity, and its remarkable longevity. In the spirit of those engineers, we carry the signal forward.
