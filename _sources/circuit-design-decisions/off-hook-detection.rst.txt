=========================================================
Scored Off-Hook Detection Circuit Design Decision Matrix
=========================================================

This table ranks different off-hook detection circuit design approaches based on risk to ESP32 (safety & isolation), reliability, simplicity (novice-friendly), cost, and other relevant features. The approaches are ordered from oldest to newest technology mirroring the progression the CO would have used over the decades.  

While Solid-State relays were not used by the COs for off-hook detection, this approach was considered anyway for this project.

.. list-table::
   :header-rows: 1
   :widths: auto

   * - **Feature**
     - **Standalone Mechanical Relay (Clicky, Old-School CO)**
     - **Resistor + Transistor (Vintage Solid-State)**
     - **Optocoupler (Modern Isolation)**
     - **Solid-State Relay (SSR, No Click)**
   * - **Click Sound (CO Feel)**
     - ✅ Yes! Satisfying relay "clunk"
     - ❌ No sound
     - ❌ No sound
     - ❌ No sound
   * - **Response Time**
     - ❌ Slower (~5-20ms delay)
     - ✅ Fast (<1ms)
     - ✅ Fast (<1ms)
     - ✅ Fast (<1ms)
   * - **Wear Over Time**
     - ❌ Mechanical wear over time
     - ✅ No wear, stable
     - ✅ No wear, stable
     - ✅ No wear, stable
   * - **Voltage Isolation (Safety to ESP32) (3x weight)**
     - ✅ 5 (Fully isolated)
     - ❌ 1 (Direct connection risk)
     - ✅ 5 (Fully isolated)
     - ✅ 5 (Fully isolated)
   * - **Reliability (No Wear & Tear, No False Triggers) (2x weight)**
     - ❌ 2 (Relay contacts degrade)
     - ✅ 4 (Transistor-based reliability)
     - ✅ 5 (No moving parts, very reliable)
     - ✅ 4 (No moving parts, some SSR leakage current)
   * - **Simplicity & Novice-Friendly (3x weight)**
     - ❌ 3 (Requires relay driver)
     - ❌ 2 (Needs voltage protection)
     - ✅ 4 (Simple circuit)
     - ✅ 5 (Easiest, plug-and-play)
   * - **Complexity (Higher = Harder to Build)**
     - ❌ Moderate (Relay requires driver circuit)
     - ✅ Low (Basic transistor circuit)
     - ✅ Low (Basic optocoupler circuit)
     - ✅ Low (Easy module integration)
   * - **Cost (Lower is Better) (2x weight)**
     - ❌ 3 (Moderate cost)
     - ✅ 5 (Cheapest method)
     - ✅ 4 (Affordable)
     - ❌ 2 (More expensive)
   * - **Current Draw (Phone Line Load)**
     - 20-60mA (Relay coil must match this)
     - 20-60mA (Small drop across resistor)
     - 20-60mA (Drop across LED inside optocoupler)
     - Very low (some leakage in SSRs)
   * - **ESP32 Integration**
     - ✅ Simple digital GPIO
     - ✅ Simple digital GPIO
     - ✅ Simple digital GPIO
     - ✅ Simple digital GPIO
   * - **Power Needs**
     - ❌ Relay coil consumes extra power
     - ✅ No extra power needed
     - ✅ No extra power needed
     - ✅ No extra power needed
   * - **Potential Pitfalls**
     - ❌ Relay bounces, needs debounce delay
     - ❌ Needs ESP32 overvoltage protection (Zener diode)
     - ❌ Optocoupler LED must be matched for voltage drop
     - ❌ Some SSRs have leakage current (may falsely trigger)
   * - **🔥 Final Score (Weighted Calculation)**
     - **34**
     - **27**
     - **45**
     - **42**

---

Best Choice Based on Score
==========================

.. list-table::
   :header-rows: 1
   :widths: auto

   * - **Best Off-Hook Detection Method Based on Score**
     - **Final Score (Max: 50)**
   * - ✅ **Optocoupler (Modern Isolation) → Best Overall**
     - **45/50**
   * - ✅ **Solid-State Relay (SSR, No Click) → Simple & Plug-and-Play**
     - **42/50**
   * - ✅ **Mechanical Relay (Clicky, Old-School CO) → Classic Bell System Feel**
     - **34/50**
   * - ✅ **Resistor + Transistor (Vintage Solid-State) → Historically Used but Riskier**
     - **27/50**

Final Thoughts
==============

- **Optocoupler is the best choice overall** → Safe, reliable, and simple to implement.
- **SSR is also an excellent option** → Easiest for beginners but slightly more expensive.
- **Mechanical relays provide historical accuracy and the "click" sound**, but they wear out over time.
- **Resistor + Transistor is cheap and historically relevant but exposes the ESP32 to risk.

---
