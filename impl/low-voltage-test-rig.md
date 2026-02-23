# Ma Bell Gateway — Low-Voltage Build (v1.0)

This guide builds a fully working Ma Bell Gateway on a breadboard using a cheap
corded desk phone, with no high-voltage hardware anywhere. A $10-15 phone with a
T/P (tone/pulse) switch provides a real hook switch, real pulse dialing, real DTMF
tones, and — with simple AC coupling — voice through the handset during BT calls.

The production design needs −48V DC (SLIC loop), +12V (SLIC power), and 90V AC
(ringer). None of that is required here. The phone's signaling and voice circuits
run on a 5V DC loop from the ESP32's USB rail, and every firmware feature gets
exercised with real hardware.

**Prerequisite:** Sub-Assembly A from the
[prototyping build guide](prototyping-build-guide.md) (ESP32 + PCM5100 DAC +
PCM1808 ADC on breadboard with I2S wiring). This build adds the phone, audio
coupling, and indicator modules to that existing assembly.

---

## v1.0 vs v2.0

| | v1.0: Low-Voltage Build (this guide) | v2.0: SLIC Build |
|---|---|---|
| **Phone** | Cheap corded desk phone (~$15) | Vintage rotary (Western Electric 500, etc.) |
| **Loop supply** | 5V USB via 330Ω (~15mA) | −48V via AG1171 SLIC (~25mA) |
| **Hook/dial** | Phone's own hook switch, pulse dial, DTMF IC | SLIC SHD pin, loop current interruption |
| **Voice** | AC-coupled onto 5V loop (handset earpiece + mic) | SLIC hybrid 2-wire ↔ 4-wire conversion |
| **Ring** | Piezo buzzer / LED from GPIO 13 | 90V AC ring generator → mechanical bell |
| **Purpose** | Prove out all firmware end-to-end | Support vintage hardware with proper line voltages |
| **BOM** | ~$30-40 total including phone | ~$80-120 (SLIC, ring gen, protection, PSU) |

v1.0 proves every firmware feature works — state machine, BT HFP audio, Goertzel
DTMF detection, pulse decoding, tone generation, call lifecycle — using a real
phone in your hand making real calls. v2.0 is the electrical engineering to make
Grandma's 1965 rotary phone work exactly like it did when LBJ was president.

---

## What Gets Replaced

| Production Hardware | v1.0 Replacement | Voltage |
|---|---|---|
| Telephone (via SLIC) | Cheap corded phone on 5V DC loop | 5V |
| Hook switch (via SLIC SHD pin) | Phone's real hook switch → voltage divider → GPIO 32 | 5V → 3.3V |
| Rotary dial pulses (via SLIC loop) | Phone's pulse dial (P mode) → GPIO 34 | 5V → 3.3V |
| DTMF dialing (via line audio) | Phone's DTMF IC (T mode) → AC couple → ADC → Goertzel | 5V |
| Earpiece audio (via SLIC hybrid) | DAC → AC couple onto loop → phone earpiece | 5V |
| Microphone audio (via SLIC hybrid) | Phone mic on loop → AC couple → ADC | 5V |
| 90V AC ringer (via ring generator) | Piezo buzzer via transistor from GPIO 13, or LED | 3.3V |
| SLIC ring detect status (RD pin) | Loopback from GPIO 13 → GPIO 33 | 3.3V |
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

### New for v1.0 Build

