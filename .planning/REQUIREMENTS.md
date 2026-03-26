# Requirements: FreeSWITCH ASR/TTS Modules

**Defined:** 2026-03-26
**Core Value:** Provide high-performance, dynamically switchable, auto-fallback capable real-time speech recognition and synthesis for intelligent call centers.

## v1 Requirements

Requirements for initial release. Each maps to roadmap phases.

### ASR Core

- [ ] **ASR-01**: System can perform streaming speech recognition with Aliyun Cloud ASR
- [ ] **ASR-02**: System can detect voice activity (VAD) to distinguish speech from silence
- [ ] **ASR-03**: System can publish ASR results via ESL custom events for Java business system integration
- [ ] **ASR-04**: System can handle 8kHz G.711 (PCMU/PCMA) telephony audio with proper resampling to 16kHz for ASR
- [ ] **ASR-05**: System can be configured via XML configuration file in FreeSWITCH autoload_configs directory
- [ ] **ASR-06**: System can be loaded/unloaded via standard FreeSWITCH module API

### TTS

- [ ] **TTS-01**: System can perform speech synthesis with Aliyun Cloud TTS
- [ ] **TTS-02**: System can stream TTS audio to reduce first-packet latency

### Advanced

- [ ] **ADV-01**: User can interrupt TTS playback by speaking (Barge-In support)

## v2 Requirements

Deferred to future release. Tracked but not in current roadmap.

### Local Providers

- **ASR-07**: System can perform streaming speech recognition with local FunASR engine
- **TTS-03**: System can perform speech synthesis with local sherpa-onnx engine

### Reliability

- **ADV-02**: System can automatically switch between cloud and local ASR providers on failure
- **ADV-03**: System can automatically switch between cloud and local TTS providers on failure
- **ADV-04**: System can monitor provider health status with periodic checks
- **ADV-05**: System can dynamically modify configuration via API/ESL without module reload

## Out of Scope

Explicitly excluded. Documented to prevent scope creep.

| Feature | Reason |
|---------|--------|
| MRCP Protocol Support | Direct API calls chosen for lower latency |
| Non-Linux Platform Support | Production target is Debian 12 only |
| Script Language Modules | C modules only for performance |
| Real-Time Web Dashboard | Belongs in Java business system |
| Built-in LLM Integration | Belongs in business logic layer |
| Noise-Aware VAD | Commercial grade, defer to v2+ |
| Recording with ASR | QA feature, not core functionality |
| Hotwords/Keywords Boosting | Domain-specific enhancement, defer to v2+ |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

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
*Requirements defined: 2026-03-26*
*Last updated: 2026-03-26 after initial definition*