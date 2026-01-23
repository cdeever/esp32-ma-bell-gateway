# Ma Bell Gateway Verification and Test Plan

**Document Version:** 1.0
**Date:** 2026-01-22
**Project:** ESP32 Ma Bell Gateway

---

## 1. Introduction

### 1.1 Purpose

This document defines the verification and test plan for the Ma Bell Gateway system. It establishes test strategies, test cases, and acceptance criteria to verify that the system meets its functional and non-functional requirements.

### 1.2 Scope

This plan covers:
- Unit testing of software modules
- Integration testing of subsystems
- System-level functional testing
- Performance and stress testing
- Hardware-in-the-loop testing

### 1.3 Test Environment

| Component | Description |
|-----------|-------------|
| Target Hardware | ESP32-WROVER module (8MB flash, 4MB PSRAM) |
| Development Host | macOS/Linux with ESP-IDF v5.0+ |
| Test Framework | Unity (ESP-IDF integrated) |
| External Equipment | Analog telephone, oscilloscope, multimeter, Bluetooth phone |
| Serial Monitor | 115200 baud via USB-UART |

### 1.4 Test Categories

| Category | Abbreviation | Description |
|----------|--------------|-------------|
| Unit Test | UT | Isolated module testing |
| Integration Test | IT | Subsystem interaction testing |
| System Test | ST | End-to-end functional testing |
| Performance Test | PT | Timing and resource testing |
| Hardware Test | HT | Hardware interface verification |

---

## 2. Test Strategy

### 2.1 Test Levels

```
┌─────────────────────────────────────────────────────────┐
│                    System Tests (ST)                     │
│         End-to-end call scenarios, user workflows        │
├─────────────────────────────────────────────────────────┤
│                 Integration Tests (IT)                   │
│      Bluetooth+Audio, WiFi+Web, State+Events            │
├─────────────────────────────────────────────────────────┤
│                    Unit Tests (UT)                       │
│   Tone generation, state machine, event system, etc.    │
└─────────────────────────────────────────────────────────┘
```

### 2.2 Test Priorities

| Priority | Description | Coverage Target |
|----------|-------------|-----------------|
| P1 | Critical path functionality (calls, audio) | 100% |
| P2 | Core features (tones, state management) | 90% |
| P3 | Secondary features (web interface, logging) | 70% |
| P4 | Edge cases and error handling | 50% |

### 2.3 Test Automation

- **Automated:** Unit tests, state machine tests, API response tests
- **Semi-automated:** Audio verification with known signals, timing measurements
- **Manual:** End-to-end call quality, hardware integration, user experience

---

## 3. Unit Test Cases

### 3.1 Tone Generation Module (`main/app/tones.c`)

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| UT-TONE-001 | Dial tone frequencies | Verify dial tone generates 350Hz + 440Hz | FFT shows peaks at 350Hz and 440Hz | P1 |
| UT-TONE-002 | Ringback frequencies | Verify ringback generates 440Hz + 480Hz | FFT shows peaks at 440Hz and 480Hz | P1 |
| UT-TONE-003 | Busy tone frequencies | Verify busy generates 480Hz + 620Hz | FFT shows peaks at 480Hz and 620Hz | P2 |
| UT-TONE-004 | Reorder tone frequencies | Verify reorder generates 480Hz + 620Hz | FFT shows peaks at 480Hz and 620Hz | P2 |
| UT-TONE-005 | Dial tone continuous | Verify dial tone has no cadence (continuous) | Tone plays without interruption | P1 |
| UT-TONE-006 | Ringback cadence | Verify 2s on / 4s off cadence | Timing matches within 50ms | P1 |
| UT-TONE-007 | Busy cadence | Verify 0.5s on / 0.5s off cadence | Timing matches within 20ms | P2 |
| UT-TONE-008 | Reorder cadence | Verify 0.25s on / 0.25s off cadence | Timing matches within 20ms | P2 |
| UT-TONE-009 | Tone amplitude | Verify output amplitude is appropriate | Signal level within -10dB to 0dB | P2 |
| UT-TONE-010 | Tone start/stop | Verify clean tone start and stop | No clicks or pops | P3 |

