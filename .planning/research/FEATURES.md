# Feature Research

**Domain:** FreeSWITCH ASR/TTS Module for Intelligent Call Center
**Researched:** 2026-03-26
**Confidence:** HIGH

## Feature Landscape

### Table Stakes (Users Expect These)

Features users assume exist. Missing these = product feels incomplete.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| **Streaming ASR** | Real-time recognition is core to intelligent call center; users speak and expect immediate response | HIGH | Must support chunked audio input and incremental results |
| **VAD (Voice Activity Detection)** | Need to know when user is speaking vs silence; prevents sending silence to ASR | MEDIUM | Should integrate with ASR pipeline for efficiency |
| **ESL Event Output** | Business systems need to receive ASR results; FreeSWITCH standard integration pattern | MEDIUM | Custom event `asr` with JSON payload |
| **Basic TTS** | Call center needs to respond to users; speech synthesis is essential | MEDIUM | Support at minimum one provider (Aliyun) |
| **Audio Codec Support** | Telephony audio is 8kHz; must handle G.711 (PCMU/PCMA) | LOW | FreeSWITCH provides transcoding; module focuses on 8kHz mono |
| **XML Configuration** | FreeSWITCH standard configuration format; operators expect it | LOW | Module config file in `autoload_configs/` |
| **Module Lifecycle** | Load/unload via `load mod_xxx`; standard FreeSWITCH module behavior | LOW | Implement `SWITCH_MODULE_LOAD_FUNCTION` and shutdown |

### Differentiators (Competitive Advantage)

Features that set the product apart. Not required, but valuable.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| **Multi-Provider ASR** | Cloud quality (Aliyun) with local fallback (FunASR); ensures availability | HIGH | Cloud-first, auto-fallback to local when cloud unavailable |
| **Multi-Provider TTS** | Cloud quality (Aliyun) with local fallback (sherpa-onnx); cost optimization | HIGH | Similar failover pattern to ASR |
| **Noise-Aware VAD** | Distinguish human speech from background noise; critical in call centers | HIGH | Commercial VAD (like DDRJ libsad) achieves 99.9% accuracy for >1s speech |
| **Barge-In Support** | User can interrupt TTS; natural conversation flow | HIGH | Requires VAD integration during TTS playback |
| **Dynamic Configuration** | Change provider/settings without module reload; operational flexibility | MEDIUM | API/ESL commands to modify config at runtime |
| **Service Health Monitoring** | Auto-detect provider availability; trigger failover | MEDIUM | Health checks, retry logic, circuit breaker |
| **Recording with ASR** | Save user speech for quality assurance, training, dispute resolution | LOW | Parallel recording during ASR session |
| **Timestamps** | Know when each word was spoken; useful for analytics and debugging | MEDIUM | FunASR and Aliyun both support timestamps |
| **Hotwords/Keywords Boosting** | Improve recognition of domain-specific terms (product names, etc.) | MEDIUM | WFST-based hotword support in FunASR |

### Anti-Features (Commonly Requested, Often Problematic)

Features that seem good but create problems.

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| **MRCP Protocol Support** | Industry standard, supports multiple vendors | Adds protocol overhead; increases latency; requires UniMRCP server; barge-in difficult via MRCP | Direct API calls (already decided in PROJECT.md) |
| **Multi-Platform Support** | Broader deployment options | Increases testing burden; Debian 12 is production target; focus resources on core functionality | Debian 12 only (project constraint) |
| **Script Language Modules** | Easier development, faster iteration | Performance penalty in high-concurrency call center; C is FreeSWITCH native | C modules only (project constraint) |
| **Real-Time Web Dashboard** | Operators want visibility | Module scope; belongs in separate Java business system | ESL integration provides data to external monitoring |
| **Built-in LLM Integration** | End-to-end AI conversation | Module scope creep; LLM belongs in business logic layer | ESL events feed Java system which handles LLM |

## Feature Dependencies

```
Streaming ASR
    ├──requires──> VAD (Voice Activity Detection)
    │                   └──requires──> Audio Codec Support (8kHz mono)
    ├──requires──> ESL Event Output
    │                   └──requires──> Module Lifecycle
    └──requires──> XML Configuration

Multi-Provider ASR
    ├──requires──> Streaming ASR (base capability)
    └──requires──> Service Health Monitoring

Barge-In Support
    ├──requires──> VAD (detect user speaking)
    └──requires──> Basic TTS (to be interrupted)

Dynamic Configuration
    └──requires──> XML Configuration (base config to modify)

Recording with ASR
    └──enhances──> Streaming ASR (parallel operation)
```

### Dependency Notes

- **Streaming ASR requires VAD:** VAD determines speech boundaries; essential for knowing when to send audio to ASR and when user finished speaking.
- **Multi-Provider ASR requires Service Health Monitoring:** Cannot auto-fallback without knowing provider health status.
- **Barge-In Support requires VAD:** Must detect user speaking during TTS playback to interrupt.
- **Dynamic Configuration requires XML Configuration:** Base configuration must exist to be dynamically modified.
- **Recording enhances Streaming ASR:** Can record while doing ASR; independent feature that adds value.

