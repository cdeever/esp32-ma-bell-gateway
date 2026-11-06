# Ma Bell Gateway — SLIC Module Build (v2.0)

This guide builds a real-SLIC Ma Bell Gateway using an off-the-shelf **AG1171 SLIC
module** (sold as the **KS0835F**, advertised "AG1171 / AG1170 compatible"). It gives
you a genuine 48V battery feed, proper 600Ω line impedance, real 2-wire ↔ 4-wire hybrid
audio, and enough ringing voltage to ring a vintage mechanical bell — all from a **single
+3.3V to +5V supply, with no high-voltage hardware anywhere**.

This is the **v2.0: SLIC Build** referenced in the
[low-voltage build guide](low-voltage-test-rig.md). It is also the **recommended
prototyping line interface** for the whole project: it replaces both Sub-Assembly B
(HC-5504B + discrete −48V/+12V rails) and Sub-Assembly C (the LT1684 90V AC ring
generator) from the [prototyping build guide](prototyping-build-guide.md) with a single
14-pin module. The HC-5504B remains the project's production / from-scratch reference
design — see `docs/source/implementation/circuit/line-interface-hc5504b.rst`.

**Prerequisite:** Sub-Assembly A from the
[prototyping build guide](prototyping-build-guide.md) (ESP32 + PCM5100 DAC + PCM1808 ADC
on breadboard with I2S wiring). This build adds the SLIC module and connects it to that
existing assembly.

---

## Why the AG1171 module

The Silvertel **Ag1171** is a complete single-channel SLIC in a 14-pin SIL package with an
**integral DC/DC converter**. From one +3.3–5V rail it internally generates the ~48V
battery feed and the ~65 Vrms / 20 Hz ringing voltage that conventional SLICs need
external −48V and 90V AC supplies to produce. The KS0835F is a low-cost clone of this part
on a small carrier board.

| | v1.0: Low-Voltage Build | **v2.0: AG1171 Module (this guide)** | Production: HC-5504B |
|---|---|---|---|
| **Line interface** | None — 5V DC loop, NPN current sense | AG1171/KS0835F SLIC module | HC-5504B discrete SLIC |
| **Supply rails** | 5V USB only | **Single +3.3–5V** | −48V, +12V, 90V AC |
| **Battery feed** | Fake 5V loop (~8.6mA) | Real ~48V feed, ~30mA constant | Real −48V feed |
| **Ringing** | Piezo buzzer / LED | **~65 Vrms 20Hz — rings a real bell** | LT1684 90V AC ring gen |
| **Hybrid** | AC-couple onto loop (echo) | Real 2-wire↔4-wire hybrid | Real SLIC hybrid |
| **HV hardware** | None | **None** | −48V DC-DC + ring gen board |
| **Vintage rotary phone** | No | **Yes** | Yes |

The big wins over the HC-5504B prototyping path: no −48V supply, no +12V supply, no LT1684
ring-generator board, no relay, no 90V AC anywhere. The big win over the v1.0 low-voltage
rig: a real SLIC with a real hybrid and enough ringing voltage to drive an
electromechanical bell, so a vintage Western Electric 500 works as it should.

> **Ratings (per Silvertel Ag1171 datasheet V1.6):** drives ~470Ω loops (≈1km of line
> with a 300Ω phone). Rings **2 phones with mechanical bells** or **3 with tone ringers**
> at ≥40 Vrms. A single vintage rotary phone is well within this.

---

## Parts Checklist

### Already Have (from Sub-Assembly A)

| Qty | Component | Notes |
|-----|-----------|-------|
| 1 | ESP32-WROVER DevKitC | Must be WROVER (PSRAM) for BT Classic |
| 1 | PCM5100PWR on TSSOP-20 breakout | DAC, with decoupling and pin strapping |
| 1 | PCM1808PWR on TSSOP-20 breakout | ADC, with decoupling and pin strapping |
| 1 | Full-size breadboard | 830 tie points |

### New for the SLIC Module Build

