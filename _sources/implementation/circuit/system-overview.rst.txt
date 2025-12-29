System Block Diagram
====================

This page provides a high-level overview of the Ma Bell Gateway hardware architecture, showing how all major subsystems interconnect.

Overall System Architecture
---------------------------

::

                                    ┌─────────────────────────────┐
                                    │       Mobile Phone          │
                                    │    (Bluetooth HFP 1.7)      │
                                    └──────────────┬──────────────┘
                                                   │
                                           Bluetooth Classic
                                              (Audio + Control)
                                                   │
    ┌──────────────────────────────────────────────┼──────────────────────────────────────────────┐
    │                                              │                                              │
    │                              MA BELL GATEWAY │                                              │
    │                                              │                                              │
    │  ┌────────────────┐                          │                                              │
    │  │  48V DC Power  │                          │                                              │
    │  │    Supply      │                          │                                              │
    │  │   (External)   │                          │                                              │
    │  └───────┬────────┘                          │                                              │
    │          │                                   │                                              │
    │          ▼                                   ▼                                              │
    │  ┌────────────────────────────────────────────────────────────────────┐                    │
    │  │                        Power Distribution                          │                    │
    │  │  ┌──────────┐  ┌──────────┐  ┌──────────────┐  ┌────────────────┐  │                    │
    │  │  │ 48V→12V  │  │ 48V→3.3V │  │  48V→-48V    │  │  Ring Generator│  │                    │
    │  │  │   Buck   │  │   Buck   │  │  Inverting   │  │   90V AC 20Hz  │  │                    │
    │  │  └────┬─────┘  └────┬─────┘  └──────┬───────┘  └───────┬────────┘  │                    │
    │  │       │             │               │                  │           │                    │
    │  │       │ 12V         │ 3.3V          │ -48V             │ 90VAC     │                    │
    │  └───────┼─────────────┼───────────────┼──────────────────┼───────────┘                    │
    │          │             │               │                  │                                │
    │          │             │               │                  │                                │
    │          ▼             ▼               ▼                  ▼                                │
    │  ┌───────────────────────────────────────────────────────────────────────────────────┐    │
    │  │                              HC-5504B SLIC                                         │    │
    │  │                     (Subscriber Line Interface Circuit)                            │    │
    │  │                                                                                    │    │
    │  │   • Battery Feed: -48V loop current to telephone                                   │    │
    │  │   • 2-to-4 Wire Hybrid: Separates TX/RX audio paths                               │    │
    │  │   • Off-Hook Detection: SHD pin → GPIO 32                                         │    │
    │  │   • Ring Control: GPIO 13 → RC pin (triggers 90VAC ring)                          │    │
    │  │   • Ring Status: RD pin → GPIO 33                                                 │    │
    │  │                                                                                    │    │
    │  │   Audio Out (RX pin 21) ◄──────────────┐    ┌──────────────► Audio In (TX pin 24) │    │
    │  └────────────────────────────────────────┼────┼─────────────────────────────────────┘    │
    │                                           │    │                                          │
    │                                           │    │ Analog Audio                             │
    │                                           │    │                                          │
    │  ┌────────────────────────────────────────┼────┼─────────────────────────────────────┐    │
    │  │                              Audio Codec                                          │    │
    │  │                                        │    │                                     │    │
    │  │   ┌─────────────────┐                  │    │              ┌─────────────────┐   │    │
    │  │   │    PCM5100      │◄─────────────────┘    └─────────────►│    PCM1808      │   │    │
    │  │   │      DAC        │                                      │      ADC        │   │    │
    │  │   │  (ESP32→Phone)  │                                      │  (Phone→ESP32)  │   │    │
    │  │   └────────┬────────┘                                      └────────┬────────┘   │    │
    │  │            │                                                        │            │    │
    │  │            │ I2S DOUT (GPIO 26)                    I2S DIN (GPIO 35)│            │    │
    │  │            │                                                        │            │    │
    │  │            │         Shared: BCLK (GPIO 5), LRCLK (GPIO 25)         │            │    │
    │  └────────────┼────────────────────────────────────────────────────────┼────────────┘    │
    │               │                                                        │                 │
    │               │                        I2S Bus                         │                 │
    │               │                                                        │                 │
    │               ▼                                                        ▼                 │
    │  ┌───────────────────────────────────────────────────────────────────────────────────┐   │
    │  │                             ESP32-WROVER-IE                                        │   │
    │  │                                                                                    │   │
    │  │   ┌─────────────────┐   ┌─────────────────┐   ┌─────────────────────────────────┐ │   │
    │  │   │   I2S Driver    │   │  Bluetooth HFP  │   │         GPIO Control            │ │   │
    │  │   │                 │   │   (Internal)    │   │                                 │ │   │
    │  │   │ • 8kHz/16-bit   │   │                 │   │ • Off-hook: GPIO 32 (input)     │ │   │
    │  │   │ • Mono          │   │ • HFP 1.7       │   │ • Ring cmd: GPIO 13 (output)    │ │   │
    │  │   │ • HCI audio path│   │ • Audio routing │   │ • Ring status: GPIO 33 (input)  │ │   │
    │  │   └─────────────────┘   │ • Call control  │   │ • Pulse dial: GPIO 34 (input)   │ │   │
    │  │                         └─────────────────┘   └─────────────────────────────────┘ │   │
    │  │                                                                                    │   │
    │  │   ┌─────────────────┐   ┌─────────────────┐   ┌─────────────────────────────────┐ │   │
    │  │   │   WiFi (STA)    │   │   Web Server    │   │     State Machine / App         │ │   │
    │  │   │  (Internal)     │   │   (HTTP API)    │   │  (Dial tone, busy, ringback)    │ │   │
    │  │   └─────────────────┘   └─────────────────┘   └─────────────────────────────────┘ │   │
    │  └───────────────────────────────────────────────────────────────────────────────────┘   │
    │                                                                                          │
    └──────────────────────────────────────────────────────────────────────────────────────────┘
                                                   │
                                       Tip/Ring (RJ11: green=Tip, red=Ring)
                                                   │
                                                   ▼
                                    ┌─────────────────────────────┐
                                    │     Vintage Telephone       │
                                    │   (500/2500 series, etc.)   │
                                    │                             │
                                    │  • Handset (mic + earpiece) │
                                    │  • Rotary or touch-tone dial│
                                    │  • Electromechanical ringer │
                                    └─────────────────────────────┘


