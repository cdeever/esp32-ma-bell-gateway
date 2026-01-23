# Ma Bell Gateway Risk Register

**Document Version:** 1.0
**Date:** 2026-01-22
**Project:** ESP32 Ma Bell Gateway

---

## 1. Introduction

### 1.1 Purpose

This document identifies, assesses, and provides mitigation strategies for risks associated with the Ma Bell Gateway project. It covers technical, operational, and schedule risks throughout the development and deployment lifecycle.

### 1.2 Risk Assessment Methodology

#### Probability Scale
| Level | Description | Probability |
|-------|-------------|-------------|
| 1 - Rare | Unlikely to occur | < 10% |
| 2 - Unlikely | Could occur but not expected | 10-30% |
| 3 - Possible | May occur | 30-60% |
| 4 - Likely | Probably will occur | 60-90% |
| 5 - Almost Certain | Expected to occur | > 90% |

#### Impact Scale
| Level | Description | Effect |
|-------|-------------|--------|
| 1 - Negligible | Minor inconvenience | No significant impact |
| 2 - Minor | Some rework required | Delays < 1 week |
| 3 - Moderate | Significant rework | Delays 1-4 weeks |
| 4 - Major | Major redesign needed | Delays > 1 month |
| 5 - Catastrophic | Project failure | Project cancelled or unusable |

#### Risk Score
**Risk Score = Probability × Impact**

| Score | Priority | Action |
|-------|----------|--------|
| 1-4 | Low | Monitor |
| 5-9 | Medium | Mitigation plan required |
| 10-15 | High | Active mitigation, escalation |
| 16-25 | Critical | Immediate action required |

---

## 2. Technical Risks

### RISK-T001: Bluetooth Audio Quality Degradation

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-T001 |
| **Category** | Technical - Audio |
| **Description** | Bluetooth audio quality may degrade due to WiFi interference, buffer underruns, or HFP codec limitations, resulting in poor call quality. |
| **Probability** | 4 - Likely |
| **Impact** | 4 - Major |
| **Risk Score** | 16 - Critical |
| **Status** | Active |

**Root Causes:**
- 2.4 GHz band shared between WiFi and Bluetooth
- Ring buffer sizing may be insufficient for variable latency
- HFP uses narrow-band audio (8 kHz)
- ESP32 processing load during simultaneous operations

**Mitigation Strategies:**
1. Initialize WiFi before Bluetooth to establish coexistence parameters
2. Size ring buffers for 11+ frames (220ms) of buffering
3. Monitor buffer levels and log overflows/underflows
4. Consider WiFi channel selection to avoid BT interference
5. Implement adaptive buffer management if issues persist

**Contingency:**
- Disable WiFi during active calls if interference is severe
- Reduce web server polling frequency

**Owner:** Audio subsystem developer
**Review Date:** Monthly

---

### RISK-T002: Critical Bluetooth Initialization Bug (RESOLVED)

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-T002 |
| **Category** | Technical - Bluetooth |
| **Description** | Original ESP-IDF example code had a critical bug where `bt_app_task_start_up()` was never called, causing BT message queue failures. |
| **Probability** | 1 - Rare (fixed) |
| **Impact** | 5 - Catastrophic |
| **Risk Score** | 5 - Medium (was 25 before fix) |
| **Status** | Mitigated |

**Root Cause:**
- ESP-IDF example code omission
- BT callbacks registered before task/queue initialization

**Mitigation Applied:**
- Fixed in `bt_init.c` - `bt_app_task_start_up()` called at step 6, before any callbacks
- Documented in CLAUDE.md as critical initialization order

**Residual Risk:**
- Future refactoring could reintroduce bug
- Mitigation: Code review checklist item, unit test coverage

**Owner:** Bluetooth subsystem developer
**Review Date:** Quarterly

---

### RISK-T003: Audio Tone Blocking Bluetooth Audio

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-T003 |
| **Category** | Technical - Audio |
| **Description** | Local tone generation (dial tone, busy, etc.) uses the same I2S output as Bluetooth receive audio, causing blocking when tones are active. |
| **Probability** | 4 - Likely |
| **Impact** | 3 - Moderate |
| **Risk Score** | 12 - High |
| **Status** | Active |

**Root Causes:**
- Single I2S TX path shared between tone generator and BT audio
- `audio_output_write()` blocks when tone is active
- State machine may not properly sequence tone stop and audio start

