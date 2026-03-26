# Requirements: FreeSWITCH ASR/TTS Modules

**Defined:** 2026-03-26
**Core Value:** 提供高性能、可动态切换、支持自动降级的实时语音识别和语音合成能力，确保智能客服系统的语音交互稳定可靠。

## v1 Requirements

初始版本需求。每项需求映射到路线图阶段。

### ASR Core

- [ ] **ASR-01**: System can perform streaming speech recognition with Aliyun Cloud ASR
  <!-- 系统可通过阿里云 ASR 进行流式语音识别 -->
- [ ] **ASR-02**: System can detect voice activity (VAD) to distinguish speech from silence
  <!-- 系统可检测语音活动(VAD)以区分语音和静音 -->
- [ ] **ASR-03**: System can publish ASR results via ESL custom events for Java business system integration
  <!-- 系统可通过 ESL 自定义事件发布 ASR 识别结果，供 Java 业务系统集成 -->
- [ ] **ASR-04**: System can handle 8kHz G.711 (PCMU/PCMA) telephony audio with proper resampling to 16kHz for ASR
  <!-- 系统可处理 8kHz G.711 电话音频，并正确重采样至 16kHz 用于 ASR -->
- [ ] **ASR-05**: System can be configured via XML configuration file in FreeSWITCH autoload_configs directory
  <!-- 系统可通过 FreeSWITCH autoload_configs 目录中的 XML 配置文件进行配置 -->
- [ ] **ASR-06**: System can be loaded/unloaded via standard FreeSWITCH module API
  <!-- 系统可通过标准 FreeSWITCH 模块 API 加载/卸载 -->

### TTS

- [ ] **TTS-01**: System can perform speech synthesis with Aliyun Cloud TTS
  <!-- 系统可通过阿里云 TTS 进行语音合成 -->
- [ ] **TTS-02**: System can stream TTS audio to reduce first-packet latency
  <!-- 系统可流式传输 TTS 音频以降低首包延迟 -->

### Advanced

- [ ] **ADV-01**: User can interrupt TTS playback by speaking (Barge-In support)
  <!-- 用户可通过说话打断 TTS 播放（Barge-In 支持） -->

## v2 Requirements

推迟到后续版本。已跟踪但不包含在当前路线图中。

### Local Providers

- **ASR-07**: System can perform streaming speech recognition with local FunASR engine
  <!-- 系统可通过本地 FunASR 引擎进行流式语音识别 -->
- **TTS-03**: System can perform speech synthesis with local sherpa-onnx engine
  <!-- 系统可通过本地 sherpa-onnx 引擎进行语音合成 -->

### Reliability

- **ADV-02**: System can automatically switch between cloud and local ASR providers on failure
  <!-- 系统可在云端 ASR 故障时自动切换到本地 ASR 服务商 -->
- **ADV-03**: System can automatically switch between cloud and local TTS providers on failure
  <!-- 系统可在云端 TTS 故障时自动切换到本地 TTS 服务商 -->
- **ADV-04**: System can monitor provider health status with periodic checks
  <!-- 系统可通过定期检查监控服务商健康状态 -->
- **ADV-05**: System can dynamically modify configuration via API/ESL without module reload
  <!-- 系统可通过 API/ESL 动态修改配置，无需重新加载模块 -->

## Out of Scope

明确排除的范围。记录以防止范围蔓延。

| Feature | Reason |
|---------|--------|
| MRCP Protocol Support | Direct API calls chosen for lower latency / 选择直接 API 调用以获得更低延迟 |
| Non-Linux Platform Support | Production target is Debian 12 only / 生产目标仅限 Debian 12 |
| Script Language Modules | C modules only for performance / 仅限 C 模块以确保性能 |
| Real-Time Web Dashboard | Belongs in Java business system / 属于 Java 业务系统范畴 |
| Built-in LLM Integration | Belongs in business logic layer / 属于业务逻辑层范畴 |
| Noise-Aware VAD | Commercial grade, defer to v2+ / 商业级功能，推迟到 v2+ |
| Recording with ASR | QA feature, not core functionality / QA 功能，非核心功能 |
| Hotwords/Keywords Boosting | Domain-specific enhancement, defer to v2+ / 领域特定增强，推迟到 v2+ |

## Traceability

需求与阶段映射关系。在路线图创建期间更新。

| Requirement | Phase | Status |
|-------------|-------|--------|
| ASR-01 | Phase 1: ASR Core Module | Pending |
| ASR-02 | Phase 1: ASR Core Module | Pending |
| ASR-03 | Phase 1: ASR Core Module | Pending |
| ASR-04 | Phase 1: ASR Core Module | Pending |
| ASR-05 | Phase 1: ASR Core Module | Pending |
| ASR-06 | Phase 1: ASR Core Module | Pending |
| TTS-01 | Phase 2: TTS Module | Pending |
| TTS-02 | Phase 2: TTS Module | Pending |
| ADV-01 | Phase 3: Barge-In | Pending |

**Coverage:**
- v1 requirements: 9 total
- Mapped to phases: 9
- Unmapped: 0 ✓

**Phase Distribution:**
- Phase 1 (ASR Core Module): 6 requirements
- Phase 2 (TTS Module): 2 requirements
- Phase 3 (Barge-In): 1 requirement

---
*Requirements defined: 2026-03-26*
*Last updated: 2026-03-26 after user requirements collection*