## MVP Definition

### Launch With (v1)

Minimum viable product — what's needed to validate the concept.

- [ ] **Streaming ASR (Aliyun)** — Core capability; user already has credentials to test
- [ ] **VAD** — Required for ASR to function properly
- [ ] **ESL Event Output** — Java business system integration
- [ ] **XML Configuration** — Basic module configuration
- [ ] **Module Lifecycle** — Standard FreeSWITCH module behavior

### Add After Validation (v1.x)

Features to add once core is working.

- [ ] **Basic TTS (Aliyun)** — Enable voice responses (trigger: ASR validated)
- [ ] **Streaming ASR (FunASR local)** — Local fallback provider (trigger: need for offline capability)
- [ ] **Multi-Provider ASR with fallback** — Reliability improvement (trigger: both providers working)
- [ ] **Service Health Monitoring** — Enable auto-failover (trigger: multi-provider implemented)
- [ ] **Barge-In Support** — Natural conversation flow (trigger: TTS working)

### Future Consideration (v2+)

Features to defer until product-market fit is established.

- [ ] **Basic TTS (sherpa-onnx local)** — Cost optimization for TTS
- [ ] **Noise-Aware VAD** — Better accuracy in noisy environments
- [ ] **Dynamic Configuration** — Operational flexibility
- [ ] **Recording with ASR** — Quality assurance capability
- [ ] **Timestamps** — Analytics and debugging support
- [ ] **Hotwords/Keywords** — Domain-specific recognition improvement

## Feature Prioritization Matrix

| Feature | User Value | Implementation Cost | Priority |
|---------|------------|---------------------|----------|
| Streaming ASR (Aliyun) | HIGH | HIGH | P1 |
| VAD | HIGH | MEDIUM | P1 |
| ESL Event Output | HIGH | MEDIUM | P1 |
| XML Configuration | HIGH | LOW | P1 |
| Module Lifecycle | HIGH | LOW | P1 |
| Basic TTS (Aliyun) | HIGH | MEDIUM | P2 |
| Streaming ASR (FunASR) | HIGH | HIGH | P2 |
| Multi-Provider ASR | MEDIUM | HIGH | P2 |
| Service Health Monitoring | MEDIUM | MEDIUM | P2 |
| Barge-In Support | HIGH | HIGH | P2 |
| Basic TTS (sherpa-onnx) | MEDIUM | HIGH | P3 |
| Noise-Aware VAD | MEDIUM | HIGH | P3 |
| Dynamic Configuration | LOW | MEDIUM | P3 |
| Recording with ASR | LOW | LOW | P3 |
| Timestamps | LOW | MEDIUM | P3 |
| Hotwords/Keywords | LOW | MEDIUM | P3 |

**Priority key:**
- P1: Must have for launch
- P2: Should have, add when possible
- P3: Nice to have, future consideration

## Competitor Feature Analysis

| Feature | cdevelop/FreeSWITCH-ASR | wangkaisine/mrcp-plugin | easycallcenter365 | Our Approach |
|---------|-------------------------|-------------------------|-------------------|--------------|
| ASR Provider | Aliyun + DDRJ VAD | XFyun (iFlytek) via MRCP | FunASR + Aliyun | Aliyun + FunASR (multi-provider) |
| TTS Provider | — (ASR only) | XFyun via MRCP | Aliyun + Doubao | Aliyun + sherpa-onnx (multi-provider) |
| Protocol | Direct API | MRCP | Direct API | Direct API (better latency) |
| Barge-In | Yes (via VAD) | Limited (MRCP limitation) | Yes | Yes |
| Noise VAD | Yes (DDRJ commercial) | No | No | Future enhancement |
| Dynamic Config | Limited | No | Yes | Planned |
| Health Monitoring | No | No | Partial | Planned |
| ESL Integration | Yes | No | Yes | Yes |

## Sources

- **cdevelop/FreeSWITCH-ASR**: https://github.com/cdevelop/FreeSWITCH-ASR - Reference implementation for Aliyun ASR integration with VAD
- **wangkaisine/mrcp-plugin-with-freeswitch**: https://github.com/wangkaisine/mrcp-plugin-with-freeswitch - MRCP approach reference (project chose not to use MRCP)
- **easycallcenter365**: https://github.com/easycallcenter365/easycallcenter365 - Full call center solution with LLM integration
- **FunASR**: https://github.com/modelscope/FunASR - Local ASR engine with streaming support, VAD, punctuation
- **sherpa-onnx**: https://github.com/k2-fsa/sherpa-onnx - Local TTS engine with ONNX runtime, multi-platform
- **FreeSWITCH Modular Design**: https://zread.ai/signalwire/freeswitch/12-modular-design-pattern - Module architecture patterns

---
*Feature research for: FreeSWITCH ASR/TTS Module*
*Researched: 2026-03-26*