**Mitigation Strategies:**
1. Ensure state machine stops tones before establishing call audio
2. Add timeout to tone blocking to prevent deadlock
3. Implement audio mixing if simultaneous output required (call waiting tone)
4. Log warnings when blocking occurs during call

**Contingency:**
- Force-stop all tones when BT audio connects
- Prioritize BT audio over local tones during calls

**Owner:** Audio subsystem developer
**Review Date:** Monthly

---

### RISK-T004: Stack Overflow in FreeRTOS Tasks

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-T004 |
| **Category** | Technical - System |
| **Description** | Multiple concurrent tasks (reconnect, audio RX/TX, SLIC monitor, web server) may exhaust stack space, causing crashes. |
| **Probability** | 2 - Unlikely |
| **Impact** | 4 - Major |
| **Risk Score** | 8 - Medium |
| **Status** | Active |

**Root Causes:**
- Fixed stack allocations may be insufficient for edge cases
- Deep call chains in callbacks
- Local array allocations in functions

**Current Stack Allocations:**
- BT app task: 2048 bytes
- Reconnect task: 4096 bytes
- Web server: 8192 bytes
- Audio tasks: 2048 bytes each

**Mitigation Strategies:**
1. Monitor stack high water mark via `/tasks` endpoint
2. Use `uxTaskGetStackHighWaterMark()` in debug builds
3. Avoid large local arrays; use heap or static allocation
4. Enable stack overflow detection in FreeRTOS config

**Contingency:**
- Increase stack sizes if high water mark < 20% headroom
- Reduce task count by combining functionality

**Owner:** System architect
**Review Date:** Quarterly

---

### RISK-T005: Rotary Pulse Dial Detection Not Implemented

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-T005 |
| **Category** | Technical - Feature Gap |
| **Description** | Rotary pulse dial detection logic is not implemented despite GPIO configuration, limiting functionality to DTMF phones only. |
| **Probability** | 5 - Almost Certain |
| **Impact** | 3 - Moderate |
| **Risk Score** | 15 - High |
| **Status** | Open |

**Root Cause:**
- Feature not yet developed
- GPIO 34 configured but no pulse counting/decoding logic

**Technical Requirements:**
- Pulse rate: 8-12 pulses per second (10 pps typical)
- Pulse width: ~60ms make, ~40ms break
- Inter-digit pause: 300-800ms
- Debouncing required for mechanical contacts

**Mitigation Strategies:**
1. Implement pulse counting ISR or polling task
2. Use hardware timer capture for accurate timing
3. Add state machine for digit assembly
4. Test with multiple vintage rotary phones

**Contingency:**
- Document as known limitation
- Recommend DTMF adapter for rotary phones

**Owner:** Hardware interface developer
**Review Date:** Next sprint

---

### RISK-T006: DTMF Detection Not Implemented

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-T006 |
| **Category** | Technical - Feature Gap |
| **Description** | DTMF tone detection is not implemented, requiring external DTMF decoder IC or software decoding. |
| **Probability** | 5 - Almost Certain |
| **Impact** | 3 - Moderate |
| **Risk Score** | 15 - High |
| **Status** | Open |

**Root Cause:**
- Feature not yet developed
- GPIO 39 reserved but no detection logic

**Technical Options:**
1. **Hardware decoder (MT8870):** Dedicated IC, simple GPIO interface
2. **Software Goertzel algorithm:** CPU-intensive but no additional hardware
3. **Hybrid:** Use ADC input with software detection

**Mitigation Strategies:**
1. Evaluate hardware vs. software trade-offs
2. If software: implement Goertzel filter for 8 DTMF frequencies
3. If hardware: design decoder interface circuit
4. Validate with standard DTMF test signals

**Contingency:**
- Prioritize rotary dial support for vintage phone compatibility
- Add DTMF in subsequent release

**Owner:** Hardware interface developer
**Review Date:** Next sprint

---

### RISK-T007: Ring Circuit Not Fully Implemented

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-T007 |
| **Category** | Technical - Feature Gap |
| **Description** | Ring command output (GPIO 13) is configured but actual ring generation circuit and control logic are incomplete. |
| **Probability** | 4 - Likely |
| **Impact** | 4 - Major |
| **Risk Score** | 16 - Critical |
| **Status** | Open |

**Root Cause:**
- Ring voltage generation requires external circuit (90V AC)
- Safety considerations for high voltage
- Control timing logic not implemented