### 3.2 State Management Module (`main/app/state/ma_bell_state.c`)

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| UT-STATE-001 | Initial state | Verify initial state is all zeros | All state bits cleared | P1 |
| UT-STATE-002 | Set phone bits | Set PHONE_STATE_OFF_HOOK | Bit is set, others unchanged | P1 |
| UT-STATE-003 | Clear phone bits | Clear PHONE_STATE_OFF_HOOK | Bit is cleared, others unchanged | P1 |
| UT-STATE-004 | Set multiple bits | Set multiple bits atomically | All specified bits set | P1 |
| UT-STATE-005 | Query phone bits | Query specific phone state bits | Returns correct boolean | P1 |
| UT-STATE-006 | Set bluetooth bits | Set BT_STATE_CONNECTED | Bit is set correctly | P1 |
| UT-STATE-007 | Set network bits | Set NET_STATE_WIFI_CONNECTED | Bit is set correctly | P2 |
| UT-STATE-008 | State notification | Update state with notification | Registered task receives notification | P1 |
| UT-STATE-009 | Multiple subscribers | Register multiple notification tasks | All tasks receive notifications | P2 |
| UT-STATE-010 | Notification timeout | Wait with timeout, no state change | Returns after timeout | P2 |
| UT-STATE-011 | Thread safety | Concurrent state updates | No race conditions or corruption | P1 |
| UT-STATE-012 | State getter phone | Get complete phone state struct | Returns correct values | P2 |
| UT-STATE-013 | State getter bluetooth | Get complete bluetooth state struct | Returns correct values | P2 |

### 3.3 Event System Module (`main/app/events/event_system.c`)

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| UT-EVENT-001 | Subscribe single | Subscribe to single event type | Subscription successful | P1 |
| UT-EVENT-002 | Subscribe multiple | Subscribe to multiple event types (bitmask) | Subscription successful | P1 |
| UT-EVENT-003 | Publish event | Publish event with subscriber | Callback invoked | P1 |
| UT-EVENT-004 | Callback receives data | Publish with user_data | Callback receives correct data | P2 |
| UT-EVENT-005 | No match no call | Publish event with no matching subscriber | No callback invoked | P2 |
| UT-EVENT-006 | Multiple subscribers | Multiple subscribers to same event | All callbacks invoked | P1 |
| UT-EVENT-007 | Unsubscribe | Unsubscribe from events | Callback no longer invoked | P2 |
| UT-EVENT-008 | Max subscribers | Register maximum subscribers (10) | All registrations successful | P3 |
| UT-EVENT-009 | Exceed max subscribers | Register 11th subscriber | Returns error | P3 |
| UT-EVENT-010 | Thread safety | Concurrent publish/subscribe | No race conditions | P2 |

### 3.4 Storage Module (`main/storage/storage.c`)

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| UT-STOR-001 | Set string | Store string value | Returns ESP_OK | P1 |
| UT-STOR-002 | Get string | Retrieve stored string | Returns correct value | P1 |
| UT-STOR-003 | Set u8 | Store uint8_t value | Returns ESP_OK | P2 |
| UT-STOR-004 | Get u8 | Retrieve stored uint8_t | Returns correct value | P2 |
| UT-STOR-005 | Set u32 | Store uint32_t value | Returns ESP_OK | P2 |
| UT-STOR-006 | Get u32 | Retrieve stored uint32_t | Returns correct value | P2 |
| UT-STOR-007 | Delete key | Delete existing key | Returns ESP_OK, key not found after | P2 |
| UT-STOR-008 | Get nonexistent | Get key that doesn't exist | Returns ESP_ERR_NVS_NOT_FOUND | P2 |
| UT-STOR-009 | Namespace isolation | Keys in different namespaces | No cross-namespace access | P3 |
| UT-STOR-010 | Persistence | Write, reboot, read | Value persists across reboot | P1 |

