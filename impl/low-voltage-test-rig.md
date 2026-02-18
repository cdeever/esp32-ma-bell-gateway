# Ma Bell Gateway — Low-Voltage Firmware Test Rig

This guide builds a safe, all-breadboard test rig that exercises every firmware
interface without any high-voltage hardware. The production design needs −48V DC
(SLIC loop), +12V (SLIC power), and 90V AC (ringer) — but the firmware only
interacts with 3.3V GPIO signals and I2S audio. By replacing the SLIC, telephone,
and ring generator with low-voltage equivalents, you get a bench setup that can
develop and test every firmware feature.

**Prerequisite:** Sub-Assembly A from the
[prototyping build guide](prototyping-build-guide.md) (ESP32 + PCM5100 DAC +
PCM1808 ADC on breadboard with I2S wiring). This test rig adds simulator modules
to that existing assembly.

---

## What Gets Replaced

| Production Hardware | Test Rig Replacement | Voltage |
|---|---|---|
| Telephone earpiece (via SLIC) | Small 8Ω speaker on DAC output | 3.3V |
| Telephone microphone (via SLIC) | Electret mic breakout on ADC input | 3.3V |
| Hook switch (via SLIC SHD pin) | SPST toggle switch pulling GPIO 32 LOW | 3.3V |
| Rotary dial pulses (via SLIC loop) | 555 timer pulse generator → GPIO 34 | 3.3V |
| DTMF dialing (via line audio) | Smartphone DTMF app → mic → Goertzel in firmware | 3.3V |
| 90V AC ringer (via ring generator) | Piezo buzzer via NPN from GPIO 13 | 3.3V |
| SLIC ring detect status (RD pin) | Loopback from GPIO 13 or manual switch → GPIO 33 | 3.3V |
| Dial lamp (27V from line) | LED + resistor from GPIO 27 | 3.3V |
| Status LEDs | LEDs + 330Ω on GPIO 2 and 18 | 3.3V |
| User button | Tactile pushbutton on GPIO 4 | 3.3V |

**What stays the same from Sub-Assembly A:** ESP32 DevKitC, PCM5100 DAC breakout,
PCM1808 ADC breakout, I2S wiring (GPIO 25/5/26/35), all codec pin strapping.

---

## Parts Checklist

### Already Have (from Sub-Assembly A)

| Qty | Component | Notes |
|-----|-----------|-------|
| 1 | ESP32-WROVER DevKitC | Must be WROVER (PSRAM) for BT Classic |
| 1 | PCM5100PWR on TSSOP-20 breakout | With decoupling, XSMT pull-up, pin strapping |
| 1 | PCM1808PWR on TSSOP-20 breakout | With decoupling, pin strapping, bias resistor |
| 1 | Full-size breadboard | 830 tie points |
| — | Jumper wires | Various lengths |

### New for Test Rig

| Qty | Component | Value / Part | Module | Notes |
|-----|-----------|-------------|--------|-------|
| 1 | SPST toggle switch | Panel or breadboard mount | Hook switch | |
| 1 | NE555 timer IC | DIP-8 | Pulse dial | |
| 1 | 8-pin DIP socket | | Pulse dial | Optional but recommended |
| 1 | Resistor | 39kΩ ¼W | Pulse dial | 555 timing R1 |
| 1 | Resistor | 68kΩ ¼W | Pulse dial | 555 timing R2 |
| 1 | Capacitor | 1µF ceramic or film | Pulse dial | 555 timing C |
| 1 | Capacitor | 10µF electrolytic | Pulse dial | 555 power decoupling |
| 1 | Momentary pushbutton | | Pulse dial | 555 trigger / dial start |
| 1 | Piezo buzzer | 3–5V, through-hole | Ring indicator | |
| 1 | NPN transistor | 2N2222A (TO-92) | Ring indicator | Buzzer driver |
| 1 | Resistor | 1kΩ ¼W | Ring indicator | Transistor base |
| 1 | Diode | 1N4148 | Ring indicator | Flyback across piezo |
| 1 | Electret mic breakout | MAX9814 or MAX4466 | Audio input | With built-in amplifier |
| 1 | Small speaker | 8Ω, 0.25–0.5W | Audio output | Or use 3.5mm jack from Sub-A |
| 4 | LEDs | 3mm or 5mm, any color | Ring, lamp, status ×2 | |
| 4 | Resistors | 330Ω ¼W | LED current limiting | |
| 1 | Tactile pushbutton | 6mm through-hole | User button | |
| 2 | Capacitors | 10µF non-polarized | Audio AC coupling | May reuse from Sub-A |