**Technical Requirements:**
- Ring voltage: 90V RMS, 20 Hz
- Ring cadence: 2 seconds on, 4 seconds off
- Ring trip detection (stop when answered)
- Safety isolation from ESP32

**Mitigation Strategies:**
1. Design ring generator circuit with proper isolation
2. Implement ring cadence timing in software
3. Add ring trip detection via SLIC feedback
4. Conduct safety review of high-voltage design

**Contingency:**
- Use electronic ringer as alternative to electromechanical
- External ring generator module

**Owner:** Hardware designer
**Review Date:** Before hardware rev 2

---

### RISK-T008: Memory Leak Over Extended Operation

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-T008 |
| **Category** | Technical - Reliability |
| **Description** | Long-running operation may reveal memory leaks causing gradual DRAM exhaustion and eventual system instability. |
| **Probability** | 3 - Possible |
| **Impact** | 4 - Major |
| **Risk Score** | 12 - High |
| **Status** | Active |

**Root Causes:**
- Repeated allocation/deallocation in event handlers
- NVS operations may leak handles
- Web server request handling
- Bluetooth reconnection cycles

**Mitigation Strategies:**
1. Implement heap monitoring task (log free heap periodically)
2. Use static allocation where possible
3. Run 24-hour soak tests and monitor heap trend
4. Use ESP-IDF heap tracing in debug builds
5. Review all malloc/free pairs

**Contingency:**
- Implement watchdog-triggered restart if heap critically low
- Scheduled periodic restart (e.g., weekly)

**Owner:** System architect
**Review Date:** Monthly

---

### RISK-T009: WiFi and Bluetooth Coexistence Issues

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-T009 |
| **Category** | Technical - Connectivity |
| **Description** | Simultaneous WiFi and Bluetooth Classic operation may cause connectivity issues, packet loss, or audio artifacts. |
| **Probability** | 3 - Possible |
| **Impact** | 3 - Moderate |
| **Risk Score** | 9 - Medium |
| **Status** | Active |

**Root Cause:**
- Both radios share 2.4 GHz band and antenna
- ESP32 coexistence arbitration has limitations
- Heavy WiFi traffic during BT audio streaming

**Mitigation Strategies:**
1. Initialize WiFi before Bluetooth (current implementation)
2. Reduce web server polling/response frequency
3. Use WiFi power save mode during calls
4. Consider 5 GHz WiFi if hardware supports

**Contingency:**
- Disable WiFi during active calls
- Reduce WiFi functionality to status-only

**Owner:** Network/Bluetooth developer
**Review Date:** Quarterly

---

### RISK-T010: ESP-IDF Version Compatibility

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-T010 |
| **Category** | Technical - Dependencies |
| **Description** | Future ESP-IDF updates may introduce breaking changes to Bluetooth HFP, I2S, or WiFi APIs. |
| **Probability** | 3 - Possible |
| **Impact** | 3 - Moderate |
| **Risk Score** | 9 - Medium |
| **Status** | Monitoring |

**Root Cause:**
- Dependency on ESP-IDF v5.0+ APIs
- Bluetooth HFP client is less stable than mature APIs
- I2S driver underwent changes between v4.x and v5.x

**Mitigation Strategies:**
1. Document minimum and tested ESP-IDF versions
2. Pin to specific ESP-IDF version in CI/CD
3. Test on new ESP-IDF releases before upgrading
4. Monitor ESP-IDF release notes for deprecations

**Contingency:**
- Maintain compatibility shim layer if needed
- Delay ESP-IDF upgrades until validated

**Owner:** Build system maintainer
**Review Date:** Per ESP-IDF release

---

## 3. Operational Risks

### RISK-O001: No WiFi Provisioning UI

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-O001 |
| **Category** | Operational - Usability |
| **Description** | WiFi credentials must be provisioned via external tool or NVS programming; no built-in provisioning interface exists. |
| **Probability** | 5 - Almost Certain |
| **Impact** | 2 - Minor |
| **Risk Score** | 10 - High |
| **Status** | Open |

**Root Cause:**
- No AP mode or BLE provisioning implemented
- Credentials stored in NVS require programming tool

**Mitigation Strategies:**
1. Document provisioning procedure clearly
2. Create provisioning script/tool
3. Consider adding BLE provisioning (SoftAP + web config)
4. Allow operation without WiFi (degrades gracefully)

