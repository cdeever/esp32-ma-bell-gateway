========================
Off-Hook Detection
========================

An **off-hook detection circuit** is required to sense when a telephone receiver is lifted—completing the subscriber loop between tip and ring—so that the system can recognize an attempt to place or answer a call.

The challenge is that this loop typically carries **high voltages (up to 48V DC idle, 90V AC ringing)**, while modern microcontrollers like the **ESP32 operate at just 3.3V and cannot safely interface with the line directly**. Therefore, any detection method must safely **isolate the ESP32** from the phone line while accurately identifying the off-hook condition.

Several design approaches were considered, including the possibility of applying **authentic historical circuit styles** used by central offices (COs), alongside modern alternatives that offer improved safety and integration.

----

Historical Framing
==================

.. list-table::
   :header-rows: 1
   :class: scrollable-table
   :widths: 25 75

   * - **Era**
     - **Detection Approach**
   * - 1900s–1940s
     - Mechanical relays sensed loop current and physically closed contacts to engage switching equipment.
   * - 1950s–1970s
     - Solid-state transistor circuits began replacing relays in central office and PBX gear.
   * - 1970s–1990s
     - Optocouplers introduced safe isolation between analog lines and digital switching logic.
   * - 2000s+
     - Solid-state relays (SSRs) and line interface ICs became common in VoIP and embedded systems.

----

Behavior & Performance
======================

.. list-table::
   :header-rows: 1
   :widths: 22 19 19 19 19

   * - **Feature**
     - **Mechanical Relay**
     - **Resistor + Transistor**
     - **Optocoupler**
     - **SSR**
   * - **Sound / Feel**
     - ✅ Clunky click
     - ❌ Silent
     - ❌ Silent
     - ❌ Silent
   * - **Speed**
     - ❌ Slower
     - ✅ Fast
     - ✅ Fast
     - ✅ Fast
   * - **Wear**
     - ❌ Wears out
     - ✅ No wear
     - ✅ No wear
     - ✅ No wear
   * - **Reliability**
     - ❌ Contact wear
     - ✅ Stable
     - ✅ Very stable
     - ✅ Some leakage

----

Safety, Power & Integration
===========================

All methods connect to an ESP32 using basic GPIO—no special peripherals needed.

.. list-table::
   :header-rows: 1
   :widths: 22 19 19 19 19

   * - **Feature**
     - **Mechanical Relay**
     - **Resistor + Transistor**
     - **Optocoupler**
     - **SSR**
   * - **Voltage Isolation**
     - ✅ Isolated
     - ❌ Direct risk
     - ✅ Isolated
     - ✅ Isolated
   * - **Current Draw**
     - 20–60mA
     - 20–60mA
     - 20–60mA
     - Very low
   * - **Power Needs**
     - ❌ Extra power
     - ✅ None
     - ✅ None
     - ✅ None

----

Complexity, Cost & Final Score
==============================

.. list-table::
   :header-rows: 1
   :widths: 22 19 19 19 19

   * - **Feature**
     - **Mechanical Relay**
     - **Resistor + Transistor**
     - **Optocoupler**
     - **SSR**
   * - **Simplicity**
     - ❌ Needs driver
     - ❌ Needs protection
     - ✅ Simple
     - ✅ PnP
   * - **Complexity**
     - ❌ Moderate
     - ✅ Low
     - ✅ Low
     - ✅ Low
   * - **Cost**
     - ❌ Moderate
     - ✅ Cheapest
     - ✅ Affordable
     - ❌ Higher
   * - **Pitfalls**
     - Debounce needed
     - Needs Zener
     - Vf matching
     - Leakage
   * - **🔥 Final Score**
     - **34**
     - **27**
     - **45**
     - **42**

----

Best Choice Based on Score
==========================

.. list-table::
   :header-rows: 1
   :widths: 50 20

   * - **Detection Method**
     - **Final Score (Max 50)**
   * - ✅ **Optocoupler (Modern Isolation)** → *Best Overall*
     - **45**
   * - ✅ **SSR (No Click)** → *PnP Option*
     - **42**
   * - ✅ **Mechanical Relay (Clicky CO Feel)**
     - **34**
   * - ✅ **Resistor + Transistor (Vintage Style)**
     - **27**

----

Final Thoughts
==============

- ✅ **Optocoupler is the best all-around option** – safe, reliable, and simple to integrate.
- ✅ **SSR is beginner-friendly and silent**, offering a plug-and-play solution at slightly higher cost.
- ✅ **Mechanical relay adds authentic charm**, but with mechanical wear over time.
- ⚠️ **Resistor + transistor is the least safe**, and while it’s historically relevant, it places the ESP32 at risk without careful protection.

In this project, the **optocoupler-based circuit** will be used for off-hook detection. It provides galvanic isolation from the phone line and allows the ESP32 to monitor the line safely. The implementation will require:

  - A **current-limiting resistor** on the optocoupler's input side  
  - Careful matching of the **forward voltage (Vf)** and current for reliable triggering  
  - A **pull-up resistor** or logic input to the ESP32 on the output side  
  - Optional use of a **Schmitt trigger or GPIO interrupt** for clean digital transitions

This approach reflects how later-generation COs managed line isolation while providing a practical and safe solution for modern embedded design.