| Qty | Component | Value / Part | Notes |
|-----|-----------|-------------|-------|
| 1 | SLIC module | KS0835F (AG1171/AG1170 compatible) | The line interface |
| 1 | Telephone | Vintage rotary (WE 500) or any analog phone | Connects to module TIP/RING |
| 1 | Dedicated supply | Regulated +3.3V or +5V, **≥1A** | Ringing draws up to ~500mA — see warning |
| 1 | Coupling cap (VIN) | **10nF** ceramic | DAC → module audio in (datasheet C4) |
| 1 | Coupling cap (VOUT) | **100nF** ceramic | Module audio out → ADC (datasheet C3) |
| 1 | Bulk cap (supply) | 470µF, 16V electrolytic | Across +VPWR / GNDPWR (datasheet C2) |
| 1 | Bypass cap (supply) | 100nF ceramic | Across +VPWR / GNDPWR (datasheet C1) |
| 1 | RJ11 jack or screw terminals | 6P2C/6P4C | Green = TIP, Red = RING |
| — | Jumper wires | Various | To Sub-A and supply |

> Many KS0835F carrier boards already include the supply bypass caps and the ESD
> protection diodes from the datasheet's typical-application circuit. Inspect your board;
> only add the parts above that aren't already populated. Audio coupling caps (10nF/100nF)
> are usually **not** on the carrier and must be added externally.

> **The HC-5504B path's −48V DC-DC converter, +12V buck, LT1684, MOSFETs, ring
> transformer, ring relay, and relay driver are all eliminated.** The AG1171 module
> replaces every one of them.

---

## Module Pinout (AG1171 / KS0835F, 14-pin SIL)

| Pin | Name | Direction | Function |
|-----|------|-----------|----------|
| 1 | RING (B) | — | Subscriber line Ring (red) |
| 2 | TIP (A) | — | Subscriber line Tip (green) |
| 3 | F/R | Input | Forward/Reverse. **Toggle at 20–25 Hz to ring**; hold HIGH (forward) for normal calls |
| 4 | RM | Input | Ring Mode. Set **HIGH during ringing**, LOW otherwise (33% max duty cycle) |
| 5 | SHK | Output | Switch Hook. **HIGH = off-hook**; also pulses during loop-disconnect (pulse) dialing. Internal 3.3kΩ pull-up |
| 6–8 | NC | — | No connection |
| 9 | VIN | Input | Audio in from codec (DAC → line). Couple via **10nF** |
| 10 | VOUT | Output | Audio out to codec (line → ADC). Couple via **100nF** |
| 11 | NC | — | No connection |
| 12 | GNDPWR | — | DC/DC ground |
| 13 | +VPWR | — | +3.3V to +5V supply |
| 14 | PD | Input | Power-down. **Leave OPEN** for free-run. Logic-L powers down. **Never drive HIGH (damages the part)** |

---

## Wiring to the ESP32 (Sub-Assembly A)

The firmware's SLIC GPIOs (`main/config/pin_assignments.h`) were defined for the HC-5504B.
The AG1171 maps to them as follows:

| Module pin | ESP32 GPIO / `#define` | Direction | Notes |
|---|---|---|---|
| SHK (5) | GPIO 32 `PIN_OFF_HOOK_DETECT` **and** GPIO 34 `PIN_PULSE_DIAL_IN` | Module → ESP32 | One output drives both inputs — hook *and* pulse dialing come from SHK. See firmware note 1 below |
| RM (4) | GPIO 13 `PIN_RING_COMMAND` | ESP32 → Module | Ring-mode enable + 2s/4s cadence |
| F/R (3) | **needs a free GPIO** | ESP32 → Module | 20–25 Hz toggle during ring; tie HIGH otherwise. See firmware note 2 below |
| VIN (9) | PCM5100 OUTL via 10nF | DAC → Module | Receive path (far-end audio → earpiece) |
| VOUT (10) | PCM1808 VINL via 100nF | Module → ADC | Transmit path (mic → far end) |
| +VPWR (13) | Dedicated +3.3–5V supply | — | **Not** the ESP32 USB rail |
| GNDPWR (12) | Common GND | — | Tie to ESP32/codec ground |
| PD (14) | Leave open | — | Free-run |
| TIP (2) / RING (1) | RJ11 jack → phone | — | Green = TIP, Red = RING |

