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
     - For 3.3V/9V/-48V supply input

   * - C1, C2, ...
     - Capacitors (see schematic)
     - [values as needed]
     - [Mfr/P/N]
     - Filter, bypass, and timing

   * - R1, R2, ...
     - Resistors (see schematic)
     - [values as needed]
     - [Mfr/P/N]
     - Feed, balance, pull-up/down, etc.

   * - Q1
     - Relay or MOSFET
     - --
     - [Part #]
     - Ring relay driver or lamp control

   * - D1, D2, ...
     - Diodes/Bridge Rectifier
     - --
     - [Part #]
     - Protection (bridge for line input, etc.)

   * - T1
     - Transformer (if used for ringing)
     - 20Hz, [spec]
     - [Part #]
     - Ring generator

   * - Misc.
     - PCB, hardware, headers, jumpers
     - --
     - --
     - Enclosure, standoffs, screws, etc.

