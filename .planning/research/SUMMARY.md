# Project Research Summary

**Project:** FreeSWITCH ASR/TTS Module for Intelligent Call Center
**Domain:** FreeSWITCH Module Development / Speech Processing
**Researched:** 2026-03-26
**Confidence:** HIGH

## Executive Summary

This project is a FreeSWITCH module that provides ASR (speech recognition) and TTS (speech synthesis) capabilities for intelligent call centers. The module integrates with cloud services (Alibaba Cloud) for quality and local services (FunASR, sherpa-onnx) for reliability and offline operation. Experts in this domain build such modules using FreeSWITCH's native C/C++ module API with direct WebSocket integration to ASR/TTS providers, avoiding the MRCP protocol for lower latency.

The recommended approach is a phased implementation starting with core ASR infrastructure using Alibaba Cloud, then adding TTS capabilities, and finally implementing multi-provider failover. The module uses a Provider Abstraction Pattern to enable seamless switching between cloud and local services. Key architecture decisions include using FreeSWITCH's media bug mechanism for audio capture, ESL custom events for Java integration, and asynchronous processing to avoid blocking the audio path.

The key risks are audio codec mismatch (8kHz telephony vs 16kHz ASR), memory leaks in streaming audio processing, and WebSocket connection state management. These are mitigated through proper resampling using FreeSWITCH's built-in utilities, memory pool management with paired create/destroy calls, and implementing connection state machines with reconnection logic.

## Key Findings

### Recommended Stack

The stack centers on FreeSWITCH 1.10.12 with C++ modules for ASR/TTS integration. Cloud services provide best quality while local services ensure reliability. The architecture uses direct API calls (not MRCP) for lower latency.

**Core technologies:**
- **FreeSWITCH 1.10.12** — Telephony platform — Latest stable release (Aug 2024), well-documented module API
- **GCC/G++ 12+** — C/C++ compiler — Required for C++17 features and Debian 12 compatibility
- **ONNX Runtime 1.17+** — ML inference engine — Shared backend for both FunASR and sherpa-onnx local inference
- **Alibaba Cloud NLS SDK** — Cloud ASR — Streaming recognition with WebSocket API
- **FunASR Runtime** — Local ASR — Chinese-optimized streaming ASR with paraformer-zh-streaming model
- **sherpa-onnx** — Local TTS — Offline TTS with C API for direct FreeSWITCH integration
- **mod_event_socket** — ESL integration — Standard pattern for Java business system communication

### Expected Features

Features are categorized into table stakes (must have for MVP), differentiators (competitive advantage), and deferred items. The dependency analysis shows VAD is foundational for ASR, and multi-provider support requires health monitoring.

**Must have (table stakes):**
- **Streaming ASR** — Real-time recognition with chunked audio input and incremental results
- **VAD (Voice Activity Detection)** — Detect speech vs silence, prevent sending silence to ASR
- **ESL Event Output** — Custom `asr` events with JSON payload for Java integration
- **Basic TTS** — Speech synthesis for system responses
- **Audio Codec Support** — Handle 8kHz G.711 (PCMU/PCMA) with resampling to 16kHz
- **XML Configuration** — FreeSWITCH standard configuration in `autoload_configs/`
- **Module Lifecycle** — Standard load/unload via FreeSWITCH module API

**Should have (competitive):**
- **Multi-Provider ASR/TTS** — Cloud-first with local fallback for reliability
- **Barge-In Support** — User can interrupt TTS, requires VAD during playback
- **Service Health Monitoring** — Auto-detect provider availability, trigger failover
- **Dynamic Configuration** — Runtime config changes without module reload

**Defer (v2+):**
- **Noise-Aware VAD** — Better accuracy in noisy environments (commercial VAD achieves 99.9%)
- **Hotwords/Keywords Boosting** — Domain-specific term recognition improvement
- **Recording with ASR** — Save speech for quality assurance and training

### Architecture Approach

The module follows FreeSWITCH's modular design pattern with clear separation between ASR and TTS subsystems. Each subsystem uses a Provider Abstraction Pattern to enable transparent switching between implementations. Audio capture uses FreeSWITCH's media bug mechanism for non-blocking operation, and results are published via ESL custom events.