**Contingency:**
- Compile-time WiFi credentials for specific deployments
- Pre-programmed NVS images

**Owner:** Product owner
**Review Date:** Before v1.0 release

---

### RISK-O002: Single Paired Device Limitation

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-O002 |
| **Category** | Operational - Usability |
| **Description** | System only stores one paired Bluetooth device; pairing a new device overwrites the previous pairing. |
| **Probability** | 4 - Likely |
| **Impact** | 2 - Minor |
| **Risk Score** | 8 - Medium |
| **Status** | Accepted |

**Root Cause:**
- Design decision for simplicity
- NVS storage only holds one device address

**Mitigation Strategies:**
1. Document limitation in user guide
2. Implement device selection UI (future enhancement)
3. Allow multiple stored devices with selection mechanism

**Contingency:**
- Users can re-pair as needed
- Acceptable for single-user home deployment

**Owner:** Product owner
**Review Date:** Future feature consideration

---

### RISK-O003: No Remote Diagnostics

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-O003 |
| **Category** | Operational - Maintainability |
| **Description** | Troubleshooting requires physical access to serial port; no remote logging or diagnostic capability. |
| **Probability** | 4 - Likely |
| **Impact** | 2 - Minor |
| **Risk Score** | 8 - Medium |
| **Status** | Open |

**Root Cause:**
- Serial logging only available via USB connection
- Web interface provides status but not logs
- No MQTT/syslog implementation

**Mitigation Strategies:**
1. Enhance web interface with log viewing
2. Implement MQTT logging (infrastructure exists)
3. Add log level control via web interface
4. Store recent logs in RAM ring buffer

**Contingency:**
- Provide detailed troubleshooting guide
- Status LED patterns for common issues

**Owner:** System architect
**Review Date:** Post-v1.0

---

### RISK-O004: No OTA Update Capability

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-O004 |
| **Category** | Operational - Maintainability |
| **Description** | Firmware updates require physical USB connection; no over-the-air update capability implemented. |
| **Probability** | 5 - Almost Certain |
| **Impact** | 2 - Minor |
| **Risk Score** | 10 - High |
| **Status** | Open |

**Root Cause:**
- OTA subsystem not implemented
- Partition table supports OTA but code doesn't use it

**Mitigation Strategies:**
1. Implement ESP-IDF OTA component
2. Add OTA trigger via web interface
3. Add version checking and rollback capability
4. Secure OTA with signature verification

**Contingency:**
- Provide USB update procedure documentation
- Pre-configured update tool

**Owner:** System architect
**Review Date:** v1.1 feature

---

## 4. Hardware Risks

### RISK-H001: SLIC Component Availability

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-H001 |
| **Category** | Hardware - Supply Chain |
| **Description** | The HC-5504B SLIC may become unavailable or have long lead times, impacting production. |
| **Probability** | 3 - Possible |
| **Impact** | 4 - Major |
| **Risk Score** | 12 - High |
| **Status** | Monitoring |

**Root Cause:**
- Specialized telephony component
- Limited suppliers
- Global semiconductor supply variability

**Mitigation Strategies:**
1. Identify alternate SLIC components (Si3210, AG1171)
2. Maintain component inventory buffer
3. Design hardware abstraction for SLIC variations
4. Source from multiple distributors

**Contingency:**
- Redesign for alternate SLIC if needed
- Discrete transistor implementation (complex)

**Owner:** Hardware designer
**Review Date:** Quarterly

---

### RISK-H002: High-Voltage Ring Generator Safety

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-H002 |
| **Category** | Hardware - Safety |
| **Description** | Ring voltage generation (90V AC) poses electrical safety risks if not properly isolated and protected. |
| **Probability** | 2 - Unlikely |
| **Impact** | 5 - Catastrophic |
| **Risk Score** | 10 - High |
| **Status** | Active |

**Root Cause:**
- Ring voltage significantly exceeds safe touch voltage
- Failure could expose users to shock hazard
- DIY construction may lack proper safety measures

**Mitigation Strategies:**
1. Design with proper isolation (optocouplers, transformers)
2. Include fusing and current limiting
3. Enclose high-voltage section
4. Clear safety warnings in documentation
5. Consider low-voltage electronic ringer alternative

**Contingency:**
- Use external, certified ring generator
- Recommend professional installation

**Owner:** Hardware designer
**Review Date:** Before hardware release