> ⚠ **Power: use a dedicated ≥1A supply, not the ESP32 USB rail.** Supply current is
> ~60–70mA on-hook, ~290–360mA off-hook, and **up to ~500mA during ringing** (datasheet
> §8.3). Powering the module from the ESP32's USB/3V3 rail will brown out the ESP32 when
> the phone goes off-hook or rings. Give the module its own regulated 3.3V or 5V supply and
> tie its ground to the common ground. Keep +VPWR/GNDPWR tracks short and thick, with the
> 470µF + 100nF caps right at the module.

### Audio path

The module's hybrid converts the 2-wire line to ground-referenced VIN/VOUT at 0 dB gain
(600Ω). Connect to the existing Sub-A codec analog points, replacing the v1.0 AC-coupling
onto the loop:

```
    PCM5100 OUTL ──┤10nF├── VIN  (pin 9)     receive: far-end / tones → phone earpiece
    PCM1808 VINL ──┤100nF├── VOUT (pin 10)    transmit: phone mic → far end
```

Because the AG1171 has a real hybrid, the bad far-end echo of the v1.0 AC-coupled build is
gone — no firmware AEC or transformer hybrid needed. If the PCM5100's ~2 Vrms full-scale
output overdrives VIN, attenuate in firmware (codec volume) rather than adding hardware.

---

## Ringing and Ring-Trip

Ringing is a host-driven sequence (datasheet §3.3, Figure 3):

1. To ring: set **RM HIGH** and **toggle F/R at 20–25 Hz** for the ring burst. The North
   American cadence is 2s on / 4s off (RM follows the cadence; F/R oscillates only while
   RM is high). RM duty cycle must stay ≤33%.
2. **Ring-trip (answer during ring):** when the phone goes off-hook, the module pulses
   **SHK**. Debounce SHK for 10ms, then drive **F/R permanently HIGH**, then drop **RM LOW**
   within >10ms and <500ms. The call is now connected; SHK stays HIGH while off-hook.

This is genuinely different from the HC-5504B, whose single RC pin enables ringing on its
own. The AG1171 needs the host to generate the F/R oscillation — see firmware note 2.

---

## Firmware Integration Notes

The current firmware targets the HC-5504B. Two deltas are needed to fully exercise the
AG1171 module. **This guide documents them; it does not implement them.**

**Note 1 — SHK polarity is inverted.** The firmware's `PIN_OFF_HOOK_DETECT` (GPIO 32) is
**active-LOW** (LOW = off-hook), matching the HC-5504B's SHD. The AG1171's SHK is
**active-HIGH** (HIGH = off-hook). Resolve one of two ways:

- *Firmware (recommended):* invert the off-hook read for this build (a config flag /
  `#define`, e.g. an "active-high hook" option in the SLIC interface code). Same for the
  pulse-dial decode on GPIO 34, which also reads SHK.
- *Hardware:* add a single inverter (e.g. one gate of a 74HC14, or an NPN common-emitter
  stage) between SHK and GPIO 32/34.

**Note 2 — Ringing needs an F/R oscillation.** The firmware drives only a simple ring-enable
on GPIO 13 (RC for the HC-5504B). The AG1171 additionally needs a 20–25 Hz square wave on
F/R while ringing. Options:

- *Firmware (recommended):* assign a free GPIO to F/R and toggle it at ~20 Hz (LEDC/timer)
  gated by the ring cadence on GPIO 13. Hold F/R HIGH when not ringing.
- *Hardware (no firmware change):* a 555 astable at ~20 Hz driving F/R, gated/enabled by
  the GPIO 13 ring command. F/R then oscillates only during the ring burst.

Until note 2 is addressed, everything except ringing the bell works (hook, dialing, dial
tone, bidirectional BT audio, call lifecycle). You can still confirm incoming-call handling
via the BT HFP event and the GPIO 13 ring command in the serial log.

---

## Build & Test Sequence

1. **Power the module alone.** Connect +VPWR/GNDPWR to the dedicated ≥1A supply with the
   470µF + 100nF caps. Leave PD open. Confirm the supply sits at 3.3–5V and the module
   draws ~60–70mA at idle (on-hook). No phone connected yet.
2. **Connect the phone.** Wire TIP/RING (pins 2/1) to the RJ11 jack → phone. On-hook, SHK
   (pin 5) reads LOW.