**Major components:**
1. **Module Entry Point (mod_asr_tts.c)** — Lifecycle management, interface registration to FreeSWITCH core
2. **ASR Subsystem (asr/)** — ASR interface implementation, provider abstraction, cloud (Alibaba) and local (FunASR) providers
3. **TTS Subsystem (tts/)** — TTS interface implementation, provider abstraction, cloud (Alibaba) and local (sherpa-onnx) providers
4. **Core Utilities (core/)** — Configuration parsing, audio buffer management, ESL event publishing
5. **Health Monitor (health/)** — Background service health checking, failover state machine
6. **WebSocket Client (providers/)** — Reusable WebSocket connection handling for cloud services

### Critical Pitfalls

Research identified 8 critical pitfalls specific to FreeSWITCH ASR/TTS module development. These are ordered by phase impact.

1. **Audio Codec Mismatch** — ASR expects 16kHz but telephony uses 8kHz. Always resample using `switch_resample_create/process`. Verify format at initialization.

2. **Memory Leaks in Audio Buffer Processing** — Streaming ASR requires continuous buffer allocation. Use FreeSWITCH memory pools, pair every `*_create()` with `*_destroy()`. Test with valgrind.

3. **WebSocket Connection State Mishandling** — Connections drop silently. Implement state machine (CONNECTING→CONNECTED→DISCONNECTED→RECONNECTING), exponential backoff, and heartbeat frames.

4. **Blocking ASR API Calls in Audio Thread** — Never block in `switch_io_read_frame()`. Use worker thread pool, async WebSocket operations. Media thread must return within ~20ms.

5. **Missing VAD Integration** — Sending continuous audio including silence wastes quota and causes false positives. Implement VAD before ASR, only send voice-active segments.

6. **Incorrect ESL Event Handling** — Wrong event format causes Java system to miss results. Use `SWITCH_EVENT_CUSTOM` with `asr` subclass, include `Channel` and `ASR-Response` headers.

7. **TTS Audio Playback Blocking on Network** — Waiting for complete audio causes 1-2s delay. Implement streaming playback with pre-buffering (200-500ms before starting).

8. **Service Failover Without Health Checking** — Fallback on first error or never recovers. Implement hysteresis (N consecutive failures), periodic recovery checks.

## Implications for Roadmap

Based on research, suggested phase structure:

### Phase 1: ASR Core Module
**Rationale:** Foundation for all ASR functionality. Must establish correct audio pipeline, memory management, and threading patterns before adding features.
**Delivers:** Working streaming ASR with Alibaba Cloud, VAD integration, ESL event output
**Addresses:** Streaming ASR, VAD, ESL Event Output, XML Configuration, Module Lifecycle (table stakes)
**Avoids:** Audio Codec Mismatch, Memory Leaks, WebSocket State Mishandling, Blocking ASR Calls, Missing VAD, Incorrect ESL Events (pitfalls 1-6)
**Stack elements:** FreeSWITCH module API, Alibaba Cloud NLS SDK, cJSON, libcurl
**Architecture:** Module entry point, ASR interface, provider abstraction, media bug integration, core utilities

### Phase 2: TTS Module
**Rationale:** TTS depends on established module infrastructure. Can reuse provider abstraction pattern from Phase 1. Requires separate attention to streaming playback.
**Delivers:** Working TTS with Alibaba Cloud, streaming audio playback
**Addresses:** Basic TTS (table stakes)
**Avoids:** TTS Audio Playback Blocking (pitfall 7)
**Stack elements:** Alibaba Cloud TTS SDK, WebSocket streaming
**Architecture:** TTS interface, TTS provider abstraction, streaming playback buffer

### Phase 3: Local ASR Provider
**Rationale:** Once cloud ASR works, add local fallback. Reuses ASR interface from Phase 1.
**Delivers:** FunASR integration as local ASR option
**Addresses:** Multi-Provider ASR (competitive feature)
**Stack elements:** FunASR Runtime, ONNX Runtime, paraformer-zh-streaming model
**Architecture:** FunASR provider implementation, HTTP/gRPC client

### Phase 4: Multi-Provider with Failover
**Rationale:** Health monitoring and automatic failover require both providers to be working. Phase 3 enables this.
**Delivers:** Automatic switching between cloud and local ASR based on health
**Addresses:** Multi-Provider ASR, Service Health Monitoring (competitive features)
**Avoids:** Service Failover Without Health Checking (pitfall 8)
**Architecture:** Provider manager, health monitor background thread, failover state machine

