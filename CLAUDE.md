<!-- GSD:project-start source:PROJECT.md -->
## Project

**FreeSWITCH ASR/TTS Modules**

基于 FreeSWITCH 的语音识别(ASR)和语音合成(TTS)模块扩展，用于智能客服场景。支持云端服务（阿里云 ASR/TTS）和本地服务（FunASR、sherpa-onnx TTS），提供流式语音识别能力，通过 ESL 与 Java 业务系统集成。

**Core Value:** 提供高性能、可动态切换、支持自动降级的实时语音识别和语音合成能力，确保智能客服系统的语音交互稳定可靠。

### Constraints

- **Tech Stack**: C 语言开发 FreeSWITCH 模块 — 性能最优，与 FreeSWITCH 核心一致
- **Platform**: Linux Debian 12 — 生产环境要求
- **FreeSWITCH Version**: 最新稳定版 — 保持与上游同步
- **Integration**: ESL 与 Java 业务系统通信 — 已有业务系统架构
- **ASR Mode**: 流式识别（边说边识别）— 智能客服实时交互需求
<!-- GSD:project-end -->

<!-- GSD:stack-start source:research/STACK.md -->
## Technology Stack

## Recommended Stack
### Core Technologies
| Technology | Version | Purpose | Why Recommended |
|------------|---------|---------|-----------------|
| FreeSWITCH | 1.10.12 | Telephony platform | Latest stable release (Aug 2024), mature codebase, well-documented module API. 61.2% C, 20.3% C++ codebase matches project constraints. |
| GCC/G++ | 12+ | C/C++ compiler | Standard toolchain for Debian 12, required for FreeSWITCH module compilation. C++ recommended for ASR/TTS modules due to better SDK support. |
| ONNX Runtime | 1.17+ | ML inference engine | sherpa-onnx dependency for local TTS. Cross-platform, optimized for CPU inference, supports quantized models. |
| libopus | 1.4 | Audio codec | Required for high-quality audio streaming. Standard in FreeSWITCH, supports 8kHz telephony and higher rates. |
### ASR (Speech Recognition)
| Technology | Version | Purpose | Why Recommended |
|------------|---------|---------|-----------------|
| Alibaba Cloud NLS SDK | Latest | Cloud ASR | Direct API integration for streaming recognition. Reference implementation exists (cdevelop/FreeSWITCH-ASR). Better performance than MRCP protocol. |
| FunASR Runtime | 1.0+ | Local ASR | Alibaba's open-source ASR toolkit. Supports streaming with paraformer-zh-streaming model. 15.4k GitHub stars, active development. Has C++ runtime for ONNX inference. |
| Paraformer-zh-streaming | Latest | ASR model | Streaming Chinese ASR model from FunASR. 60k hours training data, 220M parameters. Real-time latency ~600ms. |
### TTS (Speech Synthesis)
| Technology | Version | Purpose | Why Recommended |
|------------|---------|---------|-----------------|
| Alibaba Cloud TTS SDK | Latest | Cloud TTS | Streaming synthesis with low latency. Supports multiple voices. Direct WebSocket API for real-time output. |
| sherpa-onnx | Latest | Local TTS | Offline TTS using ONNX runtime. Supports Matcha, Piper, VITS engines. 11.1k GitHub stars. C/C++ API for direct FreeSWITCH integration. No network dependency. |
### Integration Layer
| Technology | Version | Purpose | Why Recommended |
|------------|---------|---------|-----------------|
| mod_event_socket | Built-in | ESL communication | Standard FreeSWITCH module for event-based communication with Java systems. Inbound/outbound modes supported. JSON/plain event formats. |
| libcurl | 8.0+ | HTTP client | For Alibaba Cloud API calls. Required for streaming WebSocket connections. Already a FreeSWITCH dependency. |
| cJSON | 1.7+ | JSON parsing | Lightweight C JSON library. Required for parsing Alibaba Cloud and FunASR responses. Included in FreeSWITCH libs. |
### Supporting Libraries
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| APR (Apache Portable Runtime) | 1.7+ | Platform abstraction | FreeSWITCH core dependency. Use for thread/memory management in modules. |
| switch_buffer | Built-in | Audio buffering | FreeSWITCH API for audio frame management. Essential for streaming audio processing. |
| pthreads | Native | Threading | Required for concurrent ASR/TTS processing. Use with FreeSWITCH thread pool APIs. |
### Development Tools
| Tool | Purpose | Notes |
|------|---------|-------|
| fs_cli | FreeSWITCH CLI | Debug and test module loading. Use `load mod_xxx` and `reload mod_xxx` commands. |
| GDB | Debugging | Essential for C/C++ module debugging. Attach to freeswitch process. |
| Valgrind | Memory analysis | Detect memory leaks in module code. Critical for long-running telephony processes. |
## Installation
# Debian 12 - System Dependencies
# FreeSWITCH Development Headers
# Option 1: From source
# Option 2: Use fsget (recommended)
# See: https://github.com/signalwire/freeswitch/tree/master/scripts/packaging
# FunASR Runtime (for local ASR)
# Or use Docker image: registry.cn-hangzhou.aliyuncs.com/funasr_repo/funasr:funasr-runtime-cpu-1.12
# sherpa-onnx (for local TTS)
## Module Compilation
# Compile ASR module (example)
# Install module
# Add to modules.conf.xml: <load module="mod_asr"/>
## Alternatives Considered
| Category | Recommended | Alternative | Why Not |
|----------|-------------|-------------|---------|
| Cloud ASR | Alibaba Cloud NLS SDK | MRCP Protocol (UniMRCP) | Project explicitly excludes MRCP for performance. Direct API has lower latency. |
| Local ASR | FunASR | Whisper (OpenAI) | FunASR optimized for Chinese, streaming-first design. Whisper is non-streaming, higher latency. |
| Local TTS | sherpa-onnx | Coqui TTS | sherpa-onnx has C API (critical for FreeSWITCH), Coqui is Python-only. |
| Cloud ASR | Alibaba Cloud | iFlytek (讯飞) | User already has Alibaba Cloud credentials. API similarity. |
| Audio Format | Opus/PCMU | G.729 | G.729 requires commercial license. Opus is free and higher quality. |
## What NOT to Use
| Avoid | Why | Use Instead |
|-------|-----|-------------|
| MRCP Protocol | Project explicitly excludes MRCP. Higher latency due to protocol overhead. | Direct WebSocket/HTTP API calls |
| Python modules (mod_python) | Not C language. Performance overhead. | C/C++ modules |
| Whisper for streaming | Non-streaming model. High latency (2-5 seconds). | FunASR paraformer-zh-streaming |
| Coqui TTS | Python-only, no C API for FreeSWITCH integration. | sherpa-onnx with C API |
| Non-Debian platforms | Project constraint - Debian 12 only. | Debian 12 |
| Script language modules | Project constraint - C language only. | C/C++ native modules |
## Stack Patterns by Variant
- Use Alibaba Cloud NLS SDK for ASR (WebSocket streaming)
- Use Alibaba Cloud TTS SDK for TTS (WebSocket streaming)
- Implement fallback to local services on timeout
- Because: Cloud offers best accuracy, but network latency and availability are risks
- Use FunASR with paraformer-zh-streaming model
- Use sherpa-onnx with Matcha-TTS or VITS model
- No network dependency required
- Because: Air-gapped environments, privacy requirements, or cost control
- Primary: Cloud services (Alibaba Cloud ASR/TTS)
- Fallback: Local services (FunASR/sherpa-onnx)
- Health check monitoring for automatic failover
- Because: Best accuracy with reliability guarantee
## Version Compatibility
| Package A | Compatible With | Notes |
|-----------|-----------------|-------|
| FreeSWITCH 1.10.12 | Debian 12 (Bookworm) | Primary target platform |
| FreeSWITCH 1.10.12 | GCC 12+ | Required C++17 support |
| FunASR Runtime 1.12 | ONNX Runtime 1.17+ | Model inference backend |
| sherpa-onnx latest | ONNX Runtime 1.17+ | Shared dependency with FunASR |
| libopus 1.4 | FreeSWITCH 1.10.x | Bundled version works |
## Key Architecture Decisions
### 1. Direct API over MRCP
- Lower latency (no protocol overhead)
- Project explicitly excludes MRCP
- Reference implementation (cdevelop/FreeSWITCH-ASR) uses direct SDK
- WebSocket streaming for real-time results
### 2. C++ over C
- Most SDKs (FunASR, sherpa-onnx, Alibaba Cloud) have C++ bindings
- C++17 features improve code safety (RAII, smart pointers)
- FreeSWITCH codebase is 61.2% C, 20.3% C++ - both supported
- mod_skel template can be adapted for C++
### 3. ONNX Runtime for Local Inference
- Both FunASR and sherpa-onnx support ONNX
- CPU-optimized inference
- Quantized models reduce memory footprint
- Single dependency for both ASR and TTS
### 4. ESL for Java Integration
- Standard FreeSWITCH integration pattern
- Reference project (easycallcenter365) uses this approach
- JSON event format for easy parsing
- Inbound mode for control commands
## Sources
- **FreeSWITCH Official** — https://github.com/signalwire/freeswitch — Version 1.10.12 verified (Aug 2024)
- **FunASR GitHub** — https://github.com/modelscope/FunASR — 15.4k stars, streaming ASR support verified
- **sherpa-onnx GitHub** — https://github.com/k2-fsa/sherpa-onnx — 11.1k stars, TTS + C API verified
- **FreeSWITCH-ASR Reference** — https://github.com/cdevelop/FreeSWITCH-ASR — Alibaba Cloud integration pattern
- **easycallcenter365** — https://github.com/easycallcenter365/easycallcenter365 — Production reference for mod_funasr, mod_aliyun_tts
- **FreeSWITCH Module Docs** — https://developer.signalwire.com/freeswitch/FreeSWITCH-Explained/Modules/ — mod_skel template, mod_event_socket
- **FreeSWITCH Modules List** — Verified mod_event_socket, mod_esl for ESL integration
- **MRCP Plugin Reference** — https://github.com/wangkaisine/mrcp-plugin-with-freeswitch — Shows alternative (MRCP) approach, confirmed project excludes this
<!-- GSD:stack-end -->

<!-- GSD:conventions-start source:CONVENTIONS.md -->
## Conventions

Conventions not yet established. Will populate as patterns emerge during development.
<!-- GSD:conventions-end -->

<!-- GSD:architecture-start source:ARCHITECTURE.md -->
## Architecture

Architecture not yet mapped. Follow existing patterns found in the codebase.
<!-- GSD:architecture-end -->

<!-- GSD:workflow-start source:GSD defaults -->
## GSD Workflow Enforcement

Before using Edit, Write, or other file-changing tools, start work through a GSD command so planning artifacts and execution context stay in sync.

Use these entry points:
- `/gsd:quick` for small fixes, doc updates, and ad-hoc tasks
- `/gsd:debug` for investigation and bug fixing
- `/gsd:execute-phase` for planned phase work

Do not make direct repo edits outside a GSD workflow unless the user explicitly asks to bypass it.
<!-- GSD:workflow-end -->



<!-- GSD:profile-start -->
## Developer Profile

> Profile not yet configured. Run `/gsd:profile-user` to generate your developer profile.
> This section is managed by `generate-claude-profile` -- do not edit manually.
<!-- GSD:profile-end -->
