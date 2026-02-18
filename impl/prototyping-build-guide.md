# Ma Bell Gateway — Breadboard/Perfboard Prototyping Build Guide

This guide walks through building the Ma Bell Gateway hardware prototype as three
independent sub-assemblies. Each sub-assembly has its own parts checklist, wiring
order, and test points. Build them in order — each one builds on the last.

**Build order:**

| Sub-Assembly | What It Does | Voltage | Board Type |
|---|---|---|---|
| **A** — ESP32 + Audio Codecs | Digital audio over Bluetooth HFP | 3.3V only | Breadboard |
| **B** — SLIC (HC-5504B) | Telephone line interface | 3.3V, 12V, −48V | Breadboard + perfboard |
| **C** — Ring Generator (LT1684) | 90V AC ringer drive | 48V → 90V AC | Dedicated perfboard |

---

## Wire Color Convention

Use consistent wire colors throughout all sub-assemblies:

| Color | Signal |
|---|---|
| **Red** | High voltage (48V, 90V) |
| **Orange** | 12V |
| **Yellow** | 3.3V |
| **Black** | GND |
| **Blue** | Digital signals (GPIO, I2S clock) |
| **Green** | I2S data / audio signals |
| **White** | I2S clock (BCLK, LRCLK) |

---

## Sub-Assembly A: ESP32 + PCM Audio Codecs (Breadboard)

This sub-assembly gets you firmware development immediately. Once built, you can
pair via Bluetooth HFP, generate dial tones, and test bidirectional audio — all
without any high-voltage hardware.

### Parts Checklist

| Qty | Component | Value / Part | Notes |
|-----|-----------|-------------|-------|
| 1 | ESP32 dev board | ESP32-WROVER DevKitC | Must be WROVER (PSRAM) for BT Classic |
| 1 | DAC | PCM5100PWR (TSSOP-20) | Solder to TSSOP-20 breakout board |
| 1 | ADC | PCM1808PWR (TSSOP-20) | Solder to TSSOP-20 breakout board |
| 2 | TSSOP-20 breakout boards | Generic, with pin headers | Pre-solder IC + passives before breadboarding |
| 2 | Decoupling caps (DAC) | 0.1µF ceramic (X7R) + 10µF MLCC/tantalum | Near PCM5100 VDD pin |
| 2 | Decoupling caps (ADC) | 0.1µF ceramic (X7R) + 10µF MLCC/tantalum | Near PCM1808 VDD pin |
| 1 | Pull-up resistor | 10kΩ 1% metal film | PCM5100 XSMT → 3.3V (unmute) |
| 1 | Bias resistor | 10kΩ 1% metal film | PCM1808 VINL → AGND |
| 2 | AC coupling caps | 10µF non-polarized film or X7R MLCC, 16V+ | Between codec analog and SLIC (later) |
| 1 | Full-size breadboard | 830 tie points | Main build platform |
| 1 | 3.5mm headphone jack (TRS) | Panel mount or breadboard-friendly | Test point for DAC output |
| — | Jumper wires | Various lengths | Male-to-male for breadboard |
| 1 | Pin header strip | 1×8 male, 0.1" pitch | Inter-board interface (for Sub-B later) |

### Codec Breakout Board Prep (Do Before Breadboarding)

Solder the following onto each TSSOP-20 breakout board at your bench before
inserting into the breadboard. Breadboards and fine-pitch soldering don't mix.

**PCM5100 (DAC) breakout — solder these:**

| Breakout Pin | PCM5100 Pin | Connection | Purpose |
|---|---|---|---|
| VDD | VDD | 0.1µF ceramic to GND pad (on breakout) | Decoupling |
| VDD | VDD | 10µF to GND pad (on breakout) | Bulk decoupling |
| XSMT | XSMT | 10kΩ pull-up to VDD pad (on breakout) | System unmute (HIGH = unmuted) |
| FMT | FMT | Wire to GND pad (on breakout) | I2S standard format |
| SCK | SCK | Wire to GND pad (on breakout) | PLL mode (generates system clock internally) |

**PCM1808 (ADC) breakout — solder these:**

| Breakout Pin | PCM1808 Pin | Connection | Purpose |
|---|---|---|---|
| VDD | VDD | 0.1µF ceramic to GND pad (on breakout) | Decoupling |
| VDD | VDD | 10µF to GND pad (on breakout) | Bulk decoupling |
| FMT0 | FMT0 | Wire to GND pad | I2S format, 24-bit |
| FMT1 | FMT1 | Wire to GND pad | I2S format, 24-bit |
| MD0 | MD0 | Wire to VDD pad | Slave mode |
| MD1 | MD1 | Wire to GND pad | Slave mode |
| VINL | VINL | 10kΩ to AGND pad (on breakout) | Input bias resistor |

### Breadboard Placement

