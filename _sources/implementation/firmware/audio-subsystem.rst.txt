Audio Subsystem
===============

The audio subsystem provides bidirectional audio transport between the vintage telephone handset and a Bluetooth-connected mobile phone. It also generates telephone signaling tones (dial tone, busy signal, etc.).

Architecture Overview
---------------------

The audio subsystem consists of three main modules:

.. code-block:: none

   ┌─────────────────────────────────────────────────────────────────────┐
   │                        Audio Subsystem                              │
   ├─────────────────────────────────────────────────────────────────────┤
   │                                                                     │
   │  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐  │
   │  │   audio_output   │  │   audio_bridge   │  │      tones       │  │
   │  │                  │  │                  │  │                  │  │
   │  │ - I2S TX/RX init │  │ - Ring buffers   │  │ - Tone defs      │  │
   │  │ - Tone generator │  │ - BT↔Phone tasks │  │ - Frequencies    │  │
   │  │ - Audio write    │  │ - HFP callbacks  │  │ - Cadences       │  │
   │  └──────────────────┘  └──────────────────┘  └──────────────────┘  │
   │                                                                     │
   └─────────────────────────────────────────────────────────────────────┘

**Module Responsibilities:**

- ``audio_output`` - I2S hardware management, tone generation, audio output API
- ``audio_bridge`` - Bluetooth ↔ Phone audio routing via ring buffers
- ``tones`` - Telephone tone definitions (frequencies, cadences)

Audio Data Flow
---------------

Bidirectional audio flows between the phone handset and Bluetooth:

**Phone → Bluetooth (Transmit Path):**

.. code-block:: none

   Phone Microphone
         │
         ▼
   ┌─────────────┐
   │  PCM1808    │  External ADC
   │    ADC      │  (Analog → Digital)
   └─────────────┘
         │ I2S RX (GPIO 35)
         ▼
   ┌─────────────┐
   │ audio_rx_   │  Reads I2S frames
   │   task      │  every 20ms
   └─────────────┘
         │
         ▼
   ┌─────────────┐
   │ bt_tx_      │  Ring buffer
   │ ringbuf     │  (3600 bytes)
   └─────────────┘
         │
         ▼
   ┌─────────────┐
   │ HFP outgoing│  Bluetooth callback
   │  callback   │  reads from buffer
   └─────────────┘
         │
         ▼
   Mobile Phone (via Bluetooth)

**Bluetooth → Phone (Receive Path):**

.. code-block:: none

   Mobile Phone (via Bluetooth)
         │
         ▼
   ┌─────────────┐
   │ HFP incoming│  Bluetooth callback
   │  callback   │  writes to buffer
   └─────────────┘
         │
         ▼
   ┌─────────────┐
   │ bt_rx_      │  Ring buffer
   │ ringbuf     │  (3600 bytes)
   └─────────────┘
         │
         ▼
   ┌─────────────┐
   │ audio_tx_   │  Reads from buffer,
   │   task      │  writes via audio_output
   └─────────────┘
         │ (blocked if tone playing)
         ▼
   ┌─────────────┐
   │  PCM5100    │  External DAC
   │    DAC      │  (Digital → Analog)
   └─────────────┘
         │ I2S TX (GPIO 26)
         ▼
   Phone Earpiece/Speaker

I2S Configuration
-----------------

Both TX and RX channels share a single I2S port with unified configuration.

.. note::
   **Codec Choice Rationale:** The PCM5100 (DAC) and PCM1808 (ADC) were selected for "wire and go" simplicity - they require no I2C initialization or MCLK signal. While their specs (106 dB / 98 dB SNR) far exceed 8kHz telephony requirements, this trade-off eliminates codec driver complexity entirely. See the circuit documentation for details.

**I2S Parameters:**

