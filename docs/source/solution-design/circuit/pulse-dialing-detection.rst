Pulse Dialing Detection
=======================

In a rotary phone system, the same signal used for off-hook detection can also be used to decode dialed digits. Rotary dialing works by rapidly toggling the phone's hook switch—momentarily breaking the loop current—to send a series of timed pulses representing each digit.

For example:
- Dialing “5” results in five brief on-hook pulses
- Dialing “0” results in ten pulses
- Each pulse is typically 60–70ms OFF followed by 40ms ON

.. figure:: /_images/dial_5_pulse_digits.png
   :alt: Dial pulse pattern for digit 5 with timing annotations
   :width: 600px
   :align: center

   **Pulse Dialing Timing – Digit 5:** Each pulse briefly opens the loop (on-hook) for ~60ms, then closes it again (off-hook) for ~40ms before the next pulse. This process repeats the number of times corresponding to the digit dialed.


To support pulse dialing, the off-hook detection circuit must do more than just detect steady-state off-hook. It must also observe these rapid transitions and count them accurately.

Implementation Approaches
--------------------------

Several strategies can be used to detect dial pulses using the same off-hook detection hardware.

Software-Based Pulse Decoding
------------------------------

This is the simplest and most flexible method. The off-hook signal is connected to a GPIO pin and monitored in software.

- Configure the pin as an interrupt (or poll it frequently)
- Watch for falling edges (off-hook → on-hook)
- Measure the time between transitions
- Count pulses within a timeout window (~800ms of silence means digit finished)
- Apply debouncing (e.g. ignore pulses <20ms)

**Advantages:**
- No hardware changes required
- Easy to tweak behavior in firmware
- Works with any detection method (optocoupler, relay, etc.)

**Disadvantages:**
- Requires accurate software timing
- Susceptible to noise or bounce without filtering

Unfiltered Edge Detection Path
------------------------------

If your off-hook detection circuit includes significant low-pass filtering (e.g., RC filters to avoid false triggers), you may lose the ability to see fast transitions.

To solve this, you can split the signal path:
- **One filtered signal** for basic off-hook detection
- **One unfiltered signal** connected to a fast Schmitt trigger, comparator, or direct GPIO for pulse detection

This preserves pulse edges while still allowing stable off-hook state monitoring.

**Advantages:**
- Preserves clean logic level edges
- Reduces false detections while maintaining responsiveness

**Disadvantages:**
- Slightly more complex hardware
- Requires signal buffering or branching

Dedicated Pulse Detection Hardware (Optional)
---------------------------------------------

For high-reliability or analog-style builds, you may use classic circuits to convert hook pulses into digital edges:

- 555-based monostable circuits
- Differentiator + comparator
- Commercial dial pulse decoder ICs (rare)

This is typically overkill for a microcontroller-based project, but may be considered for advanced or nostalgic designs.

Recommended Approach
--------------------

For this project, pulse detection will be implemented in **software using the optocoupler output signal**. The ESP32 will:

- Count transitions during off-hook state
- Parse digits from pulse counts
- Feed the resulting number into the Bluetooth dialer system

This dual-use design leverages the existing detection circuit, avoids redundant hardware, and aligns well with the project’s embedded software architecture.