```
┌─────────────────────────────────────────────────────────────────────┐
│  Full-Size Breadboard (830 pts)                                     │
│                                                                     │
│  [+3.3V rail] ════════════════════════════════════════════════════  │
│  [GND rail]   ════════════════════════════════════════════════════  │
│                                                                     │
│  ┌─────────────────┐     ┌──────────────┐    ┌──────────────┐      │
│  │  ESP32 DevKitC  │     │  PCM5100     │    │  PCM1808     │      │
│  │                 │     │  DAC breakout│    │  ADC breakout│      │
│  │  (straddles     │     │              │    │              │      │
│  │   center gap)   │     │              │    │              │      │
│  └─────────────────┘     └──────────────┘    └──────────────┘      │
│                                                                     │
│                                    ┌──────────────┐                 │
│                                    │ Inter-board   │  ← Pin header  │
│                                    │ interface     │    strip at     │
│                                    │ (for Sub-B)   │    board edge   │
│                                    └──────────────┘                 │
│                                                                     │
│  [+3.3V rail] ════════════════════════════════════════════════════  │
│  [GND rail]   ════════════════════════════════════════════════════  │
└─────────────────────────────────────────────────────────────────────┘
```

### Wiring Order

Wire in this exact sequence. Test power before connecting signals.

**Step 1 — Power rails:**

1. Connect ESP32 3.3V output pin → breadboard +3.3V rail
2. Connect ESP32 GND pin → breadboard GND rail
3. Connect PCM5100 breakout VDD → +3.3V rail
4. Connect PCM5100 breakout GND → GND rail
5. Connect PCM1808 breakout VDD → +3.3V rail
6. Connect PCM1808 breakout GND → GND rail

**Step 2 — I2S bus (shared clock signals):**

| ESP32 GPIO | Signal | Connects To |
|---|---|---|
| **GPIO 25** | LRCLK / Frame Sync | PCM5100 LRCK **and** PCM1808 LRCK |
| **GPIO 5** | BCLK / Bit Clock | PCM5100 BCK **and** PCM1808 BCK |
| **GPIO 26** | DOUT (data to DAC) | PCM5100 DIN |
| **GPIO 35** | DIN (data from ADC) | PCM1808 DOUT |

Wire LRCLK and BCLK as short, direct runs. Fan out from ESP32 to both codecs
at a common tie point near the ESP32.

**Step 3 — Analog test points:**

- PCM5100 OUTL → 10µF AC coupling cap → 3.5mm headphone jack tip (jack sleeve → GND)
- PCM1808 VINL → left disconnected for now (bias resistor already on breakout)
- Leave AC coupling caps accessible — these will connect to SLIC RX/TX in Sub-B

**Step 4 — Inter-board interface header:**

Install a 1×8 pin header strip at the breadboard edge. Assign these signals
(label with tape):

| Header Pin | Signal | Purpose |
|---|---|---|
| 1 | DAC OUT (post AC-coupling cap) | Audio to SLIC RX (Pin 21) |
| 2 | ADC IN (pre AC-coupling cap) | Audio from SLIC TX (Pin 24) |
| 3 | GPIO 32 | SHD — off-hook detect input |
| 4 | GPIO 33 | RD — ring detect input |
| 5 | GPIO 13 | RC — ring command output |
| 6 | 3.3V | Logic power for Sub-B |
| 7 | GND | Common ground |
| 8 | GND | Common ground (redundant for low impedance) |

### Test Points — Sub-Assembly A

Run these checks before moving to Sub-B. You need a multimeter and ideally an
oscilloscope.

| # | Test | Expected Result | Tool |
|---|------|----------------|------|
| 1 | Measure 3.3V rail | 3.3V ±0.1V | Multimeter |
| 2 | Measure codec VDD pins | 3.3V ±0.1V at each IC | Multimeter |
| 3 | Flash firmware, check serial output | Boot messages, no BT queue errors | Serial monitor |
| 4 | Measure BCLK (GPIO 5) | 256 kHz square wave (8kHz × 32 bits) | Oscilloscope |
| 5 | Measure LRCLK (GPIO 25) | 8 kHz square wave | Oscilloscope |
| 6 | Play 440Hz sine via DAC | 440Hz tone on headphone jack | Oscilloscope / headphones |
| 7 | Pair BT, make test call | Audio from phone heard on headphone jack | Phone + headphones |
| 8 | Speak into phone during call | ADC DOUT (GPIO 35) shows I2S data | Oscilloscope |

**If BCLK/LRCLK are absent:** Check that PCM5100 SCK pin is tied to GND (PLL mode) and XSMT is HIGH.

---

## Sub-Assembly B: SLIC Integration — HC-5504B (Breadboard + Perfboard)

This sub-assembly adds the telephone line interface. After building it, you can
pick up a rotary phone handset, hear dial tone, and make calls over Bluetooth.

### Why Two Boards?

- **Breadboard**: SLIC IC, digital interface, low-voltage decoupling — easy to modify
- **Perfboard** (~3×5 cm): Feed resistors, AC coupling caps, RJ11 jack, line protection — soldered for reliable high-current connections

