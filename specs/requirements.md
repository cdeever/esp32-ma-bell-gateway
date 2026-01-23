# Ma Bell Gateway Requirements Specification

**Document Version:** 1.0
**Date:** 2026-01-22
**Project:** ESP32 Ma Bell Gateway

---

## 1. Introduction

### 1.1 Purpose

This document defines the formal functional requirements (FR) and non-functional requirements (NFR) for the Ma Bell Gateway system—an ESP32-based Central Office (CO) line emulator that enables vintage rotary telephones to make and receive calls via Bluetooth Hands-Free Profile (HFP) connection to a mobile phone.

### 1.2 Scope

The requirements cover:
- Bluetooth connectivity and HFP profile implementation
- Telephone line interface and signaling
- Audio processing and tone generation
- Network connectivity and web interface
- State management and system behavior

### 1.3 Definitions

| Term | Definition |
|------|------------|
| CO | Central Office - traditional telephone switching center |
| SLIC | Subscriber Line Interface Circuit |
| HFP | Hands-Free Profile (Bluetooth) |
| DTMF | Dual-Tone Multi-Frequency signaling |
| NVS | Non-Volatile Storage |
| PCM | Pulse Code Modulation |
| I2S | Inter-IC Sound interface |

---

## 2. Functional Requirements

### 2.1 Bluetooth Connectivity

#### FR-001: Bluetooth HFP Client Support
**Priority:** Critical
**Description:** The system SHALL implement Bluetooth Hands-Free Profile (HFP) as a client device to connect to mobile phones acting as Audio Gateways.

#### FR-002: Device Discovery and Pairing
**Priority:** Critical
**Description:** The system SHALL support device discovery and PIN-based pairing with Bluetooth audio gateways.

**Acceptance Criteria:**
- Device name "MA BELL" SHALL be advertised during discovery
- PIN code authentication SHALL be supported (default: "0000")
- Successful pairing SHALL store device address in NVS

#### FR-003: Automatic Reconnection
**Priority:** High
**Description:** The system SHALL automatically attempt to reconnect to the last paired device when Bluetooth connection is lost.

**Acceptance Criteria:**
- Reconnection attempts SHALL occur every 10 seconds
- Last paired device address SHALL be retrieved from NVS
- Reconnection SHALL not interfere with active phone operations

#### FR-004: Paired Device Storage
**Priority:** High
**Description:** The system SHALL persistently store paired device information (MAC address and device name) in non-volatile storage.

#### FR-005: Bluetooth Audio Path
**Priority:** Critical
**Description:** The system SHALL establish bidirectional audio paths when connected to an Audio Gateway during active calls.

**Acceptance Criteria:**
- Audio SHALL be routed via HCI (Host Controller Interface)
- Sample rate SHALL be 8 kHz (telephony standard)
- Bit depth SHALL be 16-bit PCM

### 2.2 Call Handling

#### FR-010: Inbound Call Detection
**Priority:** Critical
**Description:** The system SHALL detect incoming calls from the paired Bluetooth device and signal the local telephone.

**Acceptance Criteria:**
- Ring indication (RING_IND) from HFP SHALL trigger local ringing
- Caller ID information SHALL be captured when available

#### FR-011: Inbound Call Answer
**Priority:** Critical
**Description:** The system SHALL answer incoming calls when the telephone handset is lifted (off-hook).

**Acceptance Criteria:**
- Off-hook detection SHALL trigger call answer command to HFP
- Audio path SHALL be established within 500ms of answer

#### FR-012: Outbound Call Initiation
**Priority:** Critical
**Description:** The system SHALL initiate outbound calls when the user dials a complete phone number.

**Acceptance Criteria:**
- Dial tone SHALL play when handset is lifted
- Collected digits SHALL be transmitted via HFP ATD command
- System SHALL support numbers up to 15 digits

#### FR-013: Call Termination
**Priority:** Critical
**Description:** The system SHALL terminate active calls when the handset is replaced (on-hook) or remote party disconnects.

