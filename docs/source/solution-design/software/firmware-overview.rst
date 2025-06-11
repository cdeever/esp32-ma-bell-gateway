=================
Firmware Overview
=================

The Ma Bell Gateway firmware transforms a modern ESP32 microcontroller into a bridge between vintage analog telephony and contemporary mobile communication. It recreates the core behavioral elements of a classic landline—ringing, dialing, call progress tones, and voice transmission—while interfacing seamlessly with smartphones over Bluetooth.

This section introduces the core functions of the firmware, outlining the primary responsibilities and behaviors of each subsystem without diving into implementation details. Later sections will explore how these components are built and how they interact.

Key Functions
=============

The firmware is organized around several coordinated subsystems, each handling a distinct aspect of the gateway's operation:

- **Bluetooth Call Handling**

  Manages the connection between the ESP32 and a paired mobile phone, enabling the gateway to place and receive calls. Incoming calls ring the physical ringer. Outgoing calls are initiated from the rotary dial and relayed to the phone. Audio is streamed in both directions over Bluetooth.

- **Dialing Input Recognition**

  The firmware supports both tone-based (DTMF) and pulse-based rotary dialing. As digits are dialed, the system captures them and assembles them into a complete phone number, triggering a call once all digits are received.

- **Call Progress Simulation**

  The gateway emulates classic telephone audio feedback—including dial tone, ringback, busy signals, and reorder tones. These audio cues are played to the user locally through the handset speaker to mirror traditional telephone behavior.

- **Voice Audio Routing**

  During a live call, voice is streamed bidirectionally between the analog handset and the paired phone. The system manages transitions between local audio (tones, feedback) and Bluetooth audio (live conversation).

- **Status Monitoring and Diagnostics**

  The device exposes its internal state through an embedded web interface, showing pairing status, call progress, and debug information. This allows for quick troubleshooting and insight into system behavior.

- **User Interface Feedback**

  A small display on the gateway provides real-time updates to the user—showing Bluetooth status, dialed digits, and call events. It acts as a minimal front panel interface for environments where the web interface isn't accessible.

Future sections will explore how each of these responsibilities is realized in code, including the state machine logic, Bluetooth stack integration, audio driver setup, and real-time GPIO input handling.