### Parts Checklist

**SLIC Breadboard:**

| Qty | Component | Value / Part | Notes |
|-----|-----------|-------------|-------|
| 1 | SLIC IC | HC-5504B-5 (DIP-24) | Intersil/Renesas |
| 1 | DIP-24 socket | Standard IC socket | Optional but recommended |
| 1 | Timing cap C3 | 4.7µF electrolytic/ceramic, 16V+ | Pin 5 — loop current timing |
| 1 | Filter cap C2 | 2.2µF electrolytic/ceramic, 16V+ | Pin 17 — ring trip/ground key |
| 1 | Balance cap C4 | 0.1µF (100nF) ceramic | Pin 22 — longitudinal balance |
| 1 | Decoupling cap (VB+) | 100nF ceramic (X7R) | Pin 4 — within 0.5" of pin |
| 1 | Bulk cap (VB+) | 10µF electrolytic/ceramic, 16V+ | Pin 4 — within 1" of pin |
| 1 | Decoupling cap (VB−) | 10µF electrolytic, **63V rated** | Pin 11 — −48V supply |
| 1 | Breadboard | Half-size or full-size | **Separate** from Sub-A breadboard |
| — | Jumper wires | Various | To Sub-A interface header |

**Analog/Line Perfboard:**

| Qty | Component | Value / Part | Notes |
|-----|-----------|-------------|-------|
| 2 | Feed resistors | 150Ω, **1% tolerance**, 1/4W metal film | Pin 9 (TIP), Pin 10 (RING) — precision critical |
| 2 | AC coupling caps | 10µF non-polarized film or X7R MLCC, 16V+ | Codec↔SLIC audio path |
| 1 | RJ11 jack | 6P4C modular jack | Green=TIP, Red=RING |
| 2 | Fuses | 0.5A slow-blow, 250V | Littelfuse 0217.500 or equiv |
| 2 | Fuse holders | PCB mount | For easy replacement |
| 1 | Bridge rectifier | DB107 (1A, 1000V) | Reverse polarity protection |
| 1 | MOV | V150LA10A (150V) | Overvoltage/surge protection |
| 1 | Perfboard | ~3×5 cm | Soldered assembly |
| — | Hookup wire | 22 AWG solid | Board-to-board connections |

### SLIC Breadboard Wiring Order

> **Safety first:** The SLIC breadboard handles −48V DC. Use a current-limited
> bench supply. Label all −48V connections with red tape.

**Step 1 — Star ground (most important connection):**

Connect Pins 23 (AG), 6 (DG), and 12 (BG) to a **single common ground point**
on the breadboard. Use short, direct wires. This is the star ground — all other
GND connections on this board reference this point.

```
    SLIC Pin 23 (AG) ──┐
                        ├──► Single GND tie point ──► System GND
    SLIC Pin 6  (DG) ──┤
                        │
    SLIC Pin 12 (BG) ──┘
```

**Step 2 — VB+ power (+12V):**