---

## Module-by-Module Build Instructions

### Module 1: Hook Switch Simulator

**GPIO 32** — `PIN_OFF_HOOK_DETECT` (input, internal pull-up enabled)

This replaces the SLIC SHD pin. The firmware polls GPIO 32 every 20ms with a
50ms debounce window. Active LOW: LOW = off-hook, HIGH = on-hook.

**Wiring:**

```
GPIO 32 ──┤ ├── GND
        (SPST toggle)
```

- Connect one terminal of the SPST toggle switch to GPIO 32
- Connect the other terminal to GND
- No external pull-up needed — firmware enables the ESP32 internal pull-up
  (`GPIO_PULLUP_ENABLE` in `slic_interface.c` line 72)

**Operation:**
- Switch OPEN → GPIO 32 reads HIGH via internal pull-up → **on-hook**
- Switch CLOSED → GPIO 32 pulled to GND → **off-hook**

**Test:** Toggle the switch and observe state changes on the serial monitor.
The web interface should also reflect the phone state. Going off-hook should
trigger a dial tone through the speaker (Module 2).

**Firmware references:**
- `main/hardware/slic_interface.c` — poll interval 20ms, debounce 50ms, active LOW
- `main/config/pin_assignments.h:9` — `PIN_OFF_HOOK_DETECT` = GPIO 32

---

### Module 2: Audio I/O (Speaker + Microphone)

**Reuses all I2S wiring from Sub-Assembly A** — GPIO 25 (LRCLK), GPIO 5 (BCLK),
GPIO 26 (DOUT), GPIO 35 (DIN). I2S config: 8kHz sample rate, 16-bit, mono,
Philips format. BCLK = 256kHz.

#### Speaker Output (DAC → Speaker)

```
PCM5100 OUTL ──┤10µF├── 8Ω Speaker ── GND
              (AC coupling)
```

- PCM5100 OUTL pin → 10µF AC coupling cap → speaker positive terminal
- Speaker negative terminal → GND
- If using the 3.5mm headphone jack from Sub-Assembly A, no additional wiring needed
- Tone output level: 20% full scale (volume factor 0.2 in `audio_output.c`)

#### Microphone Input (Mic → ADC)

```
Mic Breakout OUT ──┤10µF├── PCM1808 VINL
                  (AC coupling)
Mic Breakout VCC ── 3.3V
Mic Breakout GND ── GND
```

- Electret mic breakout audio output → 10µF AC coupling cap → PCM1808 VINL
- Mic breakout VCC → 3.3V, GND → GND

**Level note:** If audio clips (distortion on serial monitor or during BT call),
add a voltage divider before the AC coupling cap:

```
Mic OUT ── 10kΩ ──┬── 10µF ── PCM1808 VINL
                  │
                 10kΩ
                  │
                 GND
```

This attenuates the mic signal by half (−6dB).

**Test:**
1. Go off-hook (Module 1) → hear dial tone (350Hz + 440Hz) through the speaker
2. During a BT call, speak into the mic → verify the far end hears you
3. Far-end speech should come through the speaker

**Firmware references:**
- `main/config/audio_config.h` — sample rate 8000, buffer size 1024
- `main/audio/audio_output.c` — I2S channel config, tone generation
- `main/audio/tones.c` — tone frequency and cadence definitions

---

### Module 3: Pulse Dial Simulator (555 Timer)

**GPIO 34** — `PIN_PULSE_DIAL_IN` (input-only pin)

Rotary dials generate a series of pulses to indicate the dialed digit: 1 pulse
for digit 1, 2 for digit 2, … 10 for digit 0. Standard rate is 10 pulses per
second with a 60/40 break/make ratio (60ms break, 40ms make per pulse).

#### Option A: 555 Astable (Automated Pulses)

The 555 in astable mode generates continuous pulses at ~10Hz. A momentary button
gates the oscillation.

**555 timing values:**
- R1 = 39kΩ, R2 = 68kΩ, C = 1µF
- HIGH time ≈ 0.693 × (R1 + R2) × C = 0.693 × 107kΩ × 1µF ≈ 74ms
- LOW time ≈ 0.693 × R2 × C = 0.693 × 68kΩ × 1µF ≈ 47ms
- Frequency ≈ 1 / (74ms + 47ms) ≈ 8.3Hz
- Adjust R1/R2 to fine-tune; closer to 10Hz is ideal