| Qty | Component | Value / Part | Module | Notes |
|-----|-----------|-------------|--------|-------|
| 1 | Corded desk phone | With T/P switch | Phone loop | ~$10-15, thrift store or online |
| 1 | Resistor | 330Ω ¼W | Phone loop | DC loop current limiting |
| 4 | Resistors | 10kΩ ¼W | Phone loop | Voltage dividers (hook sense, DTMF sense) |
| 1 | Capacitor | 100nF ceramic | Phone loop | AC coupling DTMF to ADC |
| 1 | Resistor | 1kΩ ¼W | Voice audio | Series resistor, DAC → loop injection |
| 1 | Capacitor | 10µF non-polarized | Voice audio | AC coupling DAC onto loop |
| 1 | Capacitor | 100nF ceramic | Voice audio | AC coupling loop to ADC (mic path) |
| 1 | Piezo buzzer | 3–5V, through-hole | Ring indicator | |
| 1 | NPN transistor | 2N2222A (TO-92) | Ring indicator | Buzzer driver |
| 1 | Resistor | 1kΩ ¼W | Ring indicator | Transistor base |
| 1 | Diode | 1N4148 | Ring indicator | Flyback across piezo |
| 4 | LEDs | 3mm or 5mm, any color | Ring, lamp, status ×2 | |
| 4 | Resistors | 330Ω ¼W | LED current limiting | |
| 1 | Tactile pushbutton | 6mm through-hole | User button | |

---

## Module-by-Module Build Instructions

### Module 1: Phone Loop Circuit (Hook, Pulse, DTMF)

A cheap corded desk phone with a T/P switch provides a real hook switch, real
pulse dialing, and real DTMF tone generation — all on a simple 5V DC loop.

**Why this works:** A phone's dialing circuits (DTMF IC, pulse contacts, hook
switch) only need a few volts and milliamps of loop current. The phone doesn't
care that it's running at 5V instead of −48V — it just needs enough current to
power the DTMF oscillator and close the hook switch contact.

#### DC Loop

