# 2026-11-10: Call tones, speaker ring and hook switch on the MAX98357A rig

**Version:** v1 (bench) · **Section:** call tones / ringing / hook
**Firmware:** `21fb507` → `a92cf05` (PRs #18, #19, #20) · **Hardware:** [MAX98357A bench rig](../wiring/max98357a-tone-test.md), as wired today

## Goal

Hear the call-progress tones from the firmware for the first time, then ring for an incoming call
and drive the line from a hook switch, all on the MAX98357A rig before the codecs and the line
interface exist.

## Setup

- ESP32 dev board, MAX98357A breakout on USB 5V, small speaker. I2S on GPIO 5 / 25 / 26.
- A toggle switch from GPIO 32 to GND as the hook switch (added for the third part).
- Cell phone: Galaxy S22, paired over HFP.
- Built and flashed on the Mac.

## What happened

**1. Tones on command (`21fb507`, PR #18).** Nothing in the firmware played a tone, so dialing
`*1`–`*9` on the cell phone now picks one: the gateway reads the dialed number (AT+CLCC), hangs
the call up while it is still dialing, and plays the tone. Hearing them turned up three faults:

- **Every tone sounded rough, with an image near 4 kHz.** In 16-bit mono mode the ESP32's I2S sends
  each pair of samples swapped (written 1,2,3,4, sent 2,1,4,3; see "16 bit mono" in the ESP-IDF I2S
  docs). `audio_output` now swaps each pair back before writing, for tones and Bluetooth audio.
- **The S22's calls had no audio bridge.** The bridge started only on CVSD audio. The S22 opens
  wideband (mSBC) audio, so it got no bridge and the BT RX ring buffer overflowed for the whole
  call. The bridge now starts on either.
- **Dial tone rattled the speaker** at 0.2 of full scale. Lowered by ear with `*7` to −17 dBFS.

**2. Speaker ring (`1142128`…`26d938d`, PR #19).** An incoming call plays an electronic ringer on
the speaker: a 1000 / 1250 Hz warble switching 20 times a second, 2 s on / 4 s off. The first
build failed (`2994bf8`: a leftover `phase1` / `phase2` after the oscillator moved into a struct).
Dial tone still sounded slightly crispy at −17 dBFS, so the default dropped to **−20 dBFS**
(`62f880e`).

**3. Hook switch (`22d619e`, PR #20).** The new line module (`main/app/line/line.c`) acts as the
central office: lifting the handset answers a ringing call, replacing it hangs up, and with no call
it plays dial tone, then reorder after 15 s, then the howler after 30 s more. Faults found:

- **Answering a ringing call reset the board.** Hook events run every subscriber on the hook task,
  whose stack was 2KB; answering used about 2,056 bytes, corrupted a spinlock, and the board reset.
  The stack is now 4KB.
- **A stopped tone kept sounding.** With nothing written, the I2S DMA replayed its last buffers.
  The channel now sends silence (`auto_clear_after_cb`).
- **The hook debounce took the first edge** after 50 ms of quiet. It now takes a change once the
  input has held 50 ms, and reports a handset already lifted at boot.
- The speaker rang with the handset lifted. The ringer now rings only on-hook: lifting stops it, and
  replacing the handset during an incoming call starts it.
- A call ending cleared the off-hook bit. Now only the switch sets or clears it.
- An answered call could read as a lost one, because ringing cleared before in-call was set. The
  order is now in-call first.

## Results

Taken from the commits. Measured values (scope, meter) weren't recorded this session.

| Check | Expected | Measured | Pass? |
|---|---|---|---|
| Tones `*1`–`*6` | Clean tones at the right cadence | Rough until the pair swap; clean after | ✅ after fix |
| Tone level | Clean on the bench speaker | −20 dBFS at 9 dB amp gain | ✅ |
| Incoming call | Warble ring, 2 s on / 4 s off | Rings | ✅ |
| Lift while ringing | Ring stops, call answered | Board reset (stack overflow); works at 4KB | ✅ after fix |
| Hang up during a call | Call ends on the cell phone | Ends | ✅ |
| Lift with no call | Dial tone → reorder → howler | | |
| Tone stops on hang-up | Silence | Kept sounding (DMA replay); stops after fix | ✅ after fix |
| BCLK / LRCLK on the scope | 256 kHz / 8 kHz | not measured | |

## Takeaways

- The ESP32's 16-bit mono I2S pair swap will hit the PCM5100 path too. The fix is in
  `audio_output`, so it already applies.
- Any subscriber to hook events runs on the hook task's 4KB stack. Keep callbacks short, and hand
  longer work to a task, as the line module does.
- Phones may open mSBC rather than CVSD. Test the audio bridge with both.
- An answered call connects but the caller isn't heard: call audio isn't wired to the speaker.
- **Promote?** The pair swap and the mSBC start are design facts worth a line in the audio section
  of the project site.

## Next

- Run the bring-up checks on the [wiring page](../wiring/max98357a-tone-test.md#bring-up-checks)
  with the scope and fill in the measured column.
- Call audio: the caller heard on the speaker.
