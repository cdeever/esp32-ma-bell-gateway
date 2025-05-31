===========================
Telephone Service Standards
===========================

This section defines the electrical and signaling characteristics of analog telephone service in the United States and Canada, as used from the mid-20th century through the digital transition period. These specifications are rooted in standards developed by the Bell System (AT&T), later formalized by **Bellcore** (now Telcordia), and align with international guidelines from **ITU-T** (e.g., Q.23, Q.24, G.712). The parameters listed below are suitable for projects seeking to recreate the experience of analog telephony as deployed in the **North American Numbering Plan (NANP)** region.

Understanding these standards is critical to the Ma Bell Gateway project, which aims to authentically simulate a legacy landline telephone interface. By replicating the signaling voltages, tones, and user experience of historical telephone service, we can ensure compatibility with real vintage devices and preserve the unmistakable feel of classic analog telephony.

.. note::
   These specifications apply to **the United States and Canada** only. They **do not apply** to Central America, which uses other regional or ITU-based standards. Mexico, while part of NANP, uses some differing tone cadences and voltages and is not fully compatible.

The foundation of this system was developed in the 1940s–1960s under AT&T's Bell System and standardized through the 1970s–1990s by Bellcore and ITU-T. Most equipment and user experiences during this period conformed closely to the values below.

Voltage and Signaling
=====================

- **On-hook (idle)**: ~-48 VDC across tip and ring
- **Ringing**: ~70–90 V AC at 20 Hz superimposed on -48 VDC
- **Off-hook**: Drops to ~-10 VDC with loop current of 23–35 mA

Audio Specs
===========

- **Voice bandwidth**: 300 Hz – 3400 Hz
- **Nominal impedance**: 600 ohms
- **Transmit level**: ~0 dBm at CO; ~-8 to -10 dB received
- **Echo control**: Managed via hybrid circuits (in analog) or echo cancellers (digital)

Tone Signals
============

.. list-table::
   :header-rows: 1
   :widths: 20 20 60

   * - Tone
     - Frequency (Hz)
     - Description
   * - Dial Tone
     - 350 + 440
     - Continuous tone indicating readiness to dial
   * - Ringback
     - 440 + 480
     - 2 sec ON, 4 sec OFF
   * - Busy
     - 480 + 620
     - 0.5 sec ON, 0.5 sec OFF
   * - Reorder
     - 480 + 620
     - 0.25 sec ON, 0.25 sec OFF
   * - Off-hook Warning
     - 1400 + 2060 + 2450 + 2600
     - Repeated tone when receiver is left off-hook
   * - SIT (Info Tone)
     - 950 / 1400 / 1800
     - Special Information Tone before a recording

Dual-Tone Multi-Frequency (DTMF)
================================

Used for push-button dialing. Based on the Bell System's adoption of **Touch-Tone™** in the 1960s:

.. list-table::
   :header-rows: 1
   :widths: 10 10 10 10 10

   * -
     - 1209 Hz
     - 1336 Hz
     - 1477 Hz
     - 1633 Hz
   * - 697 Hz
     - 1
     - 2
     - 3
     - A
   * - 770 Hz
     - 4
     - 5
     - 6
     - B
   * - 852 Hz
     - 7
     - 8
     - 9
     - C
   * - 941 Hz
     - *
     - 0
     - #
     - D

Note: Most consumer telephones omitted the A–D column, which was used in military, radio trunking, and internal test systems.

Standard Recordings
===================

These recordings typically follow SIT tones and indicate a failed or misdialed call:

- "We're sorry, your call cannot be completed as dialed."
- "Please check the number and try again."
- "If you need assistance, hang up and dial the operator."

These recordings were standardized by the Bell System and later by regional carriers and independent telcos.
