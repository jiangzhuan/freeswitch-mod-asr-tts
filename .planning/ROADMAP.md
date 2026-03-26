# Roadmap: FreeSWITCH ASR/TTS Modules

**Created:** 2026-03-26
**Phases:** 3
**Granularity:** standard

## Phase Overview

| # | Phase | Goal | Requirements | Success Criteria |
|---|-------|------|--------------|-------------------|
| 1 | ASR Core Module | Users can perform streaming speech recognition with cloud ASR | ASR-01, ASR-02, ASR-03, ASR-04, ASR-05, ASR-06 | 5 criteria |
| 2 | TTS Module | System can generate speech output with low latency | TTS-01, TTS-02 | 2 criteria |
| 3 | Barge-In | Users can interrupt TTS playback by speaking | ADV-01 | 2 criteria |

## Phases

- [ ] **Phase 1: ASR Core Module** - Establish complete ASR infrastructure with Aliyun Cloud streaming recognition
- [ ] **Phase 2: TTS Module** - Add TTS capability with streaming playback for low latency
- [ ] **Phase 3: Barge-In** - Enable user interruption during TTS playback

---

## Phase Details

### Phase 1: ASR Core Module

**Goal:** Users can perform streaming speech recognition with Aliyun Cloud ASR, integrating with Java business system via ESL

**Requirements:**
- ASR-01: System can perform streaming speech recognition with Aliyun Cloud ASR
- ASR-02: System can detect voice activity (VAD) to distinguish speech from silence
- ASR-03: System can publish ASR results via ESL custom events for Java business system integration
- ASR-04: System can handle 8kHz G.711 (PCMU/PCMA) telephony audio with proper resampling to 16kHz for ASR
- ASR-05: System can be configured via XML configuration file in FreeSWITCH autoload_configs directory
- ASR-06: System can be loaded/unloaded via standard FreeSWITCH module API

**Success Criteria** (what must be TRUE):
1. User can make a call and speak, and see ASR results appear in real-time (incremental recognition)
2. System distinguishes speech from silence - no ASR events are sent during silent periods
3. Java business system receives ASR results via ESL custom `asr` events with proper JSON payload
4. System correctly handles standard telephony audio (8kHz G.711) without errors or distortion
5. Administrator can configure the module via XML config file and load/unload it via FreeSWITCH CLI

**Dependencies:** None (first phase)

**Estimated Complexity:** HIGH

**Plans:** 3 plans

Plans:
- [ ] 01-01-PLAN.md — Module skeleton with lifecycle and configuration
- [ ] 01-02-PLAN.md — Audio pipeline: media bug, ring buffer, resampling
- [ ] 01-03-PLAN.md — ASR core: WebSocket client, VAD, ESL events, worker pool

---

### Phase 2: TTS Module

**Goal:** System can generate speech output with streaming playback for reduced first-packet latency

**Requirements:**
- TTS-01: System can perform speech synthesis with Aliyun Cloud TTS
- TTS-02: System can stream TTS audio to reduce first-packet latency

**Success Criteria** (what must be TRUE):
1. System can synthesize Chinese text to speech using Aliyun Cloud TTS and play it to the caller
2. TTS audio playback starts within 500ms of receiving text (streaming playback, not waiting for complete audio)

**Dependencies:** Phase 1 (requires established module infrastructure and audio pipeline)

**Estimated Complexity:** MEDIUM

---

### Phase 3: Barge-In

**Goal:** Users can interrupt TTS playback by speaking, enabling natural conversation flow

**Requirements:**
- ADV-01: User can interrupt TTS playback by speaking (Barge-In support)

**Success Criteria** (what must be TRUE):
1. During TTS playback, when user speaks, TTS audio stops immediately (within 200ms of voice detection)
2. ASR continues to recognize the interrupting speech after TTS stops

**Dependencies:** Phase 1 (ASR with VAD), Phase 2 (TTS playback)

**Estimated Complexity:** MEDIUM

---

## Requirement Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| ASR-01 | Phase 1 | Pending |
| ASR-02 | Phase 1 | Pending |
| ASR-03 | Phase 1 | Pending |
| ASR-04 | Phase 1 | Pending |
| ASR-05 | Phase 1 | Pending |
| ASR-06 | Phase 1 | Pending |
| TTS-01 | Phase 2 | Pending |
| TTS-02 | Phase 2 | Pending |
| ADV-01 | Phase 3 | Pending |

**Coverage:**
- v1 requirements: 9 total
- Mapped to phases: 9
- Unmapped: 0 ✓

---

## Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. ASR Core Module | 0/3 | Planned | - |
| 2. TTS Module | 0/0 | Not started | - |
| 3. Barge-In | 0/0 | Not started | - |

---

## Phase Ordering Rationale

1. **Phase 1 (ASR Core) first:** All other features depend on working ASR infrastructure. This phase addresses critical pitfalls: audio codec mismatch, memory leaks, WebSocket state handling, blocking ASR calls, missing VAD, and incorrect ESL events.

2. **Phase 2 (TTS) before Phase 3:** TTS is a separate subsystem; implementing it validates the provider abstraction pattern and establishes the audio playback pipeline needed for barge-in.

3. **Phase 3 (Barge-In) last:** Requires both ASR (with VAD) and TTS working together. Can't interrupt TTS that doesn't exist, and need VAD to detect interruption.

---

## v2 Roadmap Preview

Future phases planned for v2 (not in current scope):

| Phase | Goal | Requirements |
|-------|------|--------------|
| 4 | Local ASR Provider | ASR-07 |
| 5 | Multi-Provider Failover | ADV-02, ADV-03, ADV-04 |
| 6 | Local TTS Provider | TTS-03 |
| 7 | Dynamic Configuration | ADV-05 |

---

*Roadmap created: 2026-03-26*
*Phase 1 planned: 2026-03-26*
*Ready for execution: `/gsd-execute-phase 1`*