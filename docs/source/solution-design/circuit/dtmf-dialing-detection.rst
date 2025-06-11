DTMF Tone Detection
===================

Dual-Tone Multi-Frequency (DTMF) signaling is used by modern touch-tone telephones to transmit digits over an audio channel. Each button press on the keypad generates a unique combination of two tones—one from a low-frequency group and one from a high-frequency group.

The Ma Bell Gateway must support detection of DTMF tones in two primary use cases:

- When the user dials a number using a **touch-tone phone keypad** to place an outgoing call
- When the user sends DTMF tones **during an active Bluetooth call**, such as to interact with automated systems (e.g., voicemail or customer service menus)

DTMF Tone Grid
--------------

Each DTMF digit is formed by combining one low-frequency and one high-frequency tone:

.. list-table::
   :header-rows: 1
   :widths: 12 12 12 12 12

   * - 
     - **1209 Hz**
     - **1336 Hz**
     - **1477 Hz**
     - **1633 Hz**
   * - **697 Hz**
     - 1
     - 2
     - 3
     - A
   * - **770 Hz**
     - 4
     - 5
     - 6
     - B
   * - **852 Hz**
     - 7
     - 8
     - 9
     - C
   * - **941 Hz**
     - *
     - 0
     - #
     - D

Detection Options
-----------------

Several approaches can be used to detect DTMF tones depending on system requirements and available resources.

**Option 1: Software FFT-Based Detection**

This method uses a Fast Fourier Transform (FFT) on blocks of audio samples to detect frequency peaks.

- Capture PCM audio via I2S or ADC
- Perform FFT over ~40–80ms of data
- Identify dominant frequency pair
- Map to keypad digit

**Pros:**
- Flexible and visualizable
- Uses standard DSP libraries

**Cons:**
- Heavier CPU load
- Prone to false positives in noisy environments

**Option 2: Goertzel Algorithm**

A more efficient approach optimized for detecting known fixed frequencies (like DTMF).

- Runs on small audio frames (10–20ms)
- Computes energy at DTMF-specific frequencies
- Detects tone pairs with low overhead

**Pros:**
- Highly efficient for microcontrollers
- Accurate and fast
- Ideal for streaming audio detection

**Cons:**
- Slightly more involved to implement than FFT

**Option 3: External DTMF Decoder IC (e.g., MT8870)**

Uses a dedicated hardware chip that decodes audio tones and outputs 4-bit binary values representing digits.

- Analog audio in, digital code out
- Outputs ready-to-use GPIO signals

**Pros:**
- Zero CPU overhead
- Hardware-level reliability
- Simplifies software

**Cons:**
- Requires additional circuitry
- May need audio pre-conditioning (amplification/filtering)

ESP32 Integration
-----------------

The chosen method will connect to the ESP32 via:

- **I2S input**: For FFT or Goertzel-based detection of streaming audio
- **GPIO pins**: If using a decoder IC like MT8870
- **FreeRTOS ring buffers**: To capture and process audio frames efficiently

Recommended Approach
--------------------

The Ma Bell Gateway will use the **Goertzel algorithm** for DTMF tone detection. It balances efficiency and accuracy, requires no extra hardware, and integrates well with the existing I2S audio path used for Bluetooth audio.

This allows:

- Real-time tone decoding during active calls
- Dialing from a touch-tone phone
- Optional trigger of internal features or actions

Future Enhancements
-------------------

- **DTMF tone playback** via sine pair lookup
- **Logging or display** of detected digits
- **In-call DTMF passthrough** if supported by connected device
- **Setup code detection** (e.g., “##42” for debug mode)
