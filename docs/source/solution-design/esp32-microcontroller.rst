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

Audio Output Options
--------------------

The ESP32 supports several methods of delivering audio output for tones and Bluetooth-transmitted voice:

- **I2S Digital Audio** – Sends high-quality digital audio over GPIO to an external DAC or amplifier. This is the preferred path for both tones and voice. It supports shared audio output, assuming only one source is active at a time.
- **Built-in DAC (GPIO25/26)** – Offers basic 8-bit analog output. It works well for simple tone generation but lacks fidelity for voice transmission.
- **PWM (LEDC)** – Uses high-speed pulse-width modulation to simulate analog audio. This is suitable for square-wave tone generation but not recommended for voice.
- **Dedicated Audio Codec (via I2S)** – Advanced designs may use an external codec (e.g. PCM5102, MAX98357A) to handle both tones and voice with higher quality and proper analog outputs.

Audio output is typically routed to the earpiece via a 600-ohm matching circuit, with optional amplification or transformer coupling depending on the selected driver hardware.

Audio Input Options
-------------------

Capturing microphone audio for voice transmission over Bluetooth can be done through several methods:

- **I2S Microphone** – A digital microphone (e.g. INMP441) sends raw PCM data over the I2S bus. Offers high clarity with good noise immunity.
- **Analog Microphone with ADC** – A traditional analog mic is routed through an op-amp or transformer, then fed into one of the ESP32's ADC inputs (e.g. GPIO36–39). This approach is simple but requires careful gain and filtering to avoid distortion.
- **External Audio Codec with Mic Input** – High-fidelity audio input is possible with an I2S-based codec (e.g. ES8388), which handles mic biasing, gain, and digitization externally.

For the Ma Bell Gateway, the audio input path must interface cleanly with the Bluetooth HFP stack. Circuit design may involve impedance matching, echo mitigation, and DC blocking to ensure reliable voice transmission from a legacy phone microphone.