### 3.5 Web Interface Module (`main/app/web/web_interface.c`)

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| UT-WEB-001 | Root endpoint | GET / | Returns JSON with endpoint list | P2 |
| UT-WEB-002 | Status endpoint | GET /status | Returns JSON with system status | P2 |
| UT-WEB-003 | Tasks endpoint | GET /tasks | Returns JSON with task list | P3 |
| UT-WEB-004 | Status phone state | Check phone state in /status | Correct hook, ringing, dialing status | P2 |
| UT-WEB-005 | Status BT state | Check BT state in /status | Correct connected, device name, volume | P2 |
| UT-WEB-006 | Status WiFi state | Check WiFi state in /status | Correct SSID, IP, RSSI | P2 |
| UT-WEB-007 | Invalid endpoint | GET /invalid | Returns 404 | P3 |
| UT-WEB-008 | No cache headers | Check response headers | Cache-Control: no-cache present | P3 |

---

## 4. Integration Test Cases

### 4.1 Bluetooth + Audio Integration

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| IT-BTA-001 | Audio path setup | Connect BT, establish audio | Audio buffers initialized, paths connected | P1 |
| IT-BTA-002 | Incoming audio | Send audio from phone | Audio appears in rx_ringbuf | P1 |
| IT-BTA-003 | Outgoing audio | Generate tone | Audio appears in tx_ringbuf and transmits | P1 |
| IT-BTA-004 | Bidirectional audio | Full duplex during call | Both paths active simultaneously | P1 |
| IT-BTA-005 | Audio disconnect | End call | Audio paths properly torn down | P1 |
| IT-BTA-006 | Buffer overflow | Slow consumer | Overflow logged, system continues | P2 |
| IT-BTA-007 | Buffer underflow | Fast consumer | Underflow logged, no crash | P2 |

### 4.2 Phone Interface + State Integration

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| IT-PHS-001 | Off-hook state | Lift handset | PHONE_STATE_OFF_HOOK set | P1 |
| IT-PHS-002 | Off-hook dial tone | Lift handset | Dial tone starts within 200ms | P1 |
| IT-PHS-003 | On-hook state | Replace handset | PHONE_STATE_OFF_HOOK cleared | P1 |
| IT-PHS-004 | On-hook tone stop | Replace handset during tone | Tone stops immediately | P1 |
| IT-PHS-005 | Ring state | Incoming call | PHONE_STATE_RINGING set | P1 |
| IT-PHS-006 | Ring to connected | Answer during ring | PHONE_STATE_RINGING cleared, call active | P1 |

### 4.3 Bluetooth + State Integration

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| IT-BTS-001 | Connect state | BT device connects | BT_STATE_CONNECTED set | P1 |
| IT-BTS-002 | Disconnect state | BT device disconnects | BT_STATE_CONNECTED cleared | P1 |
| IT-BTS-003 | Call state | Incoming/outgoing call | BT_STATE_IN_CALL set | P1 |
| IT-BTS-004 | Audio state | Audio connection established | BT_STATE_AUDIO_CONNECTED set | P1 |
| IT-BTS-005 | Volume sync | Volume change from phone | Volume updated in state | P2 |
| IT-BTS-006 | Battery level | Battery update from phone | Battery level updated in state | P3 |
| IT-BTS-007 | Signal strength | Signal update from phone | Signal strength updated in state | P3 |

### 4.4 WiFi + Web Integration

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| IT-WFW-001 | Web server starts | WiFi connects | Web server accessible on port 80 | P2 |
| IT-WFW-002 | WiFi disconnect | WiFi disconnects | Web server stops gracefully | P2 |
| IT-WFW-003 | WiFi reconnect | WiFi reconnects | Web server resumes | P2 |
| IT-WFW-004 | Status reflects WiFi | Check /status | WiFi state matches actual connection | P2 |

### 4.5 Event System Integration

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| IT-EVT-001 | BT connect event | BT connects | BT_EVENT_CONNECTED published | P1 |
| IT-EVT-002 | BT disconnect event | BT disconnects | BT_EVENT_DISCONNECTED published | P1 |
| IT-EVT-003 | Off-hook event | Lift handset | PHONE_EVENT_OFF_HOOK published | P1 |
| IT-EVT-004 | On-hook event | Replace handset | PHONE_EVENT_ON_HOOK published | P1 |
| IT-EVT-005 | Call started event | Call begins | BT_EVENT_CALL_STARTED published | P1 |
| IT-EVT-006 | Call ended event | Call ends | BT_EVENT_CALL_ENDED published | P1 |
| IT-EVT-007 | WiFi connected event | WiFi connects | WIFI_EVENT_CONNECTED_EV published | P2 |

