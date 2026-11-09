# Tone test: ESP32 + MAX98357A I2S amp

**Status:** not built yet. This is a bench rig for hearing the call-progress tones (dial tone, busy
signal, ringback) before the PCM5100 DAC and the line interface are built. Update it to what was
actually built.

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
amp is playing. At the firmware's tone level (`TONE_VOLUME 0.2`) the amp draws little either way.

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

Nothing in the firmware calls `audio_output_play_tone()` yet. Off-hook detection will do that
later. For this bench test, add a temporary loop at the end of `app_main()` in
[`main/main.c`](../../main/main.c) (after `audio_output_init()` has run) and **don't commit it**:

```c
#include "audio/audio_output.h"   // with the other includes

// BENCH ONLY: cycle the call-progress tones on the MAX98357A
while (1) {
    ESP_LOGI(TAG, "Bench: dial tone");
    audio_output_play_tone(DIAL_TONE);       // 350 + 440 Hz, steady
    vTaskDelay(pdMS_TO_TICKS(5000));

    ESP_LOGI(TAG, "Bench: busy");
    audio_output_play_tone(BUSY_SIGNAL);     // 480 + 620 Hz, 0.5 s on / 0.5 s off
    vTaskDelay(pdMS_TO_TICKS(6000));

    ESP_LOGI(TAG, "Bench: ringback");
    audio_output_play_tone(RINGBACK_TONE);   // 440 + 480 Hz, 2 s on / 4 s off
    vTaskDelay(pdMS_TO_TICKS(12000));        // two full ring cycles

    audio_output_stop_tone();
    vTaskDelay(pdMS_TO_TICKS(2000));
}
```

The serial monitor shows `Playing tone: N` from `audio_output` each time a tone changes.

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
