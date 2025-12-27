# Ma Bell Gateway

Ever wish your vintage rotary phone could make and receive real calls again? The **Ma Bell Gateway** bridges the gap between classic analog telephony and the modern Bluetooth-connected world.

This personal project uses an ESP32-WROVER-IE to emulate a Central Office (CO) line, handling call signaling, audio transport, and even nostalgic extras like ringing and dial tone generation — all over Bluetooth.

## What It Does

- Emulates a full analog telephone line with:
  - -48V DC subscriber loop voltage (authentic Central Office line feed)
  - +12V DC SLIC analog power
  - 90V AC @ 20Hz ringing signal for electromechanical bells
  - Off-hook detection and call supervision
  - Professional-quality bidirectional audio (PCM5100 DAC + PCM1808 ADC)
- Connects to a mobile phone using the **Bluetooth Hands-Free Profile (HFP)**
- Supports both outgoing and incoming calls
- Lights up the rotary dial (9V AC lamp circuit, Slimline style!)
- Plays dial tone and call progress tones
- Provides web API for system status monitoring

## How It Works

At the heart of the system is the **HC-5504B Subscriber Line Interface Circuit (SLIC)**, a chip originally designed by Intersil (now Renesas) in the late 1990s for VoIP gateways and small PBX systems.

### The SLIC: Bridging Eras

The HC-5504B implements nearly all the classic **BORSCHT** functions that Bell System central offices used to provide to every telephone line:

- **B**attery feed (DC power to the phone line)
- **O**vervoltage protection
- **R**inging (control of ring voltage delivery)
- **S**upervision (off-hook detection, ring trip)
- **C**odec functions (2-wire to 4-wire hybrid conversion)
- **H**ybrid (separates transmit and receive audio paths)
- **T**est (built-in diagnostics)

By using this chip, we're essentially recreating a miniature telephone company central office—but instead of connecting to copper phone lines across a city, we're bridging to Bluetooth.

### System Architecture

The complete system includes:

- **ESP32-WROVER-IE** microcontroller (dual-core, Bluetooth Classic + WiFi)
- **HC-5504B SLIC** for analog telephone interface and line supervision
- **PCM5100 DAC** for high-quality audio playback (Bluetooth → Phone)
- **PCM1808 ADC** for audio capture (Phone → Bluetooth)
- **Five voltage rails** derived from a single 48V supply:
  - -48V DC (subscriber loop)
  - +12V DC (SLIC analog power)
  - +3.3V DC (ESP32 and audio codecs)
  - 90V AC @ 20Hz (ringer)
  - 9V AC (dial lamp, separate supply)
- Custom firmware for Bluetooth HFP, audio routing, and telephony signaling

This is more than just a Bluetooth adapter—it's a tribute to 20th-century phone tech, brought back to life with modern components.

## Development Framework

The project is built using the [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/) (Espressif IoT Development Framework). To build it yourself:

```
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Make sure you're using a compatible ESP-IDF version (v5.0 or later is recommended) and have set up your environment variables properly.

## Documentation

The documentation is written in reStructuredText and built using [Sphinx](https://www.sphinx-doc.org/).

### Setup

Install documentation dependencies:

```bash
pip install -r docs/requirements.txt
```

### Build and View Locally

Build HTML documentation and start a local web server:

```bash
make html
```

This builds the docs and serves them at http://localhost:8000

Other options:

```bash
make docs        # Build HTML only (no server)
make watch       # Auto-rebuild on changes with live reload
make docs-clean  # Clean build artifacts
```

### Online Documentation

To view full documentation, including circuit diagrams, power supply design, SLIC implementation details, and hardware specifications, visit:

📘 **[Project Docs on GitHub Pages](https://cdeever.github.io/esp32-ma-bell-gateway/index.html)**

Documentation is automatically published to GitHub Pages when changes are pushed to the main branch.  

## Who It's For

This is a **personal project** built for fun, learning, and nostalgia. If you're a retro tech nerd or embedded developer curious about:

- **Bluetooth HFP on ESP32** - Full-duplex audio and AT command handling
- **Classic telephony signaling** - BORSCHT functions, subscriber line interface circuits
- **Analog audio interfacing** - I2S codecs, SLIC hybrid circuits, AC coupling
- **Multi-voltage power design** - Five rails from a single supply, isolation, and protection
- **Re-animating 1970s Bell System gear** - Authentic line voltages, ring signals, and dial lamps
- **VoIP/SLIC chip applications** - Repurposing telecom ICs for modern projects

...then feel free to poke around. The `/docs` folder contains detailed circuit design rationale, implementation notes, and historical context about the Bell System components this project recreates.

---

Or just plug in a rotary phone, power it up, and hear that familiar click... 😎📞