### Phase 5: Local TTS Provider
**Rationale:** Cost optimization for TTS. Lower priority than ASR reliability.
**Delivers:** sherpa-onnx integration as local TTS option
**Addresses:** Multi-Provider TTS (competitive feature)
**Stack elements:** sherpa-onnx, ONNX Runtime
**Architecture:** sherpa-onnx TTS provider implementation

### Phase 6: Barge-In and Enhancements
**Rationale:** Barge-in requires both ASR and TTS working. Enhancements add polish.
**Delivers:** User interruption during TTS, dynamic configuration, timestamps
**Addresses:** Barge-In Support, Dynamic Configuration (competitive features)
**Architecture:** VAD during TTS playback, runtime config reload

### Phase Ordering Rationale

- **Phase 1 first:** All other features depend on working ASR infrastructure. This phase addresses 6 of 8 critical pitfalls.
- **Phase 2 before Phase 3-5:** TTS is a separate subsystem; implementing it validates the provider abstraction pattern.
- **Phase 3 before Phase 4:** Multi-provider failover requires both providers working; can't test failover with one provider.
- **Phase 5 before Phase 6:** Barge-in requires both ASR and TTS working; can't interrupt TTS that doesn't exist.

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 3 (Local ASR):** FunASR integration specifics — need to research HTTP/gRPC client implementation, model loading, and streaming protocol
- **Phase 4 (Multi-Provider):** Health check strategies — need to define health check intervals, failure thresholds, and recovery criteria
- **Phase 5 (Local TTS):** sherpa-onnx integration — need to research C API usage, model format, and audio output format

Phases with standard patterns (skip research-phase):
- **Phase 1 (ASR Core):** Well-documented FreeSWITCH module patterns, reference implementation (cdevelop/FreeSWITCH-ASR) available
- **Phase 2 (TTS Module):** Similar patterns to ASR, established WebSocket streaming approach

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Official FreeSWITCH docs, verified GitHub repos (FunASR 15.4k stars, sherpa-onnx 11.1k stars), production reference (easycallcenter365) |
| Features | HIGH | Clear table stakes identified, competitor feature analysis completed, dependency graph established |
| Architecture | HIGH | FreeSWITCH modular design pattern well-documented, reference implementations analyzed |
| Pitfalls | HIGH | Based on official documentation, reference implementations, and production project analysis |

**Overall confidence:** HIGH

### Gaps to Address

Areas requiring attention during implementation:

- **FunASR C++ SDK details:** FunASR has Python bindings well-documented, but C++ runtime specifics need validation during Phase 3 implementation
- **Alibaba Cloud token refresh:** Research shows token expiration handling is needed, but exact refresh timing and implementation needs testing
- **Scaling thresholds:** Connection pooling limits, thread pool sizing need empirical testing at scale (100+ concurrent calls)

## Sources

### Primary (HIGH confidence)
- **FreeSWITCH Official Documentation** — https://developer.signalwire.com/freeswitch/ — Module API, interfaces, patterns
- **FreeSWITCH GitHub** — https://github.com/signalwire/freeswitch — Source code, version 1.10.12 verified
- **FunASR GitHub** — https://github.com/modelscope/FunASR — Local ASR, 15.4k stars, streaming support verified
- **sherpa-onnx GitHub** — https://github.com/k2-fsa/sherpa-onnx — Local TTS, 11.1k stars, C API verified

### Secondary (MEDIUM confidence)
- **cdevelop/FreeSWITCH-ASR** — https://github.com/cdevelop/FreeSWITCH-ASR — Reference implementation for Alibaba Cloud ASR integration with VAD
- **easycallcenter365** — https://github.com/easycallcenter365/easycallcenter365 — Production call center with mod_funasr, mod_aliyun_tts
- **wangkaisine/mrcp-plugin-with-freeswitch** — https://github.com/wangkaisine/mrcp-plugin-with-freeswitch — MRCP approach reference (project excludes MRCP)

### Tertiary (LOW confidence)
- **Scaling thresholds** — Estimates based on general patterns, need validation during load testing
- **Specific timeout values** — Generic recommendations (5s connect, 30s idle), need tuning for specific deployment

---
*Research completed: 2026-03-26*
*Ready for roadmap: yes*