3. **Hook detect.** Lift the handset. SHK goes HIGH and module current rises to ~300mA.
   With firmware note 1 applied, the serial log shows off-hook and dial tone plays.
4. **Audio.** Wire VIN←10nF←PCM5100 OUTL and VOUT→100nF→PCM1808 VINL. Pair "MA BELL" (PIN
   0000), place a BT call, confirm clear bidirectional audio through the handset with no
   far-end echo.
5. **Dialing.** Rotary: pulse-dial decodes from SHK pulses (GPIO 34). DTMF: decodes via the
   Goertzel path on the ADC.
6. **Ringing (after firmware note 2).** Call the paired phone. Firmware raises RM (GPIO 13)
   on the 2s/4s cadence and toggles F/R at ~20 Hz → the bell rings. Lift the handset →
   ring-trip → call connects.

### Test Matrix

| Firmware feature | Signal | Test | Expected |
|---|---|---|---|
| Off-hook detection | SHK → GPIO 32 | Lift handset | Serial log off-hook; dial tone (note 1 applied) |
| Pulse dialing | SHK → GPIO 34 | Rotary-dial a digit | Firmware decodes digit from SHK pulses |
| DTMF dialing | VOUT → ADC | Tone-dial a digit | Goertzel decodes DTMF |
| Voice TX | mic → VOUT → ADC | Speak during call | Far end hears you, no echo |
| Voice RX | DAC → VIN → earpiece | Far end speaks | Heard in handset |
| Outgoing call | full path | Dial out, answer | Ringback then connect |
| Incoming ring | RM/F/R | Call paired phone | Bell rings 2s/4s (note 2 applied) |
| Ring trip | SHK during ring | Lift during ring | Ringing stops, call connects |
| On-hook disconnect | SHK | Hang up | Call ends |

---

## What This Build Still Can't Do (vs. the HC-5504B production design)

- **Off-premise line protection.** The AG1171 expects short on-premise loops with minimal
  ESD protection only — no power-cross / lightning protection. Fine on a bench; the
  production HC-5504B design includes MOV/PTC/bridge protection for field use.
- **Ring more than ~2 mechanical bells.** A single vintage phone is fine; a multi-phone
  party line is not.
- **Polarity/loop tuning.** Loop current is a fixed ~30mA and impedance is a fixed 600Ω,
  with no external programming — adequate for prototyping, not adjustable like the discrete
  HC-5504B feed network.

For everything else — full call lifecycle, real hybrid audio, real ringing of a vintage
bell, rotary and DTMF dialing — this single low-voltage module is the fastest, safest way
to a working Ma Bell Gateway with a real phone.

---

## GPIO Quick Reference

Authoritative source: `main/config/pin_assignments.h`

| GPIO | `#define` | AG1171 pin | Notes |
|------|-----------|-----------|-------|
| 5 | `PIN_PCM_CLK_OUT` | — | I2S BCLK → codecs (Sub-A) |
| 13 | `PIN_RING_COMMAND` | RM (4) | Ring-mode enable + cadence |
| 25 | `PIN_PCM_FSYNC` | — | I2S LRCLK → codecs (Sub-A) |
| 26 | `PIN_PCM_DOUT` | — | I2S data → DAC (Sub-A) |
| 32 | `PIN_OFF_HOOK_DETECT` | SHK (5) | Active-HIGH on AG1171 — see note 1 |
| 34 | `PIN_PULSE_DIAL_IN` | SHK (5) | Same SHK output; pulse dialing |
| 35 | `PIN_PCM_DIN` | — | I2S data ← ADC (Sub-A) |
| TBD | (assign for F/R) | F/R (3) | 20–25 Hz ring oscillation — see note 2 |

---

## References

- Silvertel **Ag1171** datasheet (V1.6, Oct 2011) — pinout, ringing sequence, electrical
  characteristics: <https://silvertel.com/images/datasheets/Ag1171-datasheet-Low-cost-ringing-SLIC-with-single-supply.pdf>
- Silvertel Ag1171 product page: <https://silvertel.com/ag1171/>
- Sphinx reference page: `docs/source/implementation/circuit/line-interface-ag1171-module.rst`
- HC-5504B production design: `docs/source/implementation/circuit/line-interface-hc5504b.rst`
