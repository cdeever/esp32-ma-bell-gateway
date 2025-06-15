======================================
Input Detection and Line Signaling
======================================

The Ma Bell Gateway firmware relies on real-world physical and audio signals to determine user intent and call activity. These signals are not just passively observed—they directly **trigger state transitions** within the firmware's control logic.

This page outlines how each class of input—hook transitions, rotary pulses, DTMF tones, and incoming ring events—is used to advance the state machine described earlier. These inputs form the dynamic interface between vintage hardware and modern software-driven behavior.

Hook State Detection
====================

The firmware must reliably detect when the handset is lifted (off-hook) or placed back in the cradle (on-hook). This physical event acts as a gatekeeper for both initiating and terminating calls.

- **Off-Hook Transition**: Triggers dial tone and prepares for digit entry.
- **On-Hook Transition**: Cancels dialing, ends a live call, or returns to idle.
- **Debounce Considerations**: Mechanical transitions are noisy; filtering is conceptually required.

*→ Off-hook triggers transition from `Idle` to `Dial Tone`, or from `Ringing` to `Connected` during incoming calls.*

Rotary Pulse Dialing
=====================

Rotary phones encode digits by briefly breaking the line current a number of times equal to the digit being dialed. The firmware must count these “make-break” cycles and convert them to numeric values.

- **Digit Encoding**: 1 pulse = digit 1, ... 10 pulses = digit 0
- **Timing Constraints**: Pulses follow a predictable cadence (~10 pps), with short gaps between digits.
- **Challenges**: Missed or spurious transitions can corrupt digit interpretation.

*→ Pulse sequences push the firmware from `Dial Tone` to `Dialing`, with accumulated digits advancing to `Call Initiated` once complete.*

DTMF Tone Detection
===================

Phones with touch-tone support emit dual-frequency tones that represent digits and control characters. The firmware analyzes incoming audio to detect valid DTMF pairs.

- **Tone Pairs**: Each button generates a unique combination of two tones (e.g., 697 + 1209 Hz = “1”)
- **Validation**: Both frequencies must be present simultaneously for a minimum duration.
- **Digit Mapping**: Recognized tone pairs are translated directly to numeric or control values.

*→ Valid DTMF digits follow the same flow as rotary input, advancing the firmware through `Dialing` to `Call Initiated`.*

Line Signaling and Ringing
===========================

In addition to input from the user, the firmware also responds to signaling conditions such as incoming call alerts.

- **Incoming Call Detection**: Triggered by Bluetooth event, not line voltage.
- **Ringer Activation**: Firmware instructs hardware to activate the ringer, mimicking legacy ring cadence.
- **Answer Trigger**: Handset pickup during ring transitions the system into connected state.

*→ Incoming ring triggers a move from `Idle` to `Ringing`; pickup transitions to `Connected`.*

Display and Feedback Cues
==========================

While not required for core signaling, the firmware also updates the front panel display in response to input events.

- **Digit Display**: Shows pulse or tone digits as they are received
- **State Feedback**: Indicates transitions like “Dialing,” “Connected,” or “Call Ended”
- **Error Prompts**: Optional messaging for incomplete numbers or timeouts

*→ Display updates reflect current firmware state and serve as a visual interface for both user feedback and debugging.*

Design Considerations
======================

- Input detection must be tolerant of imperfect signals while still responsive.
- Different input modes (pulse vs. tone) must be treated uniformly at the behavioral level.
- All input ultimately serves to drive the state machine described in the previous section, maintaining a clear separation between physical signal interpretation and firmware decision logic.
