# Bench rig: ESP32 + MAX98357A I2S amp + hook switch

**Status:** built 2026-11-10 ([log](../log/2026-11-10-tones-ring-hook.md)). A bench rig for the
call-progress tones (dial tone, busy signal, ringback), the incoming-call ring and the hook switch,
before the PCM5100 DAC and the line interface are built. The wiring below is as planned; correct
anything that differs on the bench (SD and GAIN strapping, the 100µF).

**Firmware pin map (authoritative):** [`main/config/pin_assignments.h`](../../main/config/pin_assignments.h).
I2S runs at 8 kHz, 16-bit, Philips format, mono (`AUDIO_SAMPLE_RATE` in
[`main/config/audio_config.h`](../../main/config/audio_config.h); the channel setup is in
[`main/audio/audio_output.c`](../../main/audio/audio_output.c)). No MCLK. The MAX98357A doesn't
need one.

The MAX98357A is a class-D amp: I2S in, speaker out. It drives a 4–8Ω speaker directly, so
there's no jack and no coupling cap as on the PCM5100 path.

---

## Parts

- ESP32 dev board (pin names below are DevKitC-style; check your board's silkscreen)
- MAX98357A breakout (Adafruit-style: VIN, GND, SD, GAIN, DIN, BCLK, LRC, and the + / − speaker terminal)
- Small speaker, 4Ω or 8Ω, about 1–3W (8Ω is gentler on USB power)
- 100µF electrolytic for the 5V rail (optional, see Power)
- A toggle switch, standing in for the hook switch (see Hook switch). A jumper wire will do.

## Power

VIN takes 2.5–5.5V. Run it from the dev board's **5V** pin (USB 5V), not 3V3. That gives more
headroom, and the amp's current spikes stay off the ESP32's 3.3V regulator. The logic inputs
accept 3.3V levels at any VIN, so you don't need level shifting.

| From | To | Wire |
|---|---|---|
| ESP32 5V (VIN / VUSB) | MAX98357A VIN | Red |
| ESP32 GND | MAX98357A GND | Black |
| 100µF across VIN–GND at the amp (+ to VIN) | | |

The 100µF is cheap insurance against brownout resets when Bluetooth or WiFi transmit while the
amp is playing. At the firmware's tone level (`TONE_VOLUME_DEFAULT`, −20 dBFS) the amp draws little either way.

## I2S

| ESP32 | Signal | MAX98357A | Wire |
|---|---|---|---|
| GPIO 5 | BCLK / bit clock | BCLK | White |
| GPIO 25 | LRCLK / frame sync | LRC | White |
| GPIO 26 | Data out | DIN | Green |
| GPIO 35 | Data in | not used | |

Keep the clock and data runs short (under about 15cm). If the PCM5100 is on the same breadboard,
it can stay wired in parallel on the same three lines. Both chips only listen.

GPIO 5 is a strapping pin. The amp's BCLK input doesn't pull it, so boot isn't affected.

## Strapping pins

| Pin | Set to | Result |
|---|---|---|
| SD | **Wire to VIN** (>1.4V) | Amp on, plays the **left** channel |
| GAIN | Leave floating | 9 dB gain (the breakout's default) |

**SD (shutdown / channel select)** reads as a voltage: below 0.16V is shutdown, 0.16–0.77V plays
(L+R)/2, 0.77–1.4V plays right, and above 1.4V plays left. The Adafruit breakout ships with a 1MΩ
pull-up that lands in the (L+R)/2 band. That works too, but if the firmware's mono mode only fills
one slot, the average is 6 dB quieter. Tying SD to VIN removes that doubt. If you hear nothing,
try the default (SD unconnected) before anything else (see Troubleshooting).

**GAIN** options from the datasheet, if 9 dB is too loud or too quiet:

| GAIN pin | Gain |
|---|---|
| 100kΩ to GND | 15 dB |
| GND | 12 dB |
| floating | 9 dB |
| VIN | 6 dB |
| 100kΩ to VIN | 3 dB |

## Speaker

| MAX98357A | To |
|---|---|
| + | Speaker + |
| − | Speaker − |

⚠️ The output is bridge-tied (BTL): **neither terminal is ground**. Don't tie − to GND, and don't
clip a scope's ground lead to it, or you'll short the output. To scope the output, use two
channels on + and − with the probe grounds on board GND, and look at CH1 − CH2. A class-D output
is a ~300 kHz PWM, so expect fuzz on the scope unless you use the math channel with a low-pass, or
probe across the speaker after a simple RC filter. For the tones, BCLK, LRCLK and DIN on the
input side are the easier things to scope.

## Playing the tones

Lifting the handset plays dial tone, reorder and the howler in turn (see Hook switch). To play any
tone on its own, dial a test code on the paired cell phone and press call. The gateway reads the number the phone is
dialing (AT+CLCC), hangs the call up while it is still dialing, and plays the tone
([`main/audio/tone_test.c`](../../main/audio/tone_test.c), on by default with
`CONFIG_MA_BELL_TONE_TEST`):

| Dial | Tone |
|---|---|
| `*1` | Dial tone: 350 + 440 Hz, steady |
| `*2` | Ringback: 440 + 480 Hz, 2 s on / 4 s off |
| `*3` | Busy: 480 + 620 Hz, 0.5 s on / 0.5 s off |
| `*4` | Reorder (fast busy): 480 + 620 Hz, 0.25 s on / 0.25 s off |
| `*5` | Off-hook warning (howler): 1400 + 2060 Hz, 0.1 s on / 0.1 s off |
| `*6` | Stutter dial tone: 350 + 440 Hz, 0.1 s on / 0.1 s off |
| `*7` / `*8` | Quieter / louder by 3 dB (the tone keeps playing; back to the default at restart) |
| `*9` | Each of the above for 8 s, in turn |
| `*0` | Stop |

A tone plays until another code is dialed. The serial monitor shows `tone_test: *N: <name>` and
`Playing tone: N` from `audio_output`. Bluetooth must be connected.

## Incoming-call ring

The speaker also rings for an incoming call, with no new wiring: call the paired cell phone from
another phone. The gateway plays an electronic ringer, a 1000 / 1250 Hz warble switching 20 times
a second, 2 s on / 4 s off ([`main/audio/ringer.c`](../../main/audio/ringer.c), on by default with
`CONFIG_MA_BELL_SPEAKER_RING`). It stops when the handset is lifted (see Hook switch), when the
call is answered or declined on the cell phone, when the caller hangs up, or when Bluetooth
disconnects. It doesn't ring while the handset is lifted. The serial monitor shows
`ringer: Incoming call: ringing the speaker` and `ringer: Ringing stopped`. `*7` / `*8` set its
level too.

The cell phone keeps ringing as usual. Silence it there if the two together are too much.

## Hook switch

A toggle switch stands in for the phone's hook switch.

| ESP32 | Switch |
|---|---|
| GPIO 32 | One side |
| GND | The other side |

GPIO 32 has its internal pull-up on: open reads on-hook, closed (to GND) reads off-hook. This is
the same input the SLIC's SHD pin drives later, active low. A change counts once it has held for
50 ms. Use a toggle, not a momentary button: off-hook has to stay off-hook for the whole call.

The central office's side of the line is [`main/app/line/line.c`](../../main/app/line/line.c);
its timers are in [`main/config/line_config.h`](../../main/config/line_config.h).

| Do | Hear / see |
|---|---|
| Lift, no call | Dial tone; after 15 s, reorder; after 30 s more, the howler until on-hook |
| Lift with the cell phone not connected | Silence (a dead line); dial tone once it connects |
| Hang up | The tone stops |
| Call in, lift while it rings | The ring stops and the call is answered on the cell phone |
| Hang up during the call | The call ends on the cell phone |
| The caller hangs up first | 10 s of silence, then dial tone |
| A call arrives while lifted | No speaker ring; hang up and it rings |

The serial monitor shows `slic_if: Phone off-hook detected` / `on-hook detected` and each change of
the line, such as `line: on-hook -> dial tone`. Call audio isn't wired yet: an answered call is
connected, but the caller isn't heard on the speaker.

## Bring-up checks

Log the results in a dated entry.

| # | Check | Expected |
|---|---|---|
| 1 | Amp VIN | 4.75–5.25V (USB 5V) |
| 2 | SD pin | Same as VIN (or ~0.4–0.7V if left on the breakout's default pull-up) |
| 3 | Flash, serial monitor | Boots; `Audio I/O initialized (TX: GPIO26, RX: GPIO35)` |
| 4 | BCLK (GPIO 5) | 256 kHz square wave (8 kHz × 32) |
| 5 | LRCLK (GPIO 25) | 8 kHz square wave, ~50% duty |
| 6 | DIN (GPIO 26) during dial tone | Busy data stream; silence between tones |
| 7 | Dial tone | Steady, slightly beating hum (350 + 440 Hz), no clicks |
| 8 | Busy signal | 480 + 620 Hz, 60 interruptions per minute (0.5 s on / 0.5 s off) |
| 9 | Ringback | 440 + 480 Hz, 2 s on / 4 s off |
| 10 | During a BT reconnect or WiFi activity | No reset, no dropouts |
| 11 | Call the cell phone | Warble ring, 2 s on / 4 s off, starting with the cell phone's ring |
| 12 | Answer, decline, or caller hangs up | Ring stops within a fraction of a second, even mid-burst |
| 13 | Bluetooth off on the cell phone while ringing | Ring stops |

## Troubleshooting

- **Silence, clocks present:** check SD. Below 0.16V the amp is shut down. Then try SD unconnected
  (the L+R mix) in case the firmware's mono data lands in the right slot rather than the left.
- **Silence, no clocks:** look for `Failed to create I2S channels` in the log. `hardware_init()`
  routes the Bluetooth controller's PCM signals to the same three pins first; `audio_output_init()`
  should take them over for I2S right after.
- **Distortion or crackle:** lower GAIN (VIN = 6 dB) and check VIN under load. A sagging USB
  supply is the usual cause. Add the 100µF if you skipped it.
- **Pops at tone start/stop:** expected for now. The tone task starts and stops at full amplitude
  with no ramp.
- **Resets when the tone starts:** brownout. Use a better USB cable or port, and add the 100µF.
- **A tone keeps playing after it should stop:** the I2S channel must have `auto_clear_after_cb`
  set (`audio_output_init()`). Without it the DMA replays its last buffers.
- **The switch does nothing:** check that it goes to GPIO 32 and GND, and that the log shows
  `slic_if: Initial hook state: on-hook` with the switch open.