**Wiring:**

```
        3.3V
         │
    ┌────┴────┐
    │    8    4│── 3.3V (via momentary button for gating)
    │  VCC  RST│
    │         │
    │    7    3│── GPIO 34 (pulse output)
    │  DIS  OUT│
    │         │
    │    6    2│──┬── trigger (pin 2 = pin 6 in astable)
    │  THR  TRG│  │
    │         │  C (1µF)
    │    1    5│  │
    │  GND  CVT│  GND
    └────┬────┘
         │
        GND

    Pin 7 (DIS) ── R2 (68kΩ) ──┬── R1 (39kΩ) ── 3.3V
                                │
                          Pin 6 & Pin 2
                                │
                           C (1µF)
                                │
                               GND

    Pin 5 (CVT) ── 10nF ── GND  (bypass, optional but recommended)
```

**Gating:** Wire pin 4 (RESET) to 3.3V through a momentary pushbutton. When the
button is released, RESET goes LOW (via 10kΩ pull-down to GND) and the 555 stops.
Press and hold to generate pulses. Count the pulses for the desired digit.

**Alternative gating:** Connect 3.3V → pushbutton → 555 VCC (pin 8). The 555
only oscillates while the button is held.

#### Option B: Manual Pulse Button (Simplest)

Skip the 555 entirely. Connect a momentary pushbutton between GPIO 34 and GND.
Add a 10kΩ pull-up to 3.3V (GPIO 34 is input-only, no internal pull-up available).
Tap the button at roughly 10Hz to simulate pulses.

```
3.3V ── 10kΩ ──┬── GPIO 34
               │
              ┤ ├── GND
           (momentary)
```

This is less precise but requires no extra components and works fine for initial
firmware development.

**Test:** Generate pulses and verify firmware decodes the correct digit count.
The serial monitor should log each detected pulse and the final digit after the
inter-digit timeout.

**Firmware references:**
- `main/config/pin_assignments.h:13` — `PIN_PULSE_DIAL_IN` = GPIO 34
- `docs/source/solution-design/circuit/pulse-dialing-detection.rst` — timing spec: 60–70ms break, 40ms make, 10 pps

---

### Module 4: DTMF Testing (Software Goertzel via Mic/ADC)

**No extra hardware needed.** This module uses the existing electret mic and
PCM1808 ADC path from Module 2.

DTMF (Dual-Tone Multi-Frequency) signaling encodes each key as two simultaneous
tones. Rather than using a hardware decoder IC (MT8870), the firmware implements
the Goertzel algorithm to detect DTMF tones from the I2S audio stream.

**DTMF frequency grid:**

|        | 1209 Hz | 1336 Hz | 1477 Hz |
|--------|---------|---------|---------|
| 697 Hz | 1       | 2       | 3       |
| 770 Hz | 4       | 5       | 6       |
| 852 Hz | 7       | 8       | 9       |
| 941 Hz | *       | 0       | #       |

**GPIO 39** (`PIN_DTMF_IN`) is **not used** in this approach — the Goertzel
algorithm works on the I2S audio data directly.

**Test procedure:**
1. Install a DTMF tone generator app on a smartphone
2. Go off-hook (Module 1) to put the system in dialing mode
3. Hold the phone near the electret mic and play DTMF tones
4. Verify the firmware decodes the correct digit from the audio stream
5. Minimum tone duration: 50ms, minimum inter-digit pause: 50ms

**Firmware references:**
- `main/config/pin_assignments.h:14` — `PIN_DTMF_IN` = GPIO 39 (reserved, not used for Goertzel)
- `docs/source/solution-design/circuit/dtmf-dialing-detection.rst` — frequency grid, detection options

---

### Module 5: Ring Indicator (Piezo Buzzer)

**GPIO 13** — `PIN_RING_COMMAND` (output)

In production, GPIO 13 drives the SLIC ring command (RC) pin. In the test rig,
it drives a piezo buzzer through an NPN transistor to simulate ringer output.
The ring command is active LOW.

**Wiring:**