---

### RISK-H003: Audio Codec Quality Limitations

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-H003 |
| **Category** | Hardware - Audio |
| **Description** | PCM1808 ADC and PCM5100 DAC may introduce noise or distortion affecting call quality. |
| **Probability** | 2 - Unlikely |
| **Impact** | 3 - Moderate |
| **Risk Score** | 6 - Medium |
| **Status** | Monitoring |

**Root Cause:**
- PCB layout affecting analog performance
- Power supply noise coupling
- Component tolerance variations

**Mitigation Strategies:**
1. Follow codec reference design for PCB layout
2. Use proper power supply filtering
3. Separate analog and digital grounds appropriately
4. Test audio quality with spectrum analyzer

**Contingency:**
- Upgrade to higher-quality codec if needed
- Add analog filtering stages

**Owner:** Hardware designer
**Review Date:** During hardware testing

---

## 5. Schedule Risks

### RISK-S001: Feature Scope Creep

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-S001 |
| **Category** | Schedule - Scope |
| **Description** | Additional features (call waiting, conference calling, voicemail indication) may be requested, delaying core functionality. |
| **Probability** | 4 - Likely |
| **Impact** | 3 - Moderate |
| **Risk Score** | 12 - High |
| **Status** | Active |

**Mitigation Strategies:**
1. Define MVP feature set and prioritize
2. Defer non-essential features to future versions
3. Document feature requests in backlog
4. Communicate scope decisions clearly

**Owner:** Project lead
**Review Date:** Per sprint

---

### RISK-S002: Hardware Revision Delays

| Attribute | Value |
|-----------|-------|
| **ID** | RISK-S002 |
| **Category** | Schedule - Hardware |
| **Description** | PCB fabrication, component procurement, or assembly issues may delay hardware availability for testing. |
| **Probability** | 3 - Possible |
| **Impact** | 3 - Moderate |
| **Risk Score** | 9 - Medium |
| **Status** | Monitoring |

**Mitigation Strategies:**
1. Use breadboard prototype for early development
2. Order components in advance
3. Use expedited PCB services when needed
4. Maintain relationships with multiple suppliers

**Contingency:**
- Continue software development with simulation
- Use development boards with breakout modules

**Owner:** Hardware designer
**Review Date:** Per hardware revision

---

## 6. Risk Summary Dashboard

### Critical Risks (Score 16-25)
| ID | Risk | Score | Status |
|----|------|-------|--------|
| RISK-T001 | Bluetooth Audio Quality | 16 | Active |
| RISK-T007 | Ring Circuit Incomplete | 16 | Open |

### High Risks (Score 10-15)
| ID | Risk | Score | Status |
|----|------|-------|--------|
| RISK-T005 | Rotary Dial Not Implemented | 15 | Open |
| RISK-T006 | DTMF Not Implemented | 15 | Open |
| RISK-T003 | Tone Blocking Audio | 12 | Active |
| RISK-T008 | Memory Leak Risk | 12 | Active |
| RISK-H001 | SLIC Availability | 12 | Monitoring |
| RISK-S001 | Feature Scope Creep | 12 | Active |
| RISK-O001 | No WiFi Provisioning | 10 | Open |
| RISK-O004 | No OTA Updates | 10 | Open |
| RISK-H002 | Ring Generator Safety | 10 | Active |

### Medium Risks (Score 5-9)
| ID | Risk | Score | Status |
|----|------|-------|--------|
| RISK-T009 | WiFi/BT Coexistence | 9 | Active |
| RISK-T010 | ESP-IDF Compatibility | 9 | Monitoring |
| RISK-S002 | Hardware Delays | 9 | Monitoring |
| RISK-T004 | Stack Overflow | 8 | Active |
| RISK-O002 | Single Device Limit | 8 | Accepted |
| RISK-O003 | No Remote Diagnostics | 8 | Open |
| RISK-H003 | Codec Quality | 6 | Monitoring |
| RISK-T002 | BT Init Bug (Fixed) | 5 | Mitigated |

---

## 7. Risk Review Schedule

| Review Type | Frequency | Participants |
|-------------|-----------|--------------|
| Critical Risk Review | Weekly | Project lead, affected owners |
| Full Risk Review | Monthly | All stakeholders |
| Risk Retrospective | Per release | Full team |

---

## 8. Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-22 | Generated | Initial risk register based on codebase analysis |