Major Subsystems
----------------

**Power Supply**
   Converts external 48V DC into all required voltage rails. See :doc:`power-supply` for detailed design.

   - **12V**: SLIC analog power (VB+ pin)
   - **3.3V**: ESP32, audio codecs, digital logic
   - **-48V**: Telephone loop feed (on-hook voltage)
   - **90V AC 20Hz**: Ringer voltage (via ring relay)

**ESP32-WROVER-IE**
   Central microcontroller handling all digital functions:

   - Bluetooth HFP 1.7 client (internal radio)
   - I2S audio interface (8kHz, 16-bit, mono)
   - GPIO monitoring and control
   - WiFi for status/configuration web interface
   - FreeRTOS-based firmware

**Audio Codec (PCM5100 + PCM1808)**
   Converts between digital I2S and analog audio for the SLIC. See :doc:`line-interface-hc5504b` for codec details.

   - **PCM5100 DAC**: ESP32 → Phone earpiece path
   - **PCM1808 ADC**: Phone microphone → ESP32 path
   - Hardware-configured ("wire and go" - no I2C needed)

**HC-5504B SLIC**
   Subscriber Line Interface Circuit - the heart of the telephone interface. See :doc:`line-interface-hc5504b` for complete documentation.

   - Provides -48V battery feed to telephone
   - 2-wire to 4-wire hybrid conversion
   - Off-hook detection (switch hook detect)
   - Ring relay control for ringer activation

**Vintage Telephone**
   Standard analog telephone connected via RJ11 (tip/ring):

   - Receives -48V loop current when on-hook
   - Off-hook draws 20-80mA loop current
   - Ringer activated by 90V AC at 20Hz
   - Audio via tip/ring pair (green=Tip, red=Ring)


Signal Flow Summary
-------------------

**Outgoing Call (Phone → Mobile)**
   1. User lifts handset (off-hook detected via GPIO 32)
   2. ESP32 plays dial tone via I2S → DAC → SLIC → earpiece
   3. User dials number (pulse or DTMF detected)
   4. ESP32 initiates call via Bluetooth HFP
   5. Voice: Mic → SLIC TX → ADC → I2S → ESP32 → Bluetooth → Mobile

**Incoming Call (Mobile → Phone)**
   1. Mobile phone rings, HFP notifies ESP32
   2. ESP32 activates ring relay (GPIO 13 → SLIC RC)
   3. 90V AC rings the telephone bell
   4. User lifts handset (off-hook detected, ring stops)
   5. Voice: Mobile → Bluetooth → ESP32 → I2S → DAC → SLIC RX → earpiece


Cross-References
----------------

- :doc:`power-supply` - Power supply design and voltage rails
- :doc:`pin-assignments` - Complete ESP32 GPIO pin mapping
- :doc:`line-interface-hc5504b` - HC-5504B SLIC circuit details
- :doc:`../firmware/architecture` - Firmware module organization
- :doc:`../firmware/audio-subsystem` - Audio software implementation