**Acceptance Criteria:**
- On-hook SHALL send call hangup command
- Remote hangup SHALL be detected and audio path disconnected
- System SHALL return to idle state within 1 second

#### FR-014: Call State Tracking
**Priority:** High
**Description:** The system SHALL track and expose current call state including: idle, ringing, dialing, ringback, connected, and call-ended.

### 2.3 Telephone Line Interface

#### FR-020: Off-Hook Detection
**Priority:** Critical
**Description:** The system SHALL detect when the telephone handset is lifted (off-hook condition).

**Acceptance Criteria:**
- Detection via SLIC SHD pin (GPIO 32)
- Debounce period of 50ms SHALL be applied
- State change SHALL trigger appropriate system response

#### FR-021: On-Hook Detection
**Priority:** Critical
**Description:** The system SHALL detect when the telephone handset is replaced (on-hook condition).

**Acceptance Criteria:**
- Detection via SLIC SHD pin (GPIO 32)
- Debounce period of 50ms SHALL be applied
- Active calls SHALL be terminated on-hook

#### FR-022: Ring Generation
**Priority:** High
**Description:** The system SHALL generate ringing voltage to activate the telephone ringer for incoming calls.

**Acceptance Criteria:**
- Ring command output via GPIO 13
- Standard North American ring cadence: 2 seconds on, 4 seconds off
- Ringing SHALL cease when call is answered or caller hangs up

#### FR-023: Rotary Pulse Dial Detection
**Priority:** Medium
**Description:** The system SHALL decode rotary dial pulses to determine dialed digits.

**Acceptance Criteria:**
- Pulse detection via GPIO 34
- Support pulse rates of 8-12 pulses per second
- Inter-digit timeout of 3 seconds

#### FR-024: DTMF Detection
**Priority:** Medium
**Description:** The system SHALL decode DTMF tones for touch-tone telephone support.