.. list-table::
   :widths: 25 25 50
   :header-rows: 1

   * - Parameter
     - Value
     - Notes
   * - I2S Port
     - I2S_NUM_0
     - Single port for both TX/RX
   * - Sample Rate
     - 8000 Hz
     - Standard telephony rate
   * - Bit Depth
     - 16-bit
     - Signed PCM samples
   * - Slot Mode
     - MONO
     - Single channel for voice
   * - Format
     - I2S Standard (Philips)
     - Standard I2S framing
   * - MCLK
     - Disabled
     - Not required for external codec

**GPIO Pin Assignments:**

.. list-table::
   :widths: 15 20 65
   :header-rows: 1

   * - GPIO
     - Signal
     - Description
   * - 25
     - PCM_FSYNC
     - Word select / frame sync (LRCLK)
   * - 5
     - PCM_CLK_OUT
     - Bit clock (BCLK)
   * - 26
     - PCM_DOUT
     - Audio output to DAC (TX)
   * - 35
     - PCM_DIN
     - Audio input from ADC (RX, input-only pin)

**Unified Channel Creation:**

ESP-IDF requires both TX and RX channels on the same I2S port to be created in a single call:

.. code-block:: c

   // Create both channels together (required by ESP-IDF)
   i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle);

   // Initialize each with matching configuration
   i2s_channel_init_std_mode(tx_handle, &tx_std_cfg);
   i2s_channel_init_std_mode(rx_handle, &rx_std_cfg);

Bluetooth HFP Audio Path
------------------------

The system uses the **HCI audio path** for Bluetooth HFP audio, enabling software control over audio routing.

**Why HCI Path (not PCM)?**

.. list-table::
   :widths: 15 42 43
   :header-rows: 1

   * - Path
     - How It Works
     - Trade-offs
   * - **HCI**
     - Audio flows through HCI callbacks. Software handles audio frames via ring buffers.
     - Enables tone mixing, volume control, audio processing. Small CPU overhead.
   * - PCM
     - Bluetooth controller routes audio directly via I2S DMA. CPU never sees audio.
     - Zero CPU overhead but no software control. Tones can't interrupt calls.

The HCI path is configured in ``sdkconfig.defaults``:

.. code-block:: none

   CONFIG_BT_HFP_AUDIO_DATA_PATH_HCI=y

**HFP Data Callbacks:**

Two callbacks handle Bluetooth audio:

.. code-block:: c

   // Called when Bluetooth needs audio to send (Phone → BT)
   static uint32_t bt_app_hf_client_outgoing_cb(uint8_t *p_buf, uint32_t sz)
   {
       // Read from bt_tx_ringbuf (filled by audio_rx_task)
       // Return number of bytes provided
   }

   // Called when Bluetooth has audio to deliver (BT → Phone)
   static void bt_app_hf_client_incoming_cb(const uint8_t *buf, uint32_t sz)
   {
       // Write to bt_rx_ringbuf (read by audio_tx_task)
   }

Tone Generation
---------------

The audio subsystem generates authentic North American telephone tones.

**Supported Tones:**

.. list-table::
   :widths: 25 20 20 35
   :header-rows: 1

   * - Tone
     - Frequencies
     - Cadence
     - Usage
   * - Dial Tone
     - 350 + 440 Hz
     - Continuous
     - Phone off-hook, ready to dial
   * - Ringback
     - 440 + 480 Hz
     - 2s on, 4s off
     - Outgoing call ringing
   * - Busy Signal
     - 480 + 620 Hz
     - 0.5s on/off
     - Called party busy
   * - Reorder (Fast Busy)
     - 480 + 620 Hz
     - 0.25s on/off
     - Call cannot be completed
   * - Off-Hook Warning
     - 1400+2060+2450+2600 Hz
     - 0.1s on/off
     - Phone left off-hook
   * - Call Waiting
     - 440 Hz
     - 0.3s beep
     - Incoming call during active call

**Tone Priority:**

Tones have priority over Bluetooth audio. When a tone is playing:

1. ``audio_output_tone_active()`` returns true
2. ``audio_output_write()`` silently drops Bluetooth audio
3. Tone continues until explicitly stopped

**Tone API:**

