Power Supplies
==============

The Ma Bell Gateway requires multiple voltage sources to support both legacy analog telephone hardware and modern embedded electronics. These supplies must be carefully chosen and isolated where necessary to provide the right voltages for ringer operation, illuminated dial lighting, logic control, and audio processing.

.. note::
   **Safety First:** Voltages above 48V can be hazardous. All high-voltage domains should be handled with care, properly isolated, and clearly labeled. This design is intended for experienced builders familiar with electrical safety.

Power Domains
-------------

This project uses three distinct power domains:

1. **48V DC** – Simulated Central Office (CO) Line
2. **9V AC** – Dial Illumination
3. **3.3V / 5V DC** – ESP32 and Digital Peripherals

Each serves a specific role in replicating the traditional analog phone experience.

Power Sizing
------------

When selecting power supplies, ensure each domain can provide enough current for its worst-case load. Typical requirements are:

- **Mechanical ringer:** up to 40 mA (pulsed)
- **Dial illumination:** 10–50 mA (depending on lamp type)
- **ESP32 and peripherals:** 200–500 mA (including WiFi transients, relays, displays, or audio amps)

It is best practice to allow at least 25% overhead above calculated peak loads for safety and future expansion.

48V DC – Line Voltage and Ringing
---------------------------------

The 48V DC supply simulates the nominal voltage found on Bell System phone lines when idle. It is applied across the red and green pair of a standard RJ11 connection and serves as the baseline "on-hook" voltage.

This supply is also used to drive the mechanical ringer. Although traditional ringing voltages were closer to 90V AC, many ringer coils will activate with 48V DC if it is pulsed in a repeating cadence (e.g., 2 seconds on, 4 seconds off). For maximum authenticity, the design could include a step-up converter or waveform generator to provide true 90V AC ringing, but the pulsed 48V DC approach is a practical and safer compromise for most projects.

The ESP32 will control this cadence via a relay or solid-state switch, with appropriate electrical isolation. **Care must be taken to isolate this high-voltage DC from the ESP32 logic**, typically through opto-isolators or relays.

9V AC – Dial Illumination
-------------------------

The 9V AC supply powers the lighted rotary dial. Many classic telephones used neon or incandescent lamps wired to the outer black and yellow pair of the RJ11 connector. These lights expect a low-voltage AC source, often between 6V and 12V.

This supply is kept entirely separate from the 48V DC line and is used only for the dial light. It may be left always on or optionally switched on/off in sync with off-hook detection.

If switched, this will be done using an electrically isolated relay or triac to ensure that the AC circuit is not directly exposed to the microcontroller.

3.3V / 5V DC – ESP32 and Peripherals
------------------------------------

The ESP32 requires 3.3V DC to operate, typically provided by a regulator built into its development board or module. If using a bare ESP32-WROVER-IE, an external 3.3V LDO or switching regulator will be required.

Peripherals such as audio DACs, op-amps, or other digital logic components may require either 3.3V or 5V depending on their interface. Care must be taken to ensure level-shifting is applied if mixing logic levels.

The ESP32's onboard regulators can provide limited current on GPIO pins, but any peripherals drawing significant current—such as relays, audio amps, or OLED displays—should receive power directly from the 3.3V or 5V bus, not through GPIO.

**Each power supply should have an isolated ground where possible; high-voltage and low-voltage (logic) grounds should not be directly connected.**

Fuse and Surge Protection
-------------------------

For safety, include a fast-blow fuse or PTC resettable fuse on each high-voltage supply (48V DC and 9V AC). TVS (transient voltage suppression) diodes on input lines are recommended to guard against surges and accidental miswiring. This adds another layer of protection for both equipment and user.

Power Sequencing and Fault Tolerance
------------------------------------

For this design, no special sequencing of power domains is required, but ensure the ESP32 is not exposed to high voltages during power-on or while wiring. Loss of the 9V AC supply will only disable dial illumination; logic and ringer functions will continue. All power circuits should be designed to fail safe.

Mechanical and Environmental Considerations
-------------------------------------------

All power supplies and high-voltage circuitry should be housed in a ventilated enclosure, with adequate spacing and creepage/clearance between high- and low-voltage domains. Follow best practices for wiring, mounting, and labeling to support safety and maintenance.

Power Routing and Isolation
---------------------------

To protect the microcontroller and low-voltage circuitry, all high-voltage domains (48V DC and 9V AC) are switched using isolation methods:

- **Relays** (mechanical or solid-state) for high current or inductive loads
- **Opto-isolators** for low-current signal switching
- **Snubber networks** or flyback diodes where appropriate
- **Fuses** and **TVS diodes** as discussed above

Each supply should be clearly separated on the PCB and wiring harness, with appropriate labeling and spacing to meet safety guidelines.

.. note::
   Careful layout and physical separation of power domains will be addressed in the Hardware Implementation section.


Summary
-------

The use of multiple, purpose-specific power supplies allows the Ma Bell Gateway to faithfully recreate the behavior of traditional phone equipment while protecting modern digital components. The 48V DC simulates the CO line and drives the ringer (via pulsed DC for safety), 9V AC lights the rotary dial, and 3.3V or 5V DC powers the ESP32 and its peripherals. Proper isolation, surge protection, and routing are essential to prevent high-voltage exposure to sensitive digital circuitry.