---

## 5. System Test Cases

### 5.1 Inbound Call Scenarios

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| ST-INC-001 | Basic inbound call | Receive call, answer, talk, hang up | Full call cycle completes | P1 |
| ST-INC-002 | Inbound ring | Receive call, verify ringing | Ringer activates with 2s/4s cadence | P1 |
| ST-INC-003 | Inbound answer | Answer during ring | Ringing stops, audio established | P1 |
| ST-INC-004 | Inbound reject | Don't answer, caller hangs up | Ringing stops, system returns to idle | P1 |
| ST-INC-005 | Inbound audio quality | Voice conversation | Audio clear, no echo, no dropouts | P1 |
| ST-INC-006 | Remote hangup | Remote party hangs up | Call ends, audio stops, idle state | P1 |
| ST-INC-007 | Local hangup | Replace handset during call | Call ends, hangup sent to BT | P1 |
| ST-INC-008 | Multiple rings | Let phone ring multiple times | Ring count increments correctly | P2 |

### 5.2 Outbound Call Scenarios

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| ST-OUT-001 | Basic outbound call | Dial number, connect, talk, hang up | Full call cycle completes | P1 |
| ST-OUT-002 | Dial tone | Lift handset | Dial tone plays within 200ms | P1 |
| ST-OUT-003 | Dial tone stop | Enter first digit | Dial tone stops | P1 |
| ST-OUT-004 | Ringback tone | Call ringing at remote | Ringback tone plays | P1 |
| ST-OUT-005 | Call connected | Remote answers | Audio path established | P1 |
| ST-OUT-006 | Busy signal | Call busy number | Busy tone plays | P1 |
| ST-OUT-007 | Dial timeout | Lift handset, don't dial | Reorder tone after timeout | P2 |
| ST-OUT-008 | Hangup before answer | Hang up during ringback | Call cancelled, system idle | P2 |

### 5.3 Bluetooth Connectivity Scenarios

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| ST-BT-001 | Initial pairing | Pair new device | Pairing successful, device stored | P1 |
| ST-BT-002 | Reconnection | Power cycle, phone available | Automatic reconnection within 30s | P1 |
| ST-BT-003 | Reconnect after loss | Move phone out/in range | Reconnection occurs | P1 |
| ST-BT-004 | Multiple reconnect | Repeated disconnect/reconnect | Stable reconnection each time | P2 |
| ST-BT-005 | Different phone | Pair different phone | New device replaces old | P2 |
| ST-BT-006 | No phone available | Boot with no phone in range | Continues trying, no crash | P2 |

### 5.4 Error Recovery Scenarios

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| ST-ERR-001 | BT loss during call | Disconnect BT during call | Call ends gracefully, recovery attempted | P1 |
| ST-ERR-002 | WiFi loss | Disconnect WiFi | System continues, web unavailable | P2 |
| ST-ERR-003 | Audio buffer issue | Cause buffer overflow | Logged, audio recovers | P2 |
| ST-ERR-004 | Long off-hook | Leave phone off-hook | Warning tone plays | P2 |
| ST-ERR-005 | Rapid on/off hook | Quickly lift/replace handset | Debouncing works, no false triggers | P2 |

### 5.5 User Experience Scenarios

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| ST-UX-001 | Normal phone behavior | Use as regular phone | Feels like traditional phone line | P1 |
| ST-UX-002 | Status via web | Check /status during operation | Accurate real-time status | P2 |
| ST-UX-003 | Multiple calls | Make/receive multiple calls | Each call works correctly | P1 |
| ST-UX-004 | Extended use | Operate for 24 hours | No degradation or memory leaks | P2 |

---

## 6. Performance Test Cases

### 6.1 Timing Requirements