.. code-block:: c

   // Start playing a tone (thread-safe)
   esp_err_t audio_output_play_tone(tone_type_t tone);

   // Stop the current tone
   esp_err_t audio_output_stop_tone(void);

   // Check if tone is active
   bool audio_output_tone_active(void);

**Tone Generation Task:**

A dedicated FreeRTOS task generates tone samples using sine wave synthesis:

.. code-block:: c

   // Dual-frequency tone generation
   float sample1 = sinf(phase1);  // First frequency
   float sample2 = sinf(phase2);  // Second frequency
   float mixed = (sample1 + sample2) / 2.0f;
   buffer[i] = (int16_t)(32767.0f * TONE_VOLUME * mixed);

Ring Buffers
------------

Two ring buffers decouple audio tasks from Bluetooth callbacks:

**Buffer Configuration:**

- Size: 3600 bytes each (``AUDIO_HFP_RINGBUF_SIZE``)
- Type: Byte buffer (``RINGBUF_TYPE_BYTEBUF``)
- Capacity: ~11 audio frames (~220ms of audio)

**Buffer Roles:**

.. list-table::
   :widths: 25 35 40
   :header-rows: 1

   * - Buffer
     - Direction
     - Producer → Consumer
   * - ``bt_rx_ringbuf``
     - Bluetooth → Phone
     - HFP incoming callback → audio_tx_task
   * - ``bt_tx_ringbuf``
     - Phone → Bluetooth
     - audio_rx_task → HFP outgoing callback

Initialization Sequence
-----------------------

Audio modules are initialized in specific order:

.. code-block:: none

   main()
     └─ audio_output_init()     # Creates I2S TX+RX, starts tone task
          └─ audio_bridge_init() # Creates ring buffers, verifies RX handle
               └─ bluetooth_init()
                    └─ bt_app_hf_register_data_callbacks()  # Registers HFP callbacks

**Critical Dependencies:**

1. ``audio_output_init()`` must complete before ``audio_bridge_init()``
2. ``audio_bridge_init()`` must complete before Bluetooth audio connects
3. HFP data callbacks must be registered after HFP client initialization

Module Files
------------

**audio_output** (``main/audio/audio_output.c``, ``audio_output.h``):

- Owns I2S TX and RX channel handles
- Provides ``audio_output_write()`` for BT audio passthrough
- Provides ``audio_output_get_rx_handle()`` for audio_bridge
- Runs tone generation task

**audio_bridge** (``main/audio/audio_bridge.c``, ``audio_bridge.h``):

- Creates and manages ring buffers
- Runs ``audio_rx_task`` (Phone → BT) and ``audio_tx_task`` (BT → Phone)
- Provides ring buffer accessors for HFP callbacks

**tones** (``main/audio/tones.c``, ``tones.h``):

- Defines ``tone_type_t`` enum
- Stores tone frequency/cadence parameters
- Provides ``tone_get_definition()`` accessor

Diagnostic Output
-----------------

**Initialization Logs:**

.. code-block:: text

   I (1234) audio_output: Initializing audio output subsystem
   I (1240) audio_output: Audio I/O initialized (TX: GPIO26, RX: GPIO35)
   I (1245) audio_bridge: Initializing audio bridge
   I (1250) audio_bridge: Audio bridge initialized (ring buffers ready)

**Runtime Logs:**

.. code-block:: text

   I (5000) audio_output: Playing tone: 0  (DIAL_TONE)
   I (8000) audio_output: Playing tone: 10 (TONE_NONE - stopping)
   I (9000) audio_bridge: Audio RX task started (Phone → Bluetooth)
   I (9001) audio_bridge: Audio TX task started (Bluetooth → Phone)

References
----------

- :doc:`architecture` - Overall firmware architecture
- :doc:`state-management` - State management system
- ``docs/source/solution-design/circuit/audio.rst`` - Audio circuit design
- ``docs/source/implementation/circuit/pin-assignments.rst`` - GPIO assignments
