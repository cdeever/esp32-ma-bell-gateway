=====================
ESP32 Microcontroller
=====================

.. image:: /_images/esp32-wrover-ie.png
   :alt: ESP32-WROVER-IE Module
   :width: 300px
   :align: center

Selecting the right ESP32 variant can be surprisingly tricky. There are dozens of versions—ESP32-C2, ESP32-S3, ESP32-WROVER, WROOM-32, and more—each with its own trade-offs in price, peripherals, core count, wireless support, and package size. After researching compatibility, availability, and feature sets, the **ESP32-WROVER-IE** was chosen for this project.

This module strikes an ideal balance for the Ma Bell Gateway:

- ✅ **Dual-core processor (Xtensa LX6 @ up to 240 MHz)** – The ESP32 includes two powerful cores. **Core 0** is generally reserved for system-level tasks such as Wi-Fi and Bluetooth stacks, while **Core 1** runs the user application logic (e.g. GPIO handling, tone generation, state machine transitions). This separation is key for real-time responsiveness.
- ✅ **Built-in Wi-Fi and Bluetooth Classic** – Both are essential. Wi-Fi is used to serve a real-time web-based status page, and Bluetooth Classic enables call connectivity via the Hands-Free Profile (HFP).
- ✅ **PSRAM (typically 4MB)** – The WROVER series includes external pseudo-static RAM, which can be used for audio buffering, logging, or memory-intensive tasks. While not mandatory for this project, it provides headroom for future expansion.
- ✅ **FreeRTOS included** – The ESP-IDF framework includes **FreeRTOS** as its built-in real-time operating system and exposes a rich API for multitasking, timers, and inter-task communication. The Ma Bell Gateway uses FreeRTOS to juggle Bluetooth events, off-hook signaling, tone generation, and status updates — all without blocking or missing real-time events.
- ✅ **External antenna connector** – The “IE” version includes a u.FL connector for an external antenna, ensuring strong signal coverage throughout the home.
- ✅ **Flexible audio I/O support** – The ESP32 supports multiple options for generating and capturing audio signals. These include I2S for high-quality digital audio, built-in DACs for simple tones, ADC input for analog microphones, and optional support for external audio codecs. This flexibility allows the Ma Bell Gateway to mix modern Bluetooth voice with vintage analog signaling.

Bluetooth Classic vs BLE
------------------------

Although many ESP32 variants support **Bluetooth Low Energy (BLE)**, this project requires **Bluetooth Classic** due to its support for audio via the **Hands-Free Profile (HFP)**. BLE is optimized for short, energy-efficient data bursts (e.g. sensors, beacons), but it does not support voice or audio streaming. For this reason, only ESP32 modules that include Bluetooth Classic were considered.

Simultaneous Wi-Fi + Bluetooth
------------------------------

While Wi-Fi and Bluetooth share the same radio hardware, the ESP32 manages coexistence through fast time-sharing and protocol scheduling. With proper configuration:

- Wi-Fi can serve the internal status page and support future OTA updates.
- Bluetooth Classic can maintain a call session with a mobile phone.
- System tasks and phone logic remain responsive, even during heavy traffic.

The ability to reliably run both wireless stacks alongside real-time GPIO handling was a key reason the WROVER-IE was selected.

Audio Output Architecture
-------------------------

The Ma Bell Gateway uses an **external DAC connected via I2S** for all audio output, including tones and Bluetooth-transmitted voice. This design decision provides several key advantages:

- **High Audio Quality:** 16-bit+ resolution ensures clear voice transmission and high-fidelity tone generation
- **Flexible Output:** Single audio path supports both tone generation and voice playback, with hardware managing source switching
- **Hardware-Configured Operation:** Pin-strapping configuration eliminates complex software initialization and reduces potential software bugs
- **Production-Ready Quality:** Professional telephony audio suitable for commercial deployment

Audio output is routed from the external DAC to the telephone earpiece via the SLIC line interface, with proper impedance matching and AC coupling to ensure compatibility with standard telephone handsets.

**Alternative Approaches Considered:**

- *ESP32 Built-in DAC (GPIO25/26):* 8-bit resolution insufficient for high-quality voice transmission; suitable only for prototyping and early development testing
- *PWM Audio (LEDC):* Square-wave generation adequate for simple tones but lacks fidelity for voice and introduces switching noise incompatible with telephony quality standards

Audio Input Architecture
------------------------

The Ma Bell Gateway uses an **external ADC connected via I2S** for capturing microphone audio from the telephone handset. This architectural decision enables high-quality voice transmission over Bluetooth with several advantages:

- **Clear Voice Capture:** High signal-to-noise ratio and low distortion ensure intelligible voice transmission for Bluetooth HFP calls
- **Hardware-Configured Operation:** Pin-strapping configuration simplifies firmware and eliminates codec initialization complexity
- **Digital Interface Noise Immunity:** I2S digital interface reduces susceptibility to analog noise and ground loops common in telephony circuits
- **Legacy Phone Microphone Compatibility:** External ADC allows proper gain staging and filtering for vintage telephone handset microphones

The audio input path interfaces with the Bluetooth HFP stack to transmit voice from the telephone handset to the paired mobile device. Circuit design includes impedance matching, DC blocking, and AC coupling to ensure clean audio capture from the SLIC's transmit path.

**Alternative Approaches Considered:**

- *ESP32 Built-in ADC (GPIO36-39):* Designed for sensor readings, not continuous audio sampling; insufficient sample rate and quality for telephony applications
- *I2S Digital Microphone:* Would require replacing the telephone handset's microphone entirely; not applicable to the Ma Bell Gateway's use case of preserving the vintage phone experience
