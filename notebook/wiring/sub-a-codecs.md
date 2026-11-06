# Sub-Assembly A: ESP32 + PCM5100 DAC + PCM1808 ADC

**Status:** not built yet. Wiring below is as planned in the
[prototyping build guide](../../impl/prototyping-build-guide.md#sub-assembly-a-esp32--pcm-audio-codecs-breadboard);
update it to what was actually built.

**Firmware pin map (authoritative):** [`main/config/pin_assignments.h`](../../main/config/pin_assignments.h).
I2S runs at 8 kHz (`AUDIO_SAMPLE_RATE` in [`main/config/audio_config.h`](../../main/config/audio_config.h)),
with no MCLK pin assigned.

---

## ⚠️ Check against the datasheets before wiring

Three PCM1808 details in the build guide (and in `line-interface-hc5504b.rst`) look wrong to
me. Confirm each against the TI PCM1808 datasheet and record the answer here:

- [ ] **System clock (SCKI).** As I understand it, the PCM1808 has no internal PLL. It needs a
  system clock on SCKI (256, 384 or 512 × fs) in slave mode as well as master. The build guide
  doesn't wire SCKI, and the firmware sets `.mclk = GPIO_NUM_NC`. The ESP32 can output MCLK on
  GPIO 0, 1 or 3. *(The PCM5100's SCK tied to GND for PLL mode is fine; it's only the ADC.)*
- [ ] **Supplies.** I believe the PCM1808 has a separate analog supply, VCC at 5V, beside the
  3.3V digital VDD. The guide powers it from 3.3V only.
- [ ] **Mode pins.** The guide sets MD0 = VDD and MD1 = GND and calls it slave mode. Check the
  datasheet's MD1/MD0 table: I recall 00 as slave and the other three as master at different
  clock ratios.
- [ ] **Sample rate.** Check that the PCM1808 runs at 8 kHz. If its minimum is higher, run I2S at
  16 kHz and resample for HFP's 8 kHz.

None of these affects the DAC side. The firmware can play tones through the PCM5100 while the ADC
is sorted out.

---

## Power

| From | To | Wire |
|---|---|---|
| ESP32 3V3 | Breadboard +3.3V rail | Yellow |
| ESP32 GND | Breadboard GND rail | Black |
| +3.3V rail | PCM5100 breakout VDD | Yellow |
| GND rail | PCM5100 breakout GND | Black |
| +3.3V rail | PCM1808 breakout VDD (digital) | Yellow |
| *see check above* | PCM1808 breakout VCC (analog) | |
| GND rail | PCM1808 breakout GND / AGND | Black |

## I2S bus

| ESP32 | Signal | PCM5100 (DAC) | PCM1808 (ADC) | Wire |
|---|---|---|---|---|
| GPIO 25 | LRCLK / frame sync | LRCK | LRCK | White |
| GPIO 5 | BCLK / bit clock | BCK | BCK | White |
| GPIO 26 | Data out | DIN | — | Green |
| GPIO 35 | Data in | — | DOUT | Green |
| *TBD (GPIO 0/1/3)* | MCLK | — | SCKI (*see check above*) | White |

Fan LRCLK and BCLK out from a common tie point near the ESP32, with short runs.

## Strapping, soldered on the breakouts

**PCM5100:**

| Pin | Tied to | Why |
|---|---|---|
| XSMT | 10kΩ to VDD | Unmuted |
| FMT | GND | I2S format |
| SCK | GND | PLL mode, clock from BCK |
| VDD | 0.1µF + 10µF to GND | Decoupling |

**PCM1808:**

| Pin | Tied to | Why |
|---|---|---|
| FMT0, FMT1 | GND | I2S, 24-bit |
| MD0 | VDD per the guide (*check*) | Mode |
| MD1 | GND per the guide (*check*) | Mode |
| VINL | 10kΩ to AGND | Input bias |
| VDD | 0.1µF + 10µF to GND | Decoupling |

## Analog test points

- PCM5100 OUTL → 10µF coupling cap → 3.5mm jack tip (sleeve to GND). This is the first place to hear dial tone.
- PCM1808 VINL: open until the microphone is wired (v1 bench stand-ins).

## Bring-up checks

From the build guide. Log the results in a dated entry.

| # | Check | Expected |
|---|---|---|
| 1 | 3.3V rail | 3.3V ±0.1V |
| 2 | Codec VDD pins | 3.3V ±0.1V at each IC |
| 3 | Flash, serial monitor | Boots, no BT queue errors |
| 4 | BCLK (GPIO 5) | 256 kHz square wave (8 kHz × 32) |
| 5 | LRCLK (GPIO 25) | 8 kHz square wave |
| 6 | 440 Hz through the DAC | Heard on the jack, seen on the scope |
| 7 | BT test call | Far end heard on the jack |
| 8 | Speak during the call | I2S data on GPIO 35 |