| Test ID | Test Name | Description | Acceptance Criteria | Priority |
|---------|-----------|-------------|---------------------|----------|
| PT-TIM-001 | Off-hook latency | Measure time from GPIO change to state update | < 100ms including debounce | P1 |
| PT-TIM-002 | Dial tone latency | Measure time from off-hook to tone start | < 200ms | P1 |
| PT-TIM-003 | Call answer latency | Measure time from off-hook to call answer | < 500ms | P1 |
| PT-TIM-004 | Audio latency | Measure end-to-end audio delay | < 100ms | P1 |
| PT-TIM-005 | BT reconnect time | Measure reconnection time | < 30s when device available | P2 |
| PT-TIM-006 | Web response time | Measure /status response time | < 500ms | P3 |

### 6.2 Resource Utilization

| Test ID | Test Name | Description | Acceptance Criteria | Priority |
|---------|-----------|-------------|---------------------|----------|
| PT-RES-001 | Idle CPU usage | Measure CPU when idle | < 10% | P2 |
| PT-RES-002 | Call CPU usage | Measure CPU during call | < 50% | P1 |
| PT-RES-003 | Tone CPU usage | Measure CPU during tone playback | < 20% | P2 |
| PT-RES-004 | Memory baseline | Measure DRAM usage at startup | < 150KB | P2 |
| PT-RES-005 | Memory during call | Measure DRAM during active call | < 180KB | P2 |
| PT-RES-006 | Stack high water | Check task stack usage | > 20% headroom each task | P2 |

### 6.3 Stress Tests

| Test ID | Test Name | Description | Acceptance Criteria | Priority |
|---------|-----------|-------------|---------------------|----------|
| PT-STR-001 | Continuous calls | 100 back-to-back calls | All complete successfully | P2 |
| PT-STR-002 | Long call duration | Single call for 4 hours | No audio degradation | P2 |
| PT-STR-003 | Rapid reconnects | 50 BT disconnect/reconnect cycles | All reconnections successful | P2 |
| PT-STR-004 | Web hammering | 1000 /status requests in 1 minute | All return valid JSON | P3 |
| PT-STR-005 | 24-hour soak | Run system for 24 hours | No crashes, memory leaks, or degradation | P1 |

---

## 7. Hardware Test Cases

### 7.1 GPIO Verification

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| HT-GPIO-001 | Off-hook input | Apply signal to GPIO 32 | Correct logic level read | P1 |
| HT-GPIO-002 | Ring detect input | Apply signal to GPIO 33 | Correct logic level read | P2 |
| HT-GPIO-003 | Pulse dial input | Apply pulses to GPIO 34 | Pulses counted correctly | P2 |
| HT-GPIO-004 | Ring command output | Trigger ring | GPIO 13 toggles correctly | P2 |
| HT-GPIO-005 | Dial lamp output | Trigger lamp | GPIO 27 toggles correctly | P3 |
| HT-GPIO-006 | Status LED output | Trigger LED | GPIO 2 toggles correctly | P3 |

### 7.2 I2S Audio Verification

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| HT-I2S-001 | I2S TX signal | Generate tone, measure DOUT | Correct waveform on GPIO 26 | P1 |
| HT-I2S-002 | I2S RX signal | Apply signal to DIN | Correct data received on GPIO 35 | P1 |
| HT-I2S-003 | Bit clock | Verify BCLK frequency | Correct frequency on GPIO 5 | P1 |
| HT-I2S-004 | Frame sync | Verify FSYNC frequency | Correct frequency on GPIO 25 | P1 |
| HT-I2S-005 | DAC output | Measure analog output | Clean audio signal | P1 |
| HT-I2S-006 | ADC input | Apply analog signal | Correct digital conversion | P1 |

### 7.3 SLIC Interface Verification

| Test ID | Test Name | Description | Expected Result | Priority |
|---------|-----------|-------------|-----------------|----------|
| HT-SLIC-001 | Hook switch detect | Operate hook switch | Correct detection | P1 |
| HT-SLIC-002 | Debounce function | Rapid hook switch | No false triggers | P1 |
| HT-SLIC-003 | Ringer output | Trigger ring | Ringer activates | P1 |
| HT-SLIC-004 | Audio to handset | Play tone | Audio in earpiece | P1 |
| HT-SLIC-005 | Audio from handset | Speak into mic | Audio captured | P1 |

---

## 8. Test Procedures

### 8.1 Unit Test Execution

```bash
# Run all unit tests
idf.py -p /dev/ttyUSB0 flash monitor

# Tests execute automatically on boot when CONFIG_UNITY_ENABLE=y
# Look for "PASS" or "FAIL" in serial output
```