```
        3.3V (or 5V for louder)
         │
       Piezo Buzzer (+)
         │
    ┌────┤ 1N4148 (flyback diode, cathode at top)
    │    │
    │  Piezo Buzzer (−)
    │    │
    │    ├── 2N2222A Collector
    │    │
    │    │   2N2222A Base ── 1kΩ ── GPIO 13
    │    │
    │    └── 2N2222A Emitter ── GND
    │
    └── (diode anode also connects to collector)
```

**Note on polarity:** GPIO 13 LOW = ring active = transistor ON = buzzer sounds.
Since the 2N2222A needs a HIGH to turn on, the firmware's active-LOW ring command
needs an inverter. Two options:

1. **Software inversion:** Configure the ring command as active HIGH for the test
   rig (if firmware supports it)
2. **Hardware inversion:** Add a second NPN stage, or simply wire GPIO 13 through
   a 10kΩ pull-up to 3.3V and use a PNP transistor (2N3906) instead:

```
        3.3V
         │
       Piezo Buzzer (+)
         │
       Piezo Buzzer (−)
         │
         ├── 2N3906 Emitter ── 3.3V
         │
         │   2N3906 Base ── 1kΩ ── GPIO 13
         │
         └── 2N3906 Collector ── Buzzer (−) ── GND
```

With PNP: GPIO 13 LOW → base pulled low → PNP turns ON → buzzer sounds. This
matches the active-LOW ring command directly.

**Visual indicator:** Add an LED + 330Ω in parallel with the buzzer for a visual
ring indication.

**Ring cadence:** Standard North American ring cadence is 2 seconds on, 4 seconds
off. The buzzer should follow this pattern during an incoming BT call.

**Test:** Initiate an incoming call to the paired phone via BT. The buzzer should
sound for 2s and silence for 4s, repeating until the call is answered or ends.

**Firmware references:**
- `main/config/pin_assignments.h:11` — `PIN_RING_COMMAND` = GPIO 13
- `main/hardware/slic_interface.h` — ring command support noted for future implementation

---

### Module 6: Ring Detect Feedback

**GPIO 33** — `PIN_RING_DETECT` (input)

In production, the SLIC RD (ring detect) pin indicates when ringing voltage is
present on the line. In the test rig, create a loopback from the ring command
output so the firmware can verify ring status.

#### Option A: Direct Loopback

Wire GPIO 13 (ring command output) directly to GPIO 33 (ring detect input).
When the firmware commands ringing, it immediately reads the ring-active status.

```
GPIO 13 ────── GPIO 33
```

This is the simplest approach and adequate for firmware testing.

#### Option B: Manual Override

Use a separate SPST toggle switch, same as the hook switch:

```
GPIO 33 ──┤ ├── GND
        (SPST toggle)
```

Enable the internal pull-up on GPIO 33. Switch CLOSED = ring detected (LOW).
This allows testing ring detection independently from ring command.

**Test:** During an incoming call, verify that GPIO 33 reads LOW (ring active)
when GPIO 13 is driving the ringer, and HIGH when ringing stops.

**Firmware references:**
- `main/config/pin_assignments.h:10` — `PIN_RING_DETECT` = GPIO 33
- `main/hardware/slic_interface.h` — ring detect support noted for future implementation

---

### Module 7: Dial Lamp, Status LEDs, and User Button

These are straightforward GPIO connections — LEDs with current-limiting resistors
and a button with a pull-up.

#### Dial Lamp — GPIO 27

```
GPIO 27 ── 330Ω ──|>|── GND
                  (LED)
```

The dial lamp should light when the handset is off-hook (firmware drives GPIO 27
HIGH).

#### Status LED — GPIO 2

```
GPIO 2 ── 330Ω ──|>|── GND
                 (LED)
```

General status indication (e.g., BT connected).

**Note:** GPIO 2 is also connected to the ESP32 DevKitC on-board LED on most
boards. The external LED will mirror the on-board LED behavior.

#### UI LED — GPIO 18

```
GPIO 18 ── 330Ω ──|>|── GND
                  (LED)
```

Additional status indication (e.g., call active, incoming ring).

#### User Button — GPIO 4

```
3.3V ── (internal pull-up)
              │
           GPIO 4 ──┤ ├── GND
                  (tactile button)
```

- Firmware enables internal pull-up
- Button pressed = GPIO 4 reads LOW
- Used for pairing initiation, factory reset, or other user actions

**Test:** Verify each LED lights for its expected firmware condition. Verify the
button press is detected and triggers the expected action.