1. Connect +12V supply → SLIC Pin 4 (VB+)
2. Place 100nF ceramic cap between Pin 4 and star GND (within 0.5")
3. Place 10µF cap between Pin 4 and star GND (within 1")

**Step 3 — VB− power (−48V):**

1. Connect −48V supply → SLIC Pin 11 (VB−)
2. Place 10µF **63V** electrolytic between Pin 11 and star GND
3. **Double-check polarity** — negative leg of cap to Pin 11, positive to GND

> ⚠ **Polarity warning:** On the bench supply providing −48V, the **positive**
> terminal connects to **system GND** and the **negative** terminal connects to
> SLIC Pin 11. Getting this backwards will destroy the ESP32 and SLIC.

**Step 4 — Timing/filter capacitors:**

| SLIC Pin | Cap | Value | Connection |
|---|---|---|---|
| Pin 5 (C3) | Loop timing | 4.7µF, 16V+ | Pin 5 → star GND |
| Pin 17 (C2) | Ring trip filter | 2.2µF, 16V+ | Pin 17 → star GND |
| Pin 22 (C4) | Longitudinal balance | 0.1µF ceramic | Pin 22 → star GND |

**Step 5 — Digital interface jumpers (to Sub-A):**

Run jumper wires from the SLIC breadboard to the Sub-A inter-board header:

| SLIC Pin | Function | Direction | ESP32 GPIO | Header Pin |
|---|---|---|---|---|
| Pin 13 (SHD) | Off-hook detect | SLIC → ESP32 | GPIO 32 (input) | 3 |
| Pin 8 (RD) | Ring relay driver | SLIC → ESP32 | GPIO 33 (input) | 4 |
| Pin 16 (RC) | Ring command | ESP32 → SLIC | GPIO 13 (output) | 5 |

Also connect:
- Header Pin 6 (3.3V) → nothing needed yet (SLIC digital pins are 3.3V-compatible)
- Header Pin 7 (GND) → SLIC star GND
- Header Pin 8 (GND) → SLIC star GND

**Step 6 — Unused SLIC pins:**

| SLIC Pin | Name | Connection |
|---|---|---|
| Pin 7 (RS) | Ring sync | Tie to +3.3V or +5V (not used) |
| Pin 15 (PD) | Power denial | Leave unconnected (not used) |
| Pin 14 (GKD) | Ground key detect | Leave unconnected (not used) |
| Pin 18 (OUT) | Op-amp output | Leave unconnected |
| Pin 19 (−IN) | Op-amp inverting input | Leave unconnected |
| Pin 20 (+IN) | Op-amp non-inverting input | Leave unconnected |

### Analog Perfboard Assembly

Solder this small board at your bench. It handles the feed resistors, audio
coupling, RJ11 jack, and line protection.

**Layout (approximate):**

```
┌──────────────────────────────────────────────┐
│  Analog / Line Interface Perfboard (~3×5cm)  │
│                                              │
│  [RJ11 Jack]                                 │
│   TIP ──[F1 0.5A]──┬──[DB107]──┬── TIP out  │
│                     │           │            │
│   RING──[F2 0.5A]──┴──[MOV  ]──┴── RING out │
│                       150V                   │
│                                              │
│  TIP out ──[150Ω 1%]── to SLIC Pin 9 (TF)   │
│  RING out──[150Ω 1%]── to SLIC Pin 10 (RF)  │
│                                              │
│  SLIC Pin 1 (TIP)  ── wire to TIP out       │
│  SLIC Pin 2 (RING) ── wire to RING out      │
│                                              │
│  SLIC Pin 24 (TX) ──[10µF]── to ADC VINL    │
│  SLIC Pin 21 (RX) ──[10µF]── from DAC OUTL  │
│                                              │
│  [Terminal block or pin header for wires]     │
└──────────────────────────────────────────────┘
```

**Audio path connections (back to Sub-A):**

- SLIC Pin 24 (TX) → 10µF AC coupling cap → Sub-A Header Pin 2 (ADC IN) → PCM1808 VINL
- SLIC Pin 21 (RX) ← 10µF AC coupling cap ← Sub-A Header Pin 1 (DAC OUT) ← PCM5100 OUTL

> **Level matching note:** The PCM5100 DAC output is ~2V RMS full scale. The SLIC
> RX input may need a voltage divider (e.g., 10kΩ/10kΩ) to avoid clipping. Test
> audio quality first — if distorted, add a divider between the AC coupling cap
> and SLIC Pin 21.

### Power-Up Sequence

**Always power up in this order:**

1. **3.3V first** — ESP32 and codecs (via USB or Sub-A)
2. **+12V second** — SLIC VB+ (bench supply output 1)
3. **−48V last** — SLIC VB− (bench supply output 2, current-limited to 100mA)

**Shutdown: reverse order** (−48V off first, 3.3V last).

### Test Points — Sub-Assembly B

| # | Test | Expected Result | Tool |
|---|------|----------------|------|
| 1 | Measure −48V across TIP/RING | −48V DC (phone on-hook) | Multimeter |
| 2 | SHD pin (Pin 13 / GPIO 32) with phone on-hook | HIGH (3.3V) | Multimeter |
| 3 | SHD pin with phone off-hook | LOW (0V) | Multimeter |
| 4 | Loop current (series ammeter in TIP) | 20–80 mA when off-hook | Multimeter |
| 5 | Dial tone through handset earpiece | 350Hz + 440Hz continuous | Ear / scope on SLIC TX |
| 6 | Speak into handset mic, check ADC | Audio data on GPIO 35 (I2S DIN) | Oscilloscope |
| 7 | BT call, listen through handset | Clear bidirectional audio | Phone call |
| 8 | RD pin (Pin 8 / GPIO 33) | HIGH on-hook, LOW during ring command | Multimeter |

**If SHD doesn't change on hook lift:** Check −48V polarity, verify feed resistors
(150Ω) are in place between Pins 9/10 and TIP/RING, confirm star ground.

**If no audio through handset:** Check XSMT is HIGH on PCM5100, verify AC coupling
caps are in circuit, check for ground loops (add ferrite bead if hum is present).

---

## Sub-Assembly C: Ring Generator — LT1684 (Dedicated Perfboard)

This sub-assembly generates the 90V AC 20Hz signal needed to ring vintage
electromechanical telephone bells. **Build this last** — you only need it for
incoming calls.

> ⚠ **DANGER: 90V AC can cause painful shock and cardiac arrest at sufficient
> current. All high-voltage work on dedicated perfboard only. Discharge all
> capacitors before touching. Use insulated probes. One-hand rule when measuring.**

### LT1684 Key Facts

The LT1684 is a micropower ring tone generator IC. Critical things to understand
before building:

- **14-pin DIP** (LT1684CN) — **not** 16-pin as listed in `bom.rst`. Confirm
  with your datasheet when the part arrives.
- **Does NOT generate 20Hz internally** — requires external sine-weighted PWM
  from the ESP32 on its differential inputs (IN A / IN B)
- **Active Tracking Supply** — GATE+ and GATE− pins drive external N-channel and
  P-channel MOSFETs that create floating ±10V supply rails, allowing the IC to
  "ride" on the output signal and linearly generate signals hundreds of volts in
  amplitude
- **MFB filter** — internal 2nd-order multiple-feedback Butterworth lowpass
  (~100Hz cutoff, ~40dB gain) converts PWM to clean sine wave
- **VCC** — the IC self-generates its own supply rails (~±14V referenced to
  ATREF) via the Active Tracking Supply; you provide the high-voltage DC rail
  through the external MOSFETs and transformer

**Pin summary (from datasheet — verify against your actual part):**

| Pin | Name | Function |
|-----|------|----------|
| 1 | IN B | PWM negative input (via 100pF isolation cap) |
| 6 | GATE− | P-channel MOSFET gate drive (self-biases to ~−14V ref ATREF) |
| 7 | ATREF | Active tracking supply reference (connect to OUT) |
| 8 | OUT | Ring tone output — also reference for internal functions |
| 11 | GATE+ | N-channel MOSFET gate drive (self-biases to ~+14V ref ATREF) |
| 12 | AMPIN | Output amplifier input (via ~5kΩ protection resistor) |
| 13 | BGOUT | Buffered PWM output (±1.25V normalized, drives MFB filter) |
| 14 | IN A | PWM positive input (via 100pF isolation cap) |

> **Action item:** Download the LT1684 datasheet PDF to `datasheets/` and verify
> the complete pinout, recommended MOSFETs, transformer specification, and exact
> MFB filter component values (RF3=680kΩ is confirmed; other values need
> datasheet lookup). The datasheet is 16 pages and contains a complete application
> circuit.

### Parts Checklist

| Qty | Component | Value / Part | Notes |
|-----|-----------|-------------|-------|
| 1 | Ring generator IC | LT1684CN (14-pin DIP) | Analog Devices — **verify pin count on arrival** |
| 1 | 14-pin DIP IC socket | Standard | Never solder IC directly |
| 1 | N-channel power MOSFET | Per LT1684 datasheet | GATE+ driver — high voltage rated (≥200V VDS) |
| 1 | P-channel power MOSFET | Per LT1684 datasheet | GATE− driver — high voltage rated (≥200V VDS) |
| 1 | Step-up transformer | Per LT1684 datasheet | Turns ratio for 90V output from 48V supply |
| 1 | MFB filter resistor RF3 | 680kΩ | Confirmed from datasheet — sets gain |
| — | MFB filter components | RF1, RF2, CF2, CF3 per datasheet | 2nd-order Butterworth, ~100Hz cutoff |
| 2 | PWM isolation caps | 100pF ceramic | IN A and IN B input isolation |
| 2 | MOSFET gate resistors | Per LT1684 datasheet | Pull-up/pull-down for GATE+/GATE− |
| — | Decoupling caps | 0.1µF ceramic + bulk per datasheet | Near IC supply pins |
| 1 | Ring relay | DPDT, 12V coil — Omron G5V-2-H1-DC12 | Switches line between −48V and ring gen |
| 1 | Relay driver transistor | 2N2222A (or PN2222A) | NPN, drives relay coil |
| 1 | Relay base resistor | 1kΩ, 1/4W | Limits Q1 base current |
| 1 | Flyback diode | 1N4148 | Across relay coil — **mandatory** |
| 1 | Output fuse | 100mA fast-blow | Ring circuit protection |
| 1 | Perfboard | ~5×7 cm minimum | Dedicated HV board — no breadboard |
| 2 | Terminal blocks | 2-position screw terminal | Power in, signal out |
| — | Hookup wire | 22 AWG solid, silicone insulated preferred | HV-rated insulation |

### Important: Everything on Perfboard

**No breadboard for any part of this sub-assembly.** Breadboard contact resistance
causes unreliable high-voltage operation and creates shock hazards from exposed
conductors. Solder everything.

### Wiring Order

**Step 0 — Plan your layout on paper first.**

Sketch component placement on graph paper before soldering anything. Keep the
relay and its driver at one end, the LT1684 and MOSFETs in the center, and the
transformer and filter components at the other end. Route high-voltage traces
away from low-voltage signals. Maintain ≥2mm clearance between high-voltage
traces.

**Step 1 — IC socket + decoupling:**

1. Solder 14-pin DIP socket for LT1684 (do NOT insert IC yet)
2. Solder decoupling caps as close to IC socket as possible
3. Wire GND connections

**Step 2 — Active Tracking Supply (MOSFETs + transformer):**

1. Mount N-channel MOSFET (GATE+ driven, Pin 11)
2. Mount P-channel MOSFET (GATE− driven, Pin 6)
3. Solder gate resistors (pull-up for GATE+, pull-down for GATE−)
4. Mount step-up transformer
5. Wire MOSFET drains to transformer primary
6. Wire ATREF (Pin 7) to OUT (Pin 8)

> Consult LT1684 datasheet Figure 1 (Typical Application) for exact topology.
> The Active Tracking Supply creates floating supply rails — the MOSFETs act as
> voltage-following level shifters referenced to the output signal.

**Step 3 — MFB filter:**

1. Solder RF3 (680kΩ) and remaining filter components (RF1, RF2, CF2, CF3)
2. Wire BGOUT (Pin 13) → filter input
3. Wire filter output → AMPIN (Pin 12) via ~5kΩ protection resistor
4. Connect filter output to OUT (Pin 8)

**Step 4 — PWM input:**

1. Solder 100pF isolation caps to IN A (Pin 14) and IN B (Pin 1)
2. Run wires from isolation caps to terminal block (for ESP32 PWM connection)
3. ESP32 will drive these with sine-weighted PWM — complementary outputs on two
   GPIOs (to be assigned — not currently in `pin_assignments.h`)

> **PWM note:** The ESP32 LEDC or MCPWM peripheral generates the sine-weighted
> PWM. This is NOT a simple square wave — the duty cycle must follow a sinusoidal
> envelope at 20Hz to produce a clean sine output from the MFB filter.

**Step 5 — Relay + driver circuit:**

```
                              +12V
                                │
                              ┌─┴─┐
                              │   │ Relay Coil
                              │   │ (DPDT, 12V — K1)
                              └─┬─┘
                                │
                  D1 (1N4148)   │
                 ┌────┤◄├───────┤   Flyback diode
                 │              │
                 │            ┌─┴─┐
                 │            │ C │
                 │      ┌─────┤   │ Q1 (2N2222A)
                 │      │     │ E │
                 │      │     └─┬─┘
                 │      │       │
  SLIC RD ──────┴─[1kΩ]┴───────┴─── GND
  (Pin 8, via jumper from Sub-B)
```

1. Solder relay K1, driver Q1, base resistor (1kΩ), flyback diode (1N4148)
2. Wire relay coil to +12V and Q1 collector
3. Wire Q1 base (via 1kΩ) to terminal block (connects to SLIC RD / GPIO 33)
4. Wire Q1 emitter to GND

**Relay contact wiring:**

- NC (Normally Closed) contacts: −48V battery feed (normal on-hook state)
- NO (Normally Open) contacts: Ring generator 90V AC output
- Common contacts: To SLIC TIP/RING (via Sub-B perfboard)

When SLIC RD goes LOW (ring active), Q1 turns on, relay energizes, line switches
from −48V to 90V AC ring signal.

**Step 6 — Output protection:**

1. Solder 100mA fast-blow fuse on ring generator output
2. Optional: MOV (150V) across relay NO contacts for transient suppression

**Step 7 — Terminal blocks and connections:**

Wire terminal blocks for:
- 48V DC power input (from bench supply)
- 12V power input (for relay coil — from Sub-B supply or separate)
- PWM input from ESP32 (2 wires: IN A, IN B)
- SLIC RD signal input (1 wire from Sub-B)
- Ring output to Sub-B relay contacts (2 wires: to TIP/RING)
- GND (common with Sub-A and Sub-B)

### Test Points — Sub-Assembly C

Test in stages. **Do not apply full voltage until low-voltage tests pass.**

**Stage 1 — Relay driver only (no LT1684, no HV):**

| # | Test | Expected Result | Tool |
|---|------|----------------|------|
| 1 | Apply 3.3V to base resistor (simulating RD LOW) | Relay clicks, contacts switch | Ear / multimeter on contacts |
| 2 | Remove 3.3V | Relay releases | Ear |
| 3 | Measure flyback spike (optional) | Clamped by 1N4148 (~0.7V above 12V) | Oscilloscope |

**Stage 2 — LT1684 at reduced voltage:**

1. Insert LT1684 into socket
2. Apply reduced supply voltage (e.g., 24V instead of 48V)
3. Drive PWM inputs with ESP32 sine-weighted PWM at 20Hz
4. Measure output — should see reduced-amplitude 20Hz sine

| # | Test | Expected Result | Tool |
|---|------|----------------|------|
| 4 | PWM on IN A/IN B | 20Hz signal on oscilloscope at OUT pin | Oscilloscope |
| 5 | GATE+ voltage | ~+14V above ATREF | Multimeter |
| 6 | GATE− voltage | ~−14V below ATREF | Multimeter |

**Stage 3 — Full voltage:**

1. Increase supply to 48V
2. Verify 90V RMS (127V peak) 20Hz sine on output

| # | Test | Expected Result | Tool |
|---|------|----------------|------|
| 7 | Output voltage (RMS) | ~90V RMS | Multimeter (AC) |
| 8 | Output frequency | 20Hz | Oscilloscope |
| 9 | Connect actual phone ringer | Bell rings | Phone |
| 10 | Ring trip test (lift handset during ring) | Ringing stops within 200ms | Phone + scope |

---

## −48V Bench Supply Strategy

For prototyping, use a dual-output bench supply instead of the (deferred) −48V
DC-DC converter.

### Setup

```
    Bench Supply Output 1          Bench Supply Output 2
    ┌─────────────────┐            ┌─────────────────┐
    │  +12V           │            │  +48V            │
    │  Current: 200mA │            │  Current: 100mA  │  ← Current limit!
    │                 │            │                  │
    │  (+) ──► +12V rail           │  (+) ──► System GND  ← Yes, positive = GND
    │  (−) ──► GND                 │  (−) ──► SLIC VB−   ← This is −48V
    └─────────────────┘            └─────────────────┘
```

### Critical Details

- **Output 2 is wired "backwards"**: The positive terminal of the −48V supply
  connects to **system GND**. The negative terminal connects to SLIC Pin 11.
  This creates −48V referenced to system ground.
- **Getting this backwards destroys the ESP32, codecs, and SLIC.**
- **Current limit:** Set to 100–150mA. Normal off-hook draws 20–80mA. If the
  supply current-limits immediately, check for shorts or reversed polarity.

### Power-Up Sequence (Full System)

| Order | Supply | Voltage | What It Powers |
|-------|--------|---------|----------------|
| 1st | USB / 3.3V | 3.3V | ESP32 + audio codecs (Sub-A) |
| 2nd | Bench Output 1 | +12V | SLIC VB+ (Sub-B), relay coils (Sub-C) |
| 3rd | Bench Output 2 | −48V | SLIC VB− / telephone line (Sub-B) |

**Shutdown: reverse order** (−48V first, 3.3V last).

---

## Final Integration

### Physical Layout

Keep sub-assemblies physically separated. The ring generator produces EMI that
can couple into the audio codecs.

```
    ┌──────────┐        jumper wires       ┌──────────┐       jumper wires      ┌──────────┐
    │  Sub-A   │◄─────────────────────────►│  Sub-B   │◄────────────────────────►│  Sub-C   │
    │  ESP32   │   8-pin inter-board       │  SLIC    │   relay contacts +      │  Ring    │
    │  + Codecs│   header                  │  + Line  │   RD signal             │  Gen     │
    │          │                           │  Interface│                         │  (HV!)   │
    │ 3.3V only│                           │ 3.3/12/48│                         │ 48→90V   │
    └──────────┘                           └──────────┘                         └──────────┘

    ◄── LOW VOLTAGE ──────────────────────────────────────── HIGH VOLTAGE ──►
    Keep Sub-C as far from Sub-A as practical (≥12 inches / 30cm)
```

### Integration Step 1: Sub-A ↔ Sub-B

Connect Sub-A and Sub-B via the inter-board header. Test these functions:

| # | Test | Expected Result |
|---|------|----------------|
| 1 | Phone on-hook, check GPIO 32 | HIGH (SHD = on-hook) |
| 2 | Lift handset | GPIO 32 → LOW, dial tone in earpiece |
| 3 | Pair ESP32 with mobile phone via BT | "MA BELL" appears, pairs with PIN 0000 |
| 4 | Dial number on mobile, answer on handset | Bidirectional audio through handset |
| 5 | Make call from handset (dial on mobile for now) | Audio both directions |
| 6 | Hang up handset | GPIO 32 → HIGH, call disconnects |

### Integration Step 2: Add Sub-C

Connect Sub-C relay contacts and RD signal to Sub-B. Test incoming call flow:

| # | Test | Expected Result |
|---|------|----------------|
| 1 | Incoming call via BT | ESP32 drives GPIO 13 LOW (RC) |
| 2 | SLIC RD goes LOW | Relay clicks, line switches to ring gen |
| 3 | Phone rings | Electromechanical bell rings at 20Hz |
| 4 | Ring cadence | 2s ON, 4s OFF |
| 5 | Lift handset during ring (ring trip) | Ringing stops, call connects |
| 6 | Full call flow: ring → answer → talk → hang up | All states transition correctly |

### Final Verification Checklist

- [ ] Off-hook detection works reliably (no bouncing/false triggers)
- [ ] Dial tone plays immediately on hook lift (~350Hz + 440Hz)
- [ ] Bluetooth pairing and reconnection works
- [ ] Outgoing call: audio clear in both directions
- [ ] Incoming call: phone rings with correct cadence (2s on / 4s off)
- [ ] Ring trip: ringing stops promptly when handset lifted
- [ ] On-hook disconnect: call ends when handset replaced
- [ ] No audible hum or noise on audio path
- [ ] ESP32 doesn't reset during ring generator operation
- [ ] −48V supply current stays within expected range (20–80mA off-hook)

---

## Common Gotchas & Debugging Tips

### Audio Issues

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| No audio output from DAC | XSMT pin LOW (muted) | Verify 10kΩ pull-up to 3.3V |
| No I2S clocks | SCK not grounded | PCM5100 SCK pin must be tied to GND for PLL mode |
| 60Hz hum in audio | Ground loop | Use star grounding; add ferrite bead on GND between Sub-A and Sub-B |
| Distorted audio to phone | DAC output too hot for SLIC RX | Add 10kΩ/10kΩ voltage divider between AC coupling cap and SLIC Pin 21 |
| Audio only in one direction | AC coupling cap missing or wrong pin | Verify: DAC OUTL → 10µF → SLIC Pin 21 (RX), SLIC Pin 24 (TX) → 10µF → ADC VINL |

### SLIC Issues

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| SHD doesn't change on hook lift | No loop current | Check −48V polarity, verify 150Ω feed resistors on Pins 9/10 |
| −48V supply current-limits immediately | Short or reversed polarity | Disconnect, check all wiring, verify bench supply polarity |
| SHD floats or is noisy | Missing pull-up | Enable ESP32 internal pull-up on GPIO 32 |
| No voltage on TIP/RING | VB− not connected | Check Pin 11 connection and −48V supply |

### Ring Generator Issues

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| No output signal | PWM is plain square wave | Must be sine-weighted PWM (duty cycle follows sinusoidal envelope) |
| Output frequency wrong | PWM frequency wrong | ESP32 PWM must produce 20Hz fundamental after MFB filtering |
| Weak or no ringing | MFB filter values wrong | Verify RF3=680kΩ and other values against datasheet |
| Relay chatters | Insufficient base drive | Check 1kΩ base resistor, verify 12V supply to relay coil |
| Phone doesn't ring | Relay wired backwards | Verify NC=−48V (normal), NO=90V AC (ring), Common=line |

### Power Issues

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| ESP32 resets randomly | Ground bounce from HV switching | Add 100µF+ electrolytic on 3.3V rail near ESP32 |
| Intermittent failures on breadboard | Contact resistance on power paths | Use multiple parallel breadboard rows for power; solder power connections |
| ESP32 destroyed on first power-up | −48V polarity reversed | New ESP32 needed; triple-check bench supply wiring before next attempt |
| 12V rail sags during ring | Relay coil inrush | Use separate 12V supply for relay or add 100µF bulk cap on 12V rail |

---

## Notes & Open Items

### Datasheet Actions

- [ ] Download LT1684 datasheet PDF from [Analog Devices](https://www.analog.com/media/en/technical-documentation/data-sheets/1684f.pdf) to `datasheets/`
- [ ] Verify LT1684 pin count (14-pin DIP expected, BOM says DIP-16) — update `bom.rst` if needed
- [ ] Extract exact MOSFET part numbers from LT1684 datasheet application circuit
- [ ] Extract transformer specification (turns ratio, inductance, part number)
- [ ] Extract complete MFB filter BOM (RF1, RF2, CF2, CF3 values)
- [ ] Confirm VCC arrangement — IC self-supplies via Active Tracking Supply
- [ ] Extract MOSFET gate resistor values

### Design Decisions Deferred

- **−48V DC-DC converter selection** — using bench supply for prototyping
- **ESP32 PWM pin assignment for LT1684** — needs two GPIO pins for differential
  sine-weighted PWM (IN A / IN B). Not yet in `pin_assignments.h`.
- **Ring trip detection** — the SLIC handles this automatically via RFS (Pin 3),
  but firmware integration needs testing

### BOM Discrepancy

The project BOM (`docs/source/implementation/bom.rst`) lists:

> U5 — LT1684CN (**DIP-16**)

The LT1684 is actually a **14-pin** package (both DIP and SOIC). This should be
corrected after confirming with the physical part or datasheet.

---

## Quick Reference: GPIO Pin Map

Authoritative source: `main/config/pin_assignments.h`

| GPIO | #define | Function | Direction | Sub-Assembly |
|------|---------|----------|-----------|-------------|
| 25 | PIN_PCM_FSYNC | I2S LRCLK / Frame Sync | Output | A |
| 5 | PIN_PCM_CLK_OUT | I2S BCLK / Bit Clock | Output | A |
| 26 | PIN_PCM_DOUT | I2S Data Out (to DAC) | Output | A |
| 35 | PIN_PCM_DIN | I2S Data In (from ADC) | Input | A |
| 32 | PIN_OFF_HOOK_DETECT | SLIC SHD — off-hook detect | Input | B |
| 33 | PIN_RING_DETECT | SLIC RD — ring relay status | Input | B |
| 13 | PIN_RING_COMMAND | SLIC RC — ring command | Output | B |
| 27 | PIN_DIAL_LAMP_CTRL | Dial lamp relay/MOSFET | Output | B |
| 34 | PIN_PULSE_DIAL_IN | Rotary dial pulse input | Input | B |
| 2 | PIN_STATUS_LED | Status LED | Output | A |
| 4 | PIN_USER_BUTTON | User pushbutton | Input | A |
| TBD | — | LT1684 PWM IN A | Output | C |
| TBD | — | LT1684 PWM IN B | Output | C |
