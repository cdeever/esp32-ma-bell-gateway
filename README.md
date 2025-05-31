# Ma Bell Gateway

Ever wish your vintage rotary phone could make and receive real calls again? The **Ma Bell Gateway** bridges the gap between classic analog telephony and the modern Bluetooth-connected world.

This personal project uses an ESP32-WROVER-IE to emulate a Central Office (CO) line, handling call signaling, audio transport, and even nostalgic extras like ringing and dial tone generation — all over Bluetooth.

## What It Does

- Emulates a full analog telephone line with:
  - 48V DC talk voltage
  - Authentic AC ring pulses
  - Dial tone and call progress tones
- Connects to a mobile phone using the **Hands-Free Profile (HFP)**
- Supports both outgoing and incoming calls
- Lights up the rotary dial (Slimline style!) when the handset is lifted
- Plays reorder tones if left off-hook

## How It Works

The system is powered by:

- A dual-core ESP32 with Bluetooth HFP support
- Custom GPIO and PCM configuration for audio and signaling
- A tone generator and audio switching logic
- Off-hook detection with opto-isolation
- Console commands for debugging and dialing

This is more than just a Bluetooth adapter—it's a tribute to 20th-century phone tech.

## Development Framework

The project is built using the [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/) (Espressif IoT Development Framework). To build it yourself:

```
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Make sure you're using a compatible ESP-IDF version (v5.0 or later is recommended) and have set up your environment variables properly.

## Documentation

The documentation is written in reStructuredText and built using [Sphinx](https://www.sphinx-doc.org/). To build the docs locally:

```
cd docs
make html
```

To view full documentation, including circuit diagrams, signaling waveforms, and hardware hookup details, visit:

📘 **[Project Docs on GitHub Pages](https://cdeever.github.io/esp32-ma-bell-gateway/index.html)**  

## Who It's For

This is a **personal project** built for fun, learning, and nostalgia. If you're a retro tech nerd or embedded dev curious about:

- Bluetooth HFP on ESP32
- Telephony signaling
- Re-animating 1970s Bell System gear

...then feel free to poke around. You might find something fun in the `/docs` folder or source code.

---

Or just plug in a rotary phone, power it up, and hear that familiar click... 😎📞