**Firmware references:**
- `main/config/pin_assignments.h:24–27` — `PIN_DIAL_LAMP_CTRL` = GPIO 27, `PIN_STATUS_LED` = GPIO 2, `PIN_UI_LED` = GPIO 18, `PIN_USER_BUTTON` = GPIO 4

---

## Breadboard Layout

All modules fit on one full-size breadboard (or two side by side if you need
more room).

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Full-Size Breadboard                         │
│                                                                     │
│  ┌──────────────────────┐    ┌────────────────────────────────────┐ │
│  │   SUB-ASSEMBLY A     │    │     TEST RIG MODULES               │ │
│  │                      │    │                                    │ │
│  │  ESP32 DevKitC       │    │  Hook Switch (toggle)              │ │
│  │    (center span)     │    │  User Button (tactile)             │ │
│  │                      │    │                                    │ │
│  │  PCM5100 DAC         │    │  555 Timer + components            │ │
│  │    (breakout board)  │    │    (pulse dial simulator)          │ │
│  │                      │    │                                    │ │
│  │  PCM1808 ADC         │    │  Buzzer + NPN/PNP driver           │ │
│  │    (breakout board)  │    │    (ring indicator)                │ │
│  │                      │    │                                    │ │
│  │  Mic breakout        │    │  LEDs × 4 + resistors              │ │
│  │  Speaker / jack      │    │    (ring, lamp, status ×2)         │ │
│  └──────────────────────┘    └────────────────────────────────────┘ │
│                                                                     │
│  ═══════════════════ Power Rails (3.3V / GND) ════════════════════  │
└─────────────────────────────────────────────────────────────────────┘
```

**Placement tips:**
- Keep the ESP32 centered so GPIO pins are accessible on both sides
- Place audio components (mic breakout, speaker) near the codecs to keep analog
  runs short
- Place digital I/O modules (switches, LEDs, 555) near the ESP32 GPIO pins they
  connect to
- Run 3.3V and GND on the breadboard power rails, fed from the ESP32 DevKitC
  3.3V and GND pins
- No high voltages anywhere — everything runs on 3.3V from the ESP32

---

## Firmware Test Matrix

Each row is a firmware feature to test. The Module column shows which test rig
module exercises it.

| Firmware Feature | Module | GPIO / Bus | Test Procedure | Expected Result |
|---|---|---|---|---|
| Off-hook detection | 1: Hook Switch | GPIO 32 | Toggle switch to off-hook | Serial log: state change, web UI updates |
| Dial tone on off-hook | 1 + 2 | GPIO 32, I2S | Go off-hook | 350Hz + 440Hz continuous through speaker |
| Off-hook warning | 1 + 2 | GPIO 32, I2S | Leave off-hook >30s without dialing | 1400Hz + 2060Hz, 0.1s on/off cadence |
| Pulse dial digits | 3: 555 Timer | GPIO 34 | Generate N pulses | Firmware decodes digit N (1–10 pulses = 1–0) |
| DTMF dial digits | 4: Goertzel | I2S ADC | Play DTMF tone into mic | Firmware decodes correct digit from audio |
| Outgoing call via BT | 1 + 2 + 3 or 4 | GPIO 32, BT | Off-hook → dial number → wait | Ringback tone (440+480Hz, 2s/4s), then call connects |
| Incoming call ring | 5: Buzzer | GPIO 13 | Call the paired phone from another | Buzzer sounds 2s on / 4s off |
| Ring trip (answer) | 1 + 5 | GPIO 32, 13 | Go off-hook during ringing | Ringing stops, call connects, audio bridged |
| Ring detect loopback | 6: Feedback | GPIO 33, 13 | Incoming call triggers ring | GPIO 33 reads LOW when ring active |
| Bidirectional audio | 2: Speaker + Mic | I2S, BT | Active BT call | Hear far end on speaker, far end hears mic |
| Call disconnect | 1 | GPIO 32, BT | Go on-hook during call | Call ends, BT HFP hangs up |
| Busy signal | 2 | I2S | Call a busy number | 480Hz + 620Hz, 0.5s on/off |
| Reorder tone | 2 | I2S | Dial invalid number | 480Hz + 620Hz, 0.25s on/off (fast busy) |
| Congestion tone | 2 | I2S | Network congestion | 480Hz + 620Hz, 0.2s on / 0.3s off |
| Confirmation tone | 2 | I2S | Feature activation | 350Hz + 440Hz, 0.1s beeps |
| Call waiting tone | 2 | I2S | Second incoming call | 440Hz single, 0.3s beep |
| SIT tone | 2 | I2S | Intercept condition | 950Hz + 1400Hz, 0.274s |
| Stutter dial tone | 2 | I2S | Voicemail waiting | 350Hz + 440Hz, 0.1s on/off stutter |
| Dial lamp | 7: LED | GPIO 27 | Go off-hook | LED lights |
| Status LED | 7: LED | GPIO 2 | Pair BT, connect | LED reflects BT status |
| UI LED | 7: LED | GPIO 18 | Various states | LED reflects call/ring status |
| User button | 7: Button | GPIO 4 | Press button | Triggers pairing or reset action |
| State management | All | All | Full call lifecycle | Web interface shows correct states throughout |
| Event system | All | All | All state transitions | Correct events published for each change |

---

## Tone Quick Reference

All tones from `main/audio/tones.c`, verified against firmware:

| Tone | Freq 1 | Freq 2 | On | Off | Trigger |
|---|---|---|---|---|---|
| Dial tone | 350 Hz | 440 Hz | continuous | continuous | Off-hook, no digits dialed |
| Ringback | 440 Hz | 480 Hz | 2.0s | 4.0s | Outgoing call ringing |
| Busy signal | 480 Hz | 620 Hz | 0.5s | 0.5s | Called number busy |
| Reorder (fast busy) | 480 Hz | 620 Hz | 0.25s | 0.25s | Invalid number / routing failure |
| Off-hook warning | 1400 Hz | 2060 Hz | 0.1s | 0.1s | Handset left off-hook too long |
| Congestion | 480 Hz | 620 Hz | 0.2s | 0.3s | Network congestion |
| Confirmation | 350 Hz | 440 Hz | 0.1s | 0.1s | Feature activation beeps |
| Call waiting | 440 Hz | — | 0.3s | — | Second incoming call alert |
| SIT | 950 Hz | 1400 Hz | 0.274s | 0.0s | Intercept / error condition |
| Stutter dial | 350 Hz | 440 Hz | 0.1s | 0.1s | Voicemail waiting indicator |

---

## What This Rig CAN'T Test

The following require the full Sub-Assembly B (SLIC) and Sub-Assembly C (ring
generator) from the prototyping build guide:

- Actual telephone ringer operation (mechanical bell, impedance matching)
- SLIC loop current behavior and line voltage regulation
- Audio coupling through the SLIC hybrid (2-wire ↔ 4-wire conversion)
- Ring generator high-voltage output (90V AC sine wave)
- Line protection circuits (MOV, fusing)
- Real rotary phone contact bounce characteristics (the 555 generates cleaner pulses than an actual dial)
- −48V / +12V power management and sequencing

The test rig validates all firmware logic and GPIO interfaces. Electrical
integration testing happens when you move to the full prototyping assemblies.

---

## GPIO Quick Reference

All pin assignments from `main/config/pin_assignments.h`:

| GPIO | Define | Direction | Test Rig Connection |
|------|--------|-----------|---------------------|
| 2 | `PIN_STATUS_LED` | Output | LED + 330Ω → GND |
| 4 | `PIN_USER_BUTTON` | Input | Tactile button → GND (internal pull-up) |
| 5 | `PIN_PCM_CLK_OUT` | Output | I2S BCLK → codecs (Sub-A) |
| 13 | `PIN_RING_COMMAND` | Output | PNP transistor → piezo buzzer |
| 18 | `PIN_UI_LED` | Output | LED + 330Ω → GND |
| 25 | `PIN_PCM_FSYNC` | Output | I2S LRCLK → codecs (Sub-A) |
| 26 | `PIN_PCM_DOUT` | Output | I2S data out → DAC (Sub-A) |
| 27 | `PIN_DIAL_LAMP_CTRL` | Output | LED + 330Ω → GND |
| 32 | `PIN_OFF_HOOK_DETECT` | Input | SPST toggle → GND (internal pull-up) |
| 33 | `PIN_RING_DETECT` | Input | Loopback from GPIO 13 or switch |
| 34 | `PIN_PULSE_DIAL_IN` | Input | 555 output or manual button (external pull-up) |
| 35 | `PIN_PCM_DIN` | Input | I2S data in ← ADC (Sub-A) |
| 39 | `PIN_DTMF_IN` | Input | Not used (Goertzel via I2S instead) |