Supply 5V (from the ESP32 dev board's USB rail) through a 330Ω current-limiting
resistor. The phone draws current when off-hook, creating voltage changes that
the ESP32 can detect.

```
        5V (ESP32 USB rail)
         │
       330Ω (R_loop)
         │
    A ───┤
         │
        TIP (phone, green wire)
         │
    [Corded Phone]
         │
       RING (phone, red wire)
         │
        GND
```

Point A is the sense point — where we tap voltage for hook detection, pulse
detection, and DTMF signal extraction.

**Loop current math:** With 5V supply and 330Ω, and a typical phone off-hook DC
resistance of ~100-200Ω, loop current is 5V / (330Ω + ~150Ω) ≈ 10-15mA. This
is enough for the phone's DTMF IC, pulse switch logic, and microphone circuit.

**Voltage at Point A:**
- **On-hook:** No current flows. Point A sits at 5V (supply voltage).
- **Off-hook:** Current flows through 330Ω and phone. Voltage at A drops to
  roughly 1-3V depending on the phone's DC resistance.

**If 5V isn't enough:** Some DTMF ICs need 3.5V+ across them to oscillate. If
yours doesn't generate tones at 5V, use a 9V battery with a 680Ω current-limiting
resistor instead (still ~10mA). Size the voltage dividers accordingly.

#### Hook Switch Detection — GPIO 32

`PIN_OFF_HOOK_DETECT` (input, internal pull-up enabled)

A voltage divider from Point A brings the 5V-swing signal into ESP32-safe range.

```
    Point A (from loop)
         │
        10kΩ
         │
         ├──── GPIO 32
         │
        10kΩ
         │
        GND
```

**How it works:**
- **On-hook:** Point A at 5V → divider outputs ~2.5V → GPIO 32 reads HIGH →
  firmware sees on-hook.
- **Off-hook:** Point A drops to ~1-3V → divider outputs ~0.5-1.5V → GPIO 32
  reads LOW → firmware sees off-hook.

This matches the firmware's active-LOW convention (LOW = off-hook, HIGH = on-hook).

**Firmware references:**
- `main/hardware/slic_interface.c` — poll interval 20ms, debounce 50ms, active LOW
- `main/config/pin_assignments.h:10` — `PIN_OFF_HOOK_DETECT` = GPIO 32

#### DTMF Detection — AC-Coupled to ADC

When the phone is in **T (Tone) mode** and a key is pressed, the DTMF IC generates
dual tones directly on the wire as AC current variations. AC-couple these from
Point A into the PCM1808 ADC for Goertzel decoding in firmware.

```
    Point A (from loop)
         │
        10kΩ
         │
         ├──── 100nF ──── PCM1808 VINL (GPIO 35 via I2S)
         │   (AC coupling)
        10kΩ
         │
        GND
```

The 10kΩ/10kΩ voltage divider halves the signal (−6dB) and the 100nF cap blocks
DC. The resulting AC signal feeds the PCM1808 ADC, where the firmware's Goertzel
algorithm detects the DTMF frequencies.

**DTMF frequency grid (for reference):**

|        | 1209 Hz | 1336 Hz | 1477 Hz |
|--------|---------|---------|---------|
| 697 Hz | 1       | 2       | 3       |
| 770 Hz | 4       | 5       | 6       |
| 852 Hz | 7       | 8       | 9       |
| 941 Hz | *       | 0       | #       |

**Note:** This DTMF sense path shares the PCM1808 ADC input with the voice
receive path in Module 2. Both use AC coupling to VINL. In practice, DTMF
detection happens during the dialing state (before a call connects), and voice
receive happens during the call state, so they don't conflict.

**Firmware references:**
- `main/config/pin_assignments.h:15` — `PIN_DTMF_IN` = GPIO 39 (reserved, not used for Goertzel)
- `docs/source/solution-design/circuit/dtmf-dialing-detection.rst` — frequency grid, detection options

#### Pulse Dial Detection — GPIO 34

When the phone is in **P (Pulse) mode** and a digit is dialed, the pulse switch
opens and closes the loop rapidly. Each loop break causes the voltage at Point A
to jump back to 5V (no current), then drop again when the loop closes. Standard
rate: 10 pulses/second, 60ms break / 40ms make.

Tap Point A through a second voltage divider to GPIO 34 (`PIN_PULSE_DIAL_IN`).
GPIO 34 is input-only with no internal pull-up, so include an external 10kΩ
pull-up to 3.3V.

```
    Point A (from loop)
         │
        10kΩ
         │
         ├──── GPIO 34
         │
        10kΩ
         │
        GND

    3.3V ── 10kΩ ──── GPIO 34  (external pull-up)
```

**Distinguishing pulses from hook flash:** Dial pulses arrive at ~10 per second
(60ms break, 40ms make). A hook flash is a single 300-800ms transition. The
firmware uses timing to distinguish them.

**Firmware references:**
- `main/config/pin_assignments.h:14` — `PIN_PULSE_DIAL_IN` = GPIO 34
- `docs/source/solution-design/circuit/pulse-dialing-detection.rst` — timing spec: 60–70ms break, 40ms make, 10 pps

#### T/P Switch

This is the whole point of buying a phone with T/P switching:

- **T (Tone):** Phone generates DTMF tones when keys are pressed. Use for DTMF
  detection testing via the Goertzel algorithm.
- **P (Pulse):** Phone opens/closes the loop to generate dial pulses. Use for
  pulse dial detection testing.

Switch between modes to test both dialing methods with the same hardware.

#### Complete Module 1 Wiring Diagram

All connections from a single sense point:

```
        5V (USB rail)
         │
       330Ω
         │
    A ───┼──── TIP ──── [Corded Phone] ──── RING ──── GND
         │
         ├── 10kΩ ──┬── GPIO 32 (hook detect)
         │          │
         │        10kΩ
         │          │
         │         GND
         │
         ├── 10kΩ ──┬── 100nF ── PCM1808 VINL (DTMF to ADC)
         │          │
         │        10kΩ
         │          │
         │         GND
         │
         └── 10kΩ ──┬── GPIO 34 (pulse detect)
                    │
                  10kΩ       3.3V ── 10kΩ ── GPIO 34 (pull-up)
                    │
                   GND
```

**Test:**
1. Lift handset → GPIO 32 goes LOW → serial log shows off-hook → dial tone plays
2. Set to T mode, press keys → Goertzel decodes digits from ADC audio stream
3. Set to P mode, dial a digit → firmware counts pulses and decodes digit
4. Hang up → GPIO 32 goes HIGH → serial log shows on-hook

---

### Module 2: Voice Audio (Through the Handset)

This module AC-couples the ESP32's DAC output and ADC input onto the phone's DC
loop, enabling voice through the phone's own earpiece and microphone during BT
calls.

**How it works:** The phone's earpiece and mic are connected to TIP/RING as part
of the phone's internal circuit. When off-hook, the earpiece responds to AC
voltage variations on the loop, and the mic modulates loop current. By injecting
AC audio from the DAC onto the loop and reading AC audio from the loop into the
ADC, the phone's handset becomes the audio I/O device.

#### TX Path: DAC → Phone Earpiece

The PCM5100 DAC output (far-end BT audio, tones) is AC-coupled onto the phone
loop through a series resistor. The earpiece reproduces it.

```
    PCM5100 OUTL ── 10µF ── 1kΩ ──── Point A (on DC loop)
                  (AC coupling)  (current limit)
```

- The 10µF cap blocks DC so the DAC doesn't fight the loop supply.
- The 1kΩ resistor limits current from the DAC into the loop and provides some
  isolation. The phone's earpiece (typically 100-300Ω) sees the AC signal as a
  voltage variation on the loop.
- At 20% volume (firmware default), the DAC outputs ~0.3Vrms. Through the 1kΩ
  into the loop impedance, this produces adequate earpiece volume.

#### RX Path: Phone Mic → ADC

The phone's mic signal appears as small AC current variations on the loop. Read
these from Point A through a voltage divider and AC coupling cap into the PCM1808
ADC.

```
    Point A (from loop)
         │
        10kΩ
         │
         ├──── 100nF ──── PCM1808 VINL
         │   (AC coupling)
        10kΩ
         │
        GND
```

This is the **same path** used for DTMF detection in Module 1. During dialing,
the Goertzel algorithm processes DTMF tones from this input. During a call, the
BT HFP audio bridge routes this input to the far end.

#### Echo: The Honest Truth

With simple AC coupling (no hybrid transformer), the ADC picks up **both** the
phone's mic signal (wanted) **and** the DAC's injected audio reflected back from
the loop (echo). The far-end caller will hear their own voice echoed back.

**How bad is it?** The echo level depends on the impedance relationships. The 1kΩ
series resistor on the TX path provides some attenuation — the echo at the ADC
will be weaker than the original DAC signal, but still audible to the far end.
Sidetone (hearing yourself in the earpiece) is normal and expected in telephones.

**Mitigation options, from simplest to best:**

1. **Live with it for testing.** Echo is annoying but doesn't prevent functional
   testing of the call lifecycle, state machine, and audio paths. This gets you
   making calls immediately.

2. **Firmware echo cancellation.** The ESP32 knows exactly what the DAC is
   sending. A simple adaptive filter can subtract the estimated echo from the ADC
   input. The pin assignments already reserve GPIO 19/21/22 for AEC control lines,
   and the near-zero-delay echo path (same wire, no acoustic propagation) makes
   this easier than acoustic echo cancellation. This is a firmware feature to
   implement — not a hardware change.

3. **Transformer hybrid (hardware upgrade).** A 600Ω:600Ω audio transformer
   configured as a hybrid bridge can cancel 20-30dB of echo at the hardware level
   before the ADC ever sees it. This is the classic telephone technique — it's how
   every central office worked before SLICs existed. A transformer is ~$2-3
   (Triad SP-70 or similar 600Ω 1:1 audio transformer). The hybrid uses the
   transformer in a Wheatstone bridge configuration: TX drives the bridge, the
   phone line is one arm, a 600Ω balance resistor is the other arm, and the RX
   sense point sits at the null where TX cancels. This is a worthwhile upgrade
   once basic call functionality is working, but not required to get started.

#### Option B: External Speaker + Mic (Fallback)

If AC coupling to the handset proves problematic (phone mic doesn't work at low
voltage, echo is unacceptable, etc.), fall back to separate audio devices:

**Speaker output:**
```
PCM5100 OUTL ──┤10µF├── 8Ω Speaker ── GND
```

**Microphone input:**
```
Mic Breakout OUT ──┤10µF├── PCM1808 VINL
Mic Breakout VCC ── 3.3V
Mic Breakout GND ── GND
```

This gives clean, echo-free bidirectional audio using a small speaker and an
electret mic breakout (MAX9814 or MAX4466). The phone is still used for all
signaling (hook, dial, DTMF) — only voice I/O moves to external devices.

**Additional parts for fallback:**

| Qty | Component | Value / Part | Notes |
|-----|-----------|-------------|-------|
| 1 | Electret mic breakout | MAX9814 or MAX4466 | With built-in amplifier |
| 1 | Small speaker | 8Ω, 0.25–0.5W | Or use 3.5mm jack from Sub-A |
| 1 | Capacitor | 10µF non-polarized | AC coupling mic to ADC |

**Test:**
1. Go off-hook → hear dial tone through phone earpiece (or speaker if fallback)
2. During a BT call, speak into handset mic (or external mic) → far end hears you
3. Far-end speech comes through the earpiece (or speaker)

**Firmware references:**
- `main/config/audio_config.h` — sample rate 8000, buffer size 1024
- `main/audio/audio_output.c` — I2S channel config, tone generation, volume 0.2
- `main/audio/tones.c` — tone frequency and cadence definitions

---

### Module 3: Ring Indicator (Piezo Buzzer or LED)

**GPIO 13** — `PIN_RING_COMMAND` (output)

**How incoming calls work:** The paired cell phone notifies the ESP32 of an
incoming call via Bluetooth HFP. The firmware drives GPIO 13 to activate the
ringer. In v2.0, this drives the SLIC ring command for 90V AC ringing. In v1.0,
it drives a piezo buzzer or LED.

The phone's built-in ringer won't work here — electronic ringers need ~40-90V AC,
which we don't have. The piezo buzzer provides the audible ring alert instead.

The ring command is **active LOW**.

**Wiring (PNP transistor — matches active-LOW directly):**

```
        3.3V
         │
       Piezo Buzzer (+)
         │
    ┌────┤ 1N4148 (flyback diode, cathode at top)
    │    │
    │  Piezo Buzzer (−)
    │    │
    │    ├── 2N3906 Emitter ── 3.3V
    │    │
    │    │   2N3906 Base ── 1kΩ ── GPIO 13
    │    │
    │    └── 2N3906 Collector ── Buzzer (−) ── GND
    │
    └── (diode anode also connects to collector)
```

GPIO 13 LOW → PNP base pulled low → transistor ON → buzzer sounds.

**Visual indicator:** Add an LED + 330Ω in parallel with the buzzer.

#### Option B: LED-Only (Simplest)

Skip the buzzer and transistor. Connect an LED + 330Ω from 3.3V to GPIO 13.
GPIO 13 LOW → current flows → LED lights. Flashing visual ring indicator with
zero extra components beyond the LEDs already needed for Module 5.

**Ring cadence:** Standard North American: 2 seconds on, 4 seconds off.

**Test:** Call the paired cell phone from another phone. Buzzer sounds (or LED
flashes) 2s on / 4s off until answered or caller hangs up.

**Firmware references:**
- `main/config/pin_assignments.h:12` — `PIN_RING_COMMAND` = GPIO 13
- `main/hardware/slic_interface.h` — ring command support

---

### Module 4: Ring Detect Feedback

**GPIO 33** — `PIN_RING_DETECT` (input)

The firmware already knows about incoming calls from the Bluetooth HFP event —
GPIO 33 is a **feedback signal** that confirms ringing is actually happening.
In production, the SLIC RD pin confirms 90V AC is on the line. In v1.0, a
loopback from GPIO 13 confirms the ring command is being driven.

#### Option A: Direct Loopback (Recommended)

```
GPIO 13 ────── GPIO 33
```

When firmware commands ringing, it reads back ring-active status as confirmation.

#### Option B: Manual Override

```
GPIO 33 ──┤ ├── GND
        (SPST toggle)
```

Enable internal pull-up on GPIO 33. Allows testing ring detection independently —
useful for testing ring-trip behavior (going off-hook during ringing) without an
actual incoming call.

**Test:** During an incoming BT call, verify GPIO 33 reads LOW when ringing is
active, HIGH when ringing stops or call is answered.

**Firmware references:**
- `main/config/pin_assignments.h:11` — `PIN_RING_DETECT` = GPIO 33
- `main/hardware/slic_interface.h` — ring detect support

---

### Module 5: Dial Lamp, Status LEDs, and User Button

Straightforward GPIO connections.

#### Dial Lamp — GPIO 27

```
GPIO 27 ── 330Ω ──|>|── GND
                  (LED)
```

Lights when handset is off-hook (firmware drives GPIO 27 HIGH).

#### Status LED — GPIO 2

```
GPIO 2 ── 330Ω ──|>|── GND
                 (LED)
```

General status (e.g., BT connected). GPIO 2 also drives the ESP32 DevKitC
on-board LED on most boards.

#### UI LED — GPIO 18

```
GPIO 18 ── 330Ω ──|>|── GND
                  (LED)
```

Additional status (e.g., call active, incoming ring).

#### User Button — GPIO 4

```
3.3V ── (internal pull-up)
              │
           GPIO 4 ──┤ ├── GND
                  (tactile button)
```

Button pressed = GPIO 4 reads LOW. Used for pairing initiation, factory reset, etc.

**Test:** Verify each LED lights for its expected condition. Verify button press
triggers the expected action.

**Firmware references:**
- `main/config/pin_assignments.h:24–26` — `PIN_DIAL_LAMP_CTRL` = GPIO 27, `PIN_STATUS_LED` = GPIO 2, `PIN_UI_LED` = GPIO 18, `PIN_USER_BUTTON` = GPIO 4

---

## Breadboard Layout

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Full-Size Breadboard                         │
│                                                                     │
│  ┌──────────────────────┐    ┌────────────────────────────────────┐ │
│  │   SUB-ASSEMBLY A     │    │     v1.0 MODULES                   │ │
│  │                      │    │                                    │ │
│  │  ESP32 DevKitC       │    │  Phone loop (330Ω + dividers)      │ │
│  │    (center span)     │    │                                    │ │
│  │                      │    │  Voice audio coupling               │ │
│  │  PCM5100 DAC         │    │    (caps + resistors)              │ │
│  │    (breakout board)  │    │                                    │ │
│  │                      │    │  Buzzer + PNP driver                │ │
│  │  PCM1808 ADC         │    │    (ring indicator)                │ │
│  │    (breakout board)  │    │                                    │ │
│  │                      │    │  LEDs × 4 + resistors              │ │
│  │                      │    │    (ring, lamp, status ×2)         │ │
│  └──────────────────────┘    │                                    │ │
│                              │  User button                       │ │
│                              └────────────────────────────────────┘ │
│                                                                     │
│       ┌──────────────────────────────────────────┐                  │
│       │  Corded phone via RJ11 or screw terminal  │                 │
│       │  (TIP = green, RING = red, off-board)     │                 │
│       └──────────────────────────────────────────┘                  │
│                                                                     │
│  ═══════════════════ Power Rails (5V / 3.3V / GND) ═══════════════  │
└─────────────────────────────────────────────────────────────────────┘
```

**Placement tips:**
- Keep the ESP32 centered so GPIO pins are accessible on both sides
- Place audio coupling components near the codecs to keep analog runs short
- Run **both** 5V (from ESP32 USB/VIN pin) and 3.3V on the breadboard power
  rails — the phone loop uses 5V, everything else uses 3.3V
- Phone connects via two wires (TIP/RING) — strip an RJ11 cable or use screw
  terminals at the breadboard edge

---

## Firmware Test Matrix

Each row is a firmware feature to test. The Module column shows which v1.0
module exercises it.

| Firmware Feature | Module | GPIO / Bus | Test Procedure | Expected Result |
|---|---|---|---|---|
| Off-hook detection | 1: Phone Loop | GPIO 32 | Lift handset | Serial log: state change, web UI updates |
| Dial tone on off-hook | 1 + 2 | GPIO 32, I2S | Lift handset | 350Hz + 440Hz continuous through earpiece |
| Off-hook warning | 1 + 2 | GPIO 32, I2S | Leave off-hook >30s without dialing | 1400Hz + 2060Hz, 0.1s on/off through earpiece |
| Pulse dial digits | 1: Phone Loop | GPIO 34 | Set to P mode, dial a digit | Firmware decodes correct digit (1–10 pulses = 1–0) |
| DTMF dial digits | 1: Phone Loop | I2S ADC | Set to T mode, press key | Goertzel decodes DTMF from on-wire tones |
| Outgoing call via BT | 1 + 2 | GPIO 32, I2S, BT | Lift handset, dial number, wait | Ringback tone (440+480Hz, 2s/4s), then call connects |
| Incoming call ring | 3: Buzzer | GPIO 13 | Call the paired phone from another | Buzzer sounds 2s on / 4s off |
| Ring trip (answer) | 1 + 3 | GPIO 32, 13 | Lift handset during ringing | Ringing stops, call connects, audio bridged |
| Ring detect loopback | 4: Feedback | GPIO 33, 13 | Incoming call triggers ring | GPIO 33 reads LOW when ring active |
| Voice TX (to far end) | 1 + 2 | I2S ADC, BT | Active call, speak into handset | Far end hears you via phone mic → ADC → BT |
| Voice RX (from far end) | 1 + 2 | I2S DAC, BT | Active call, far end speaks | Hear far-end through phone earpiece via BT → DAC → loop |
| Call disconnect | 1 | GPIO 32, BT | Hang up during call | Call ends, BT HFP hangs up |
| Busy signal | 2 | I2S | Call a busy number | 480Hz + 620Hz, 0.5s on/off through earpiece |
| Reorder tone | 2 | I2S | Dial invalid number | 480Hz + 620Hz, 0.25s on/off (fast busy) |
| Congestion tone | 2 | I2S | Network congestion | 480Hz + 620Hz, 0.2s on / 0.3s off |
| Confirmation tone | 2 | I2S | Feature activation | 350Hz + 440Hz, 0.1s beeps |
| Call waiting tone | 2 | I2S | Second incoming call | 440Hz single, 0.3s beep |
| SIT tone | 2 | I2S | Intercept condition | 950Hz + 1400Hz, 0.274s |
| Stutter dial tone | 2 | I2S | Voicemail waiting | 350Hz + 440Hz, 0.1s on/off stutter |
| Dial lamp | 5: LED | GPIO 27 | Lift handset | LED lights |
| Status LED | 5: LED | GPIO 2 | Pair BT, connect | LED reflects BT status |
| UI LED | 5: LED | GPIO 18 | Various states | LED reflects call/ring status |
| User button | 5: Button | GPIO 4 | Press button | Triggers pairing or reset action |
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

## What v1.0 CAN'T Do

These require the v2.0 SLIC build:

- **Ring the phone's internal ringer.** Electronic ringers need 40-90V AC. The
  piezo buzzer / LED substitutes for v1.0.
- **Drive a vintage rotary phone.** Western Electric 500s, Trimlines, and other
  vintage phones need −48V loop current and proper SLIC impedance matching.
- **Perfect echo cancellation.** Simple AC coupling produces echo on the far end.
  Firmware AEC or a transformer hybrid (see Module 2 notes) will improve this.
  The v2.0 SLIC has a proper hybrid built in.
- **SLIC loop current regulation.** The 5V/330Ω loop is fixed — no dynamic
  current regulation or polarity reversal.
- **Line protection.** No MOV, PTC fuse, or isolation needed at 5V.
- **Ring generator.** No 90V AC sine wave generation or cadence control.
- **−48V / +12V power management.**

Everything else — the complete call lifecycle, state machine, BT HFP audio,
Goertzel DTMF detection, pulse decoding, tone generation, event system,
web interface — works fully at 5V with a real phone in your hand.

---

## GPIO Quick Reference

All pin assignments from `main/config/pin_assignments.h`:

| GPIO | Define | Direction | v1.0 Connection |
|------|--------|-----------|-----------------|
| 2 | `PIN_STATUS_LED` | Output | LED + 330Ω → GND |
| 4 | `PIN_USER_BUTTON` | Input | Tactile button → GND (internal pull-up) |
| 5 | `PIN_PCM_CLK_OUT` | Output | I2S BCLK → codecs (Sub-A) |
| 13 | `PIN_RING_COMMAND` | Output | PNP transistor → piezo buzzer |
| 18 | `PIN_UI_LED` | Output | LED + 330Ω → GND |
| 25 | `PIN_PCM_FSYNC` | Output | I2S LRCLK → codecs (Sub-A) |
| 26 | `PIN_PCM_DOUT` | Output | I2S data out → DAC (Sub-A) |
| 27 | `PIN_DIAL_LAMP_CTRL` | Output | LED + 330Ω → GND |
| 32 | `PIN_OFF_HOOK_DETECT` | Input | Phone loop voltage divider (hook sense) |
| 33 | `PIN_RING_DETECT` | Input | Loopback from GPIO 13 or switch |
| 34 | `PIN_PULSE_DIAL_IN` | Input | Phone loop voltage divider (pulse sense) |
| 35 | `PIN_PCM_DIN` | Input | I2S data in ← ADC (Sub-A) |
| 39 | `PIN_DTMF_IN` | Input | Not used (Goertzel via I2S instead) |

---

## Appendix: Standalone Simulator Modules

If you don't have a corded phone (or want granular control over individual
signals for debugging), these standalone modules replicate Modules 1 signaling
functions using discrete components. They can be used in any combination — e.g.,
the phone for hook and DTMF, but the 555 timer for precisely timed pulses.

### A.1: Hook Switch Simulator (replaces phone hook switch)

**GPIO 32** — `PIN_OFF_HOOK_DETECT` (input, internal pull-up)

```
GPIO 32 ──┤ ├── GND
        (SPST toggle)
```

- Switch OPEN → HIGH → on-hook
- Switch CLOSED → LOW → off-hook
- No external pull-up needed (firmware enables internal pull-up)

### A.2: 555 Pulse Dial Simulator (replaces phone pulse dial)

**GPIO 34** — `PIN_PULSE_DIAL_IN` (input-only)

555 in astable mode generates ~10Hz pulses. A momentary button gates oscillation.

**Timing:** R1 = 39kΩ, R2 = 68kΩ, C = 1µF → ~8.3Hz (close enough to 10 pps).

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

**Gating:** Pin 4 (RESET) to 3.3V through momentary pushbutton, with 10kΩ
pull-down. Press and hold to generate pulses; release to stop.

**Manual alternative:** Skip the 555. Momentary pushbutton between GPIO 34 and
GND with 10kΩ pull-up to 3.3V. Tap at ~10Hz.

### A.3: DTMF via Smartphone + Mic (replaces phone DTMF IC)

No extra hardware — uses the electret mic breakout and PCM1808 ADC from the
fallback audio setup (Module 2, Option B).

1. Install a DTMF tone generator app on a smartphone
2. Go off-hook to enter dialing mode
3. Hold smartphone near the electret mic and play DTMF tones
4. Firmware Goertzel algorithm decodes digits from the audio stream

Less reliable than on-wire DTMF from a real phone, but works with zero additional
hardware.

### Standalone Simulator Parts

| Qty | Component | Value / Part | Module | Notes |
|-----|-----------|-------------|--------|-------|
| 1 | SPST toggle switch | Panel or breadboard mount | A.1: Hook | |
| 1 | NE555 timer IC | DIP-8 | A.2: Pulse | |
| 1 | 8-pin DIP socket | | A.2: Pulse | Optional |
| 1 | Resistor | 39kΩ ¼W | A.2: Pulse | 555 timing R1 |
| 1 | Resistor | 68kΩ ¼W | A.2: Pulse | 555 timing R2 |
| 1 | Capacitor | 1µF ceramic or film | A.2: Pulse | 555 timing C |
| 1 | Capacitor | 10µF electrolytic | A.2: Pulse | 555 decoupling |
| 1 | Momentary pushbutton | | A.2: Pulse | Gating / trigger |
| 1 | Electret mic breakout | MAX9814 or MAX4466 | A.3: DTMF | Also needed for Module 2 fallback |
