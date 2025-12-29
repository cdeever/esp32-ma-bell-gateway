Bill of Materials
=================

This section lists all components required to assemble the Ma Bell Gateway.  
Values, part numbers, and notes are included for clarity.

.. list-table::
   :header-rows: 1
   :widths: 12 20 12 24 32

   * - Ref. Des.
     - Description
     - Value / Rating
     - Manufacturer / Part #
     - Notes

   * - U1
     - ESP32-WROVER-IE Module
     - --
     - Espressif ESP32-WROVER-IE
     - Main MCU

   * - U2
     - SLIC, Line Interface IC
     - HC-5504B (DIP-24)
     - Intersil / Renesas HC-5504B-5
     - Subscriber Line Interface Circuit

   * - U3
     - DAC (Digital-to-Analog Converter)
     - PCM5100PWR
     - Texas Instruments
     - TSSOP-20, I2S audio output

   * - U4
     - ADC (Analog-to-Digital Converter)
     - PCM1808PWR
     - Texas Instruments
     - TSSOP-20, I2S audio input

   * - C_AUDIO1, C_AUDIO2
     - AC Coupling Capacitors (Audio)
     - 10µF, 16V+
     - Generic / Film or X7R MLCC
     - Non-polarized, between codec and SLIC

   * - C_DAC1
     - Decoupling Capacitor (DAC)
     - 0.1µF ceramic
     - Generic / X7R
     - Near PCM5100 VDD pin

   * - C_DAC2
     - Bulk Capacitor (DAC)
     - 10µF tantalum/MLCC
     - Generic / Low ESR
     - Near PCM5100 VDD pin

   * - C_ADC1
     - Decoupling Capacitor (ADC)
     - 0.1µF ceramic
     - Generic / X7R
     - Near PCM1808 VDD pin

   * - C_ADC2
     - Bulk Capacitor (ADC)
     - 10µF tantalum/MLCC
     - Generic / Low ESR
     - Near PCM1808 VDD pin

   * - R_XSMT
     - Pull-up Resistor (DAC Mute)
     - 10kΩ, 1%
     - Generic / Metal film
     - PCM5100 XSMT pin to 3.3V

   * - R_BIAS
     - Input Bias Resistor (ADC)
     - 10kΩ, 1%
     - Generic / Metal film
     - PCM1808 VINL to AGND

   * - J1
     - RJ11 Jack
     - 6P4C or 6P2C
     - Amphenol or equivalent
     - Phone line output

   * - J2
     - Power Connector
     - Barrel Jack, 5.5x2.1mm
     - CUI PJ-102A or equivalent
     - For 48V DC supply input (60W recommended)

   * - U5
     - Ring Generator IC
     - LT1684
     - Analog Devices LT1684CN (DIP-16)
     - Telephone ring tone generator

   * - K1
     - Ring Relay
     - DPDT, 12V coil
     - Omron G5V-2-H1-DC12
     - Switches line between -48V and ring generator

   * - Q1
     - Relay Driver Transistor
     - 2N2222A
     - Generic / ON Semi PN2222A
     - Drives ring relay coil from SLIC RD pin

   * - D1
     - Flyback Diode
     - 1N4148
     - Generic
     - Across relay coil

   * - R_LED1, R_LED2
     - LED Current Limiting Resistors
     - 330Ω, 1/4W
     - Generic / Metal film
     - For 3.3V status LEDs (GPIO 2, 18)

   * - LED1, LED2
     - Status LEDs
     - 3mm or 5mm
     - Generic (any color)
     - Power/status indicators

   * - R_BASE
     - Relay Driver Base Resistor
     - 1kΩ, 1/4W
     - Generic / Metal film
     - Q1 base current limiting

   * - C_DEC1-4
     - IC Decoupling Capacitors
     - 0.1µF ceramic
     - Generic / X7R
     - Near each IC VDD pin

   * - PS1
     - 48V to 12V Buck Converter
     - 48V→12V, 1.5A
     - DROK (Amazon) or LM2596HV module
     - SLIC VB+ power

   * - PS2
     - 12V to 3.3V Buck Converter
     - 12V→3.3V, 1A
     - LM2596 module (3.3V fixed output)
     - ESP32 and codec power

   * - PS3
     - -48V DC-DC Converter
     - 48V→-48V isolated
     - TBD (see power-supply.rst)
     - SLIC VB- loop voltage (deferred)

   * - Misc.
     - PCB, hardware, headers, jumpers
     - --
     - --
     - Enclosure, standoffs, screws, etc.