**Acceptance Criteria:**
- DTMF detection via GPIO 39 or software decoding
- Support all 16 DTMF tones (0-9, *, #, A-D)
- Detection accuracy of 99% or better

#### FR-025: Dial Lamp Control
**Priority:** Low
**Description:** The system SHALL control the dial lamp for illuminated dial telephones.

**Acceptance Criteria:**
- Lamp control via GPIO 27
- Lamp SHALL illuminate when handset is off-hook

### 2.4 Audio System

#### FR-030: Bidirectional Audio Bridge
**Priority:** Critical
**Description:** The system SHALL provide full-duplex audio bridging between the telephone and Bluetooth connection.

**Acceptance Criteria:**
- Phone microphone audio SHALL transmit to Bluetooth
- Bluetooth received audio SHALL play through phone earpiece
- Audio latency SHALL not exceed 100ms end-to-end

#### FR-031: I2S Audio Interface
**Priority:** Critical
**Description:** The system SHALL interface with external ADC/DAC via I2S protocol.

**Acceptance Criteria:**
- I2S port 0 configured for audio I/O
- Sample rate: 8000 Hz
- Bit depth: 16-bit
- Frame sync, bit clock, and data lines properly configured

#### FR-032: Dial Tone Generation
**Priority:** Critical
**Description:** The system SHALL generate North American standard dial tone when user goes off-hook.

**Acceptance Criteria:**
- Dual frequencies: 350 Hz + 440 Hz
- Continuous tone until digit dialed or timeout
- Volume level appropriate for telephone earpiece

#### FR-033: Ringback Tone Generation
**Priority:** High
**Description:** The system SHALL generate ringback tone during outbound call setup.

**Acceptance Criteria:**
- Dual frequencies: 440 Hz + 480 Hz
- Cadence: 2 seconds on, 4 seconds off
- Tone SHALL cease when call is answered

#### FR-034: Busy Signal Generation
**Priority:** High
**Description:** The system SHALL generate busy signal when called party is unavailable.

**Acceptance Criteria:**
- Dual frequencies: 480 Hz + 620 Hz
- Cadence: 0.5 seconds on, 0.5 seconds off

#### FR-035: Reorder Tone Generation
**Priority:** High
**Description:** The system SHALL generate reorder (fast busy) tone for error conditions.

**Acceptance Criteria:**
- Dual frequencies: 480 Hz + 620 Hz
- Cadence: 0.25 seconds on, 0.25 seconds off

#### FR-036: Off-Hook Warning Tone
**Priority:** Medium
**Description:** The system SHALL generate off-hook warning tone when phone is left off-hook too long.

**Acceptance Criteria:**
- Dual frequencies: 1400 Hz + 2060 Hz
- Cadence: 0.1 seconds on, 0.1 seconds off
- Triggered after extended dial tone timeout

#### FR-037: Call Waiting Tone
**Priority:** Low
**Description:** The system SHALL generate call waiting tone for incoming calls during active call.

**Acceptance Criteria:**
- Frequency: 440 Hz
- Duration: 0.3 second beep

#### FR-038: Audio Ring Buffers
**Priority:** High
**Description:** The system SHALL use ring buffers to decouple audio tasks from Bluetooth callbacks.

**Acceptance Criteria:**
- Buffer size: 3600 bytes (approximately 11 audio frames)
- Separate buffers for TX and RX paths
- Buffer overflow/underflow SHALL be logged

### 2.5 Network and Web Interface

#### FR-040: WiFi Station Mode
**Priority:** Medium
**Description:** The system SHALL connect to an existing WiFi network in station mode.

**Acceptance Criteria:**
- WiFi credentials stored in NVS
- Automatic connection on startup
- Automatic reconnection on disconnect

#### FR-041: HTTP Status API
**Priority:** Medium
**Description:** The system SHALL provide an HTTP REST API for status monitoring.

**Acceptance Criteria:**
- Server on port 80
- Endpoints: `/`, `/status`, `/tasks`
- JSON response format
- No authentication required (local network)

#### FR-042: System Status Endpoint
**Priority:** Medium
**Description:** The `/status` endpoint SHALL return comprehensive system state.

**Acceptance Criteria:**
- Phone state (hook status, ringing, dialing, active tones)
- Bluetooth state (connected, device name, call status, volume, battery, signal)
- WiFi state (connected, SSID, IP, RSSI, channel)
- System state (uptime, initialized status)

#### FR-043: Task Status Endpoint
**Priority:** Low
**Description:** The `/tasks` endpoint SHALL return FreeRTOS task information.

**Acceptance Criteria:**
- List of active tasks
- Stack usage per task
- Task state and priority

### 2.6 State Management

#### FR-050: Centralized State Tracking
**Priority:** High
**Description:** The system SHALL maintain centralized state for all subsystems using bitmask representation.

**Acceptance Criteria:**
- Phone state bits (8 bits)
- Bluetooth state bits (8 bits)
- Network state bits (8 bits)
- System state bits (8 bits)

#### FR-051: State Change Notifications
**Priority:** High
**Description:** The system SHALL support task notifications for state changes.

**Acceptance Criteria:**
- Tasks can register for notifications
- Notification bits indicate which state category changed
- Timeout-based waiting supported

#### FR-052: Event Publish/Subscribe
**Priority:** High
**Description:** The system SHALL implement a publish/subscribe event system for decoupled subsystem communication.

**Acceptance Criteria:**
- Support for up to 10 subscribers
- Bitmask-based event filtering
- Synchronous callback delivery
- Thread-safe implementation

### 2.7 Configuration and Storage

#### FR-060: Non-Volatile Configuration Storage
**Priority:** High
**Description:** The system SHALL store configuration in NVS with defined namespaces.

**Acceptance Criteria:**
- `wifi` namespace for network credentials
- `bt` namespace for Bluetooth settings
- `sys` namespace for system preferences

#### FR-061: Configurable Bluetooth Identity
**Priority:** Medium
**Description:** The Bluetooth device name and PIN SHALL be configurable.

**Acceptance Criteria:**
- Default device name: "MA BELL"
- Default PIN: "0000"
- Configuration via compile-time constants

---

## 3. Non-Functional Requirements

### 3.1 Performance

#### NFR-001: Audio Latency
**Priority:** Critical
**Description:** End-to-end audio latency SHALL not exceed 100ms for acceptable conversation quality.

**Acceptance Criteria:**
- Measured from phone microphone to Bluetooth transmission
- Measured from Bluetooth reception to phone earpiece

#### NFR-002: Audio Frame Timing
**Priority:** Critical
**Description:** The system SHALL process audio frames within their 20ms deadline.

**Acceptance Criteria:**
- Frame size: 320 bytes (160 samples at 8 kHz)
- No audio frame drops under normal operation

#### NFR-003: Off-Hook Detection Latency
**Priority:** High
**Description:** Off-hook detection SHALL occur within 100ms of handset lift (including debounce).

#### NFR-004: Dial Tone Latency
**Priority:** High
**Description:** Dial tone SHALL begin within 200ms of off-hook detection.

#### NFR-005: Call Answer Latency
**Priority:** High
**Description:** Call SHALL be answered within 500ms of off-hook during ringing.

### 3.2 Reliability

#### NFR-010: System Uptime
**Priority:** High
**Description:** The system SHALL operate continuously without requiring restart under normal conditions.

**Acceptance Criteria:**
- Target uptime: 99.9% (8.76 hours downtime per year max)
- Graceful handling of transient errors

#### NFR-011: Bluetooth Reconnection Reliability
**Priority:** High
**Description:** The system SHALL successfully reconnect to a paired device within 60 seconds of it becoming available.

#### NFR-012: Error Recovery
**Priority:** High
**Description:** The system SHALL recover from error conditions without manual intervention when possible.

**Acceptance Criteria:**
- WiFi disconnect: automatic reconnection
- Bluetooth disconnect: automatic reconnection
- Audio buffer overflow: logged and recovered

#### NFR-013: State Consistency
**Priority:** High
**Description:** System state SHALL remain consistent across all subsystems.

**Acceptance Criteria:**
- No orphaned states (e.g., audio connected without Bluetooth connected)
- State transitions SHALL be atomic

### 3.3 Resource Constraints

#### NFR-020: Memory Usage
**Priority:** High
**Description:** The system SHALL operate within ESP32 memory constraints.

**Acceptance Criteria:**
- DRAM usage: < 200KB (of ~260KB available)
- Flash usage: < 2.5MB (of 3MB partition)

#### NFR-021: CPU Utilization
**Priority:** High
**Description:** Total CPU utilization SHALL not exceed 80% under normal operation.

**Acceptance Criteria:**
- Tone generation: < 15% CPU
- Audio bridging: < 10% CPU
- Bluetooth processing: < 10% CPU
- WiFi: < 5% CPU
- Headroom for burst operations

#### NFR-022: Task Stack Sizes
**Priority:** Medium
**Description:** FreeRTOS task stacks SHALL be appropriately sized to prevent overflow.

**Acceptance Criteria:**
- BT app task: 2048 bytes
- Reconnect task: 4096 bytes
- Web server: 8192 bytes
- Audio tasks: 2048 bytes each

### 3.4 Compatibility

#### NFR-030: ESP-IDF Version
**Priority:** High
**Description:** The system SHALL be compatible with ESP-IDF v5.0 or later.

#### NFR-031: Bluetooth Compatibility
**Priority:** Critical
**Description:** The system SHALL be compatible with HFP 1.7 audio gateways (iOS and Android smartphones).

#### NFR-032: Telephone Compatibility
**Priority:** Critical
**Description:** The system SHALL be compatible with standard analog telephones.

**Acceptance Criteria:**
- Western Electric and ITT rotary phones (1960s-1980s)
- Touch-tone phones with DTMF
- Standard 600-ohm telephone impedance

### 3.5 Security

#### NFR-040: Bluetooth Pairing Security
**Priority:** Medium
**Description:** Bluetooth pairing SHALL require PIN authentication.

#### NFR-041: WiFi Credential Protection
**Priority:** Medium
**Description:** WiFi credentials SHALL be stored securely in NVS (not in source code).

#### NFR-042: Web Interface Security
**Priority:** Low
**Description:** Web interface SHALL be accessible only on local network.

**Note:** No authentication implemented—relies on network isolation.

### 3.6 Maintainability

#### NFR-050: Modular Architecture
**Priority:** High
**Description:** The codebase SHALL maintain clear separation of concerns between subsystems.

**Acceptance Criteria:**
- Subsystem-specific directories
- Clean initialization APIs
- Centralized configuration

#### NFR-051: Configuration Centralization
**Priority:** High
**Description:** All configuration constants SHALL be defined in `main/config/*.h` files.

#### NFR-052: Logging Coverage
**Priority:** Medium
**Description:** The system SHALL provide comprehensive logging for diagnostics.

**Acceptance Criteria:**
- All state transitions logged
- All errors logged with context
- Log levels: ERROR, WARN, INFO, DEBUG

### 3.7 Usability

#### NFR-060: Plug and Play Operation
**Priority:** High
**Description:** After initial pairing, the system SHALL operate without user intervention.

#### NFR-061: Status Visibility
**Priority:** Medium
**Description:** System status SHALL be visible via web interface and/or status LED.

#### NFR-062: Familiar Telephone Behavior
**Priority:** High
**Description:** The system SHALL behave like a traditional telephone line from the user's perspective.

**Acceptance Criteria:**
- Standard dial tone on off-hook
- Standard ring cadence for incoming calls
- Familiar busy/reorder tones

---

## 4. Requirements Traceability Matrix

| Requirement | Category | Priority | Implemented | Verified |
|-------------|----------|----------|-------------|----------|
| FR-001 | Bluetooth | Critical | Yes | - |
| FR-002 | Bluetooth | Critical | Yes | - |
| FR-003 | Bluetooth | High | Yes | - |
| FR-004 | Bluetooth | High | Yes | - |
| FR-005 | Bluetooth | Critical | Yes | - |
| FR-010 | Call Handling | Critical | Yes | - |
| FR-011 | Call Handling | Critical | Yes | - |
| FR-012 | Call Handling | Critical | Partial | - |
| FR-013 | Call Handling | Critical | Yes | - |
| FR-014 | Call Handling | High | Yes | - |
| FR-020 | Phone Interface | Critical | Yes | - |
| FR-021 | Phone Interface | Critical | Yes | - |
| FR-022 | Phone Interface | High | Partial | - |
| FR-023 | Phone Interface | Medium | No | - |
| FR-024 | Phone Interface | Medium | No | - |
| FR-025 | Phone Interface | Low | Partial | - |
| FR-030 | Audio | Critical | Yes | - |
| FR-031 | Audio | Critical | Yes | - |
| FR-032 | Audio | Critical | Yes | - |
| FR-033 | Audio | High | Yes | - |
| FR-034 | Audio | High | Yes | - |
| FR-035 | Audio | High | Yes | - |
| FR-036 | Audio | Medium | Yes | - |
| FR-037 | Audio | Low | Yes | - |
| FR-038 | Audio | High | Yes | - |
| FR-040 | Network | Medium | Yes | - |
| FR-041 | Network | Medium | Yes | - |
| FR-042 | Network | Medium | Yes | - |
| FR-043 | Network | Low | Yes | - |
| FR-050 | State Mgmt | High | Yes | - |
| FR-051 | State Mgmt | High | Yes | - |
| FR-052 | State Mgmt | High | Yes | - |
| FR-060 | Storage | High | Yes | - |
| FR-061 | Storage | Medium | Yes | - |

---

## 5. Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-22 | Generated | Initial requirements based on codebase analysis |