### 8.2 Manual System Test Procedure

**Prerequisites:**
1. ESP32 flashed with release firmware
2. Analog telephone connected
3. Bluetooth phone paired
4. WiFi credentials configured

**Inbound Call Test (ST-INC-001):**
1. Verify system is idle (phone on-hook, BT connected)
2. Initiate call to paired phone from another number
3. Verify ringer activates
4. Lift handset
5. Verify audio connection
6. Speak and verify bidirectional audio
7. Replace handset
8. Verify system returns to idle

**Outbound Call Test (ST-OUT-001):**
1. Verify system is idle
2. Lift handset
3. Verify dial tone within 200ms
4. Dial test number
5. Verify ringback tone
6. Wait for answer
7. Verify bidirectional audio
8. Replace handset
9. Verify system returns to idle

### 8.3 Performance Test Procedure

**Latency Measurement (PT-TIM-004):**
1. Connect oscilloscope to I2S TX and phone speaker output
2. Generate known signal via Bluetooth
3. Measure time between I2S data and analog output
4. Calculate total path latency
5. Repeat 10 times and average

---

## 9. Test Data and Environment

### 9.1 Test Phone Numbers

| Purpose | Number | Description |
|---------|--------|-------------|
| Ringback test | 1-800-555-1212 | AT&T directory assistance |
| Busy signal test | Local busy number | Configured test number |
| Voicemail test | Carrier voicemail | Paired phone voicemail |

### 9.2 Test Equipment

| Equipment | Model | Purpose |
|-----------|-------|---------|
| Oscilloscope | Rigol DS1054Z | Timing, signal analysis |
| Multimeter | Fluke 87V | Voltage, continuity |
| Audio analyzer | PC + Audacity | Frequency analysis |
| Bluetooth phone | iPhone/Android | Call testing |
| Analog telephone | ITT Trimline | End-to-end testing |

### 9.3 WiFi Test Networks

| SSID | Purpose |
|------|---------|
| TestNetwork_2.4GHz | Normal operation testing |
| TestNetwork_Weak | Weak signal testing |
| TestNetwork_Congested | High-traffic testing |

---

## 10. Pass/Fail Criteria

### 10.1 Unit Tests
- **Pass:** All assertions pass, no memory leaks detected
- **Fail:** Any assertion fails, memory leak detected, or timeout

### 10.2 Integration Tests
- **Pass:** All subsystem interactions work as specified
- **Fail:** Any interaction fails or produces unexpected state

### 10.3 System Tests
- **Pass:** End-to-end scenario completes successfully with acceptable quality
- **Fail:** Scenario fails, audio quality unacceptable, or system crash

### 10.4 Performance Tests
- **Pass:** All metrics within acceptance criteria
- **Fail:** Any metric outside acceptance criteria

### 10.5 Release Criteria
- All P1 tests must pass
- 95% of P2 tests must pass
- 80% of P3 tests must pass
- No known P1 defects
- 24-hour soak test passes

---

## 11. Defect Tracking

### 11.1 Severity Levels

| Level | Description | Example |
|-------|-------------|---------|
| Critical | System unusable | Cannot make/receive calls |
| High | Major feature broken | Dial tone not playing |
| Medium | Feature partially broken | Occasional audio dropout |
| Low | Minor issue | Web status refresh slow |
| Cosmetic | Non-functional issue | Log message formatting |

### 11.2 Defect Workflow

```
New → Confirmed → Assigned → In Progress → Fixed → Verified → Closed
                         ↓
                    Cannot Reproduce
                         ↓
                      Closed
```

---

## 12. Test Schedule

| Phase | Activities | Duration |
|-------|-----------|----------|
| Unit Test Development | Write unit tests for all modules | Ongoing |
| Integration Testing | Test subsystem interactions | Per feature |
| System Testing | End-to-end scenarios | Before release |
| Performance Testing | Timing and stress tests | Before release |
| Regression Testing | Re-run all tests after changes | Per release |
| Acceptance Testing | Final verification | Before deployment |

---

## 13. Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-22 | Generated | Initial test plan based on codebase analysis |
