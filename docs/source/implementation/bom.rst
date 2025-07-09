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
     - Audio Codec (if used)
     - (e.g. ES8311)
     - [Your preferred part]
     - PCM/I2S interface between ESP32 & SLIC

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

