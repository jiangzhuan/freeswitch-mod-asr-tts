# Stack Research

**Domain:** FreeSWITCH ASR/TTS Module Development
**Researched:** 2026-03-26
**Confidence:** HIGH

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

```bash
# Debian 12 - System Dependencies
apt-get install -y build-essential cmake libtool autoconf automake
apt-get install -y libopus-dev libcurl4-openssl-dev libssl-dev
apt-get install -y libpthread-stubs0-dev

# FreeSWITCH Development Headers
# Option 1: From source
git clone -b v1.10.12 https://github.com/signalwire/freeswitch.git
cd freeswitch
./configure --prefix=/usr/local/freeswitch
make && make install

# Option 2: Use fsget (recommended)
# See: https://github.com/signalwire/freeswitch/tree/master/scripts/packaging

# FunASR Runtime (for local ASR)
pip3 install funasr funasr-onnx
# Or use Docker image: registry.cn-hangzhou.aliyuncs.com/funasr_repo/funasr:funasr-runtime-cpu-1.12

# sherpa-onnx (for local TTS)
git clone https://github.com/k2-fsa/sherpa-onnx.git
cd sherpa-onnx
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make && make install
```

## Module Compilation

```bash
# Compile ASR module (example)
g++ -shared -fPIC -o mod_asr.so mod_asr.cpp \
    -I /usr/local/freeswitch/include/freeswitch \
    -L /usr/local/freeswitch/lib \
    -lfreeswitch \
    -L /usr/local/lib \
    -lonnxruntime \
    -lcurl -lcjson

# Install module
cp mod_asr.so /usr/local/freeswitch/mod/
# Add to modules.conf.xml: <load module="mod_asr"/>
```

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

**If cloud-first deployment:**
- Use Alibaba Cloud NLS SDK for ASR (WebSocket streaming)
- Use Alibaba Cloud TTS SDK for TTS (WebSocket streaming)
- Implement fallback to local services on timeout
- Because: Cloud offers best accuracy, but network latency and availability are risks

**If offline-only deployment:**
- Use FunASR with paraformer-zh-streaming model
- Use sherpa-onnx with Matcha-TTS or VITS model
- No network dependency required
- Because: Air-gapped environments, privacy requirements, or cost control

**If hybrid deployment (recommended):**
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
**Decision:** Use Alibaba Cloud NLS SDK directly, not MRCP protocol
**Rationale:**
- Lower latency (no protocol overhead)
- Project explicitly excludes MRCP
- Reference implementation (cdevelop/FreeSWITCH-ASR) uses direct SDK
- WebSocket streaming for real-time results

### 2. C++ over C
**Decision:** Write modules in C++ (compiled as .so)
**Rationale:**
- Most SDKs (FunASR, sherpa-onnx, Alibaba Cloud) have C++ bindings
- C++17 features improve code safety (RAII, smart pointers)
- FreeSWITCH codebase is 61.2% C, 20.3% C++ - both supported
- mod_skel template can be adapted for C++

### 3. ONNX Runtime for Local Inference
**Decision:** Use ONNX Runtime as ML inference backend
**Rationale:**
- Both FunASR and sherpa-onnx support ONNX
- CPU-optimized inference
- Quantized models reduce memory footprint
- Single dependency for both ASR and TTS

### 4. ESL for Java Integration
**Decision:** Use mod_event_socket for Java business system communication
**Rationale:**
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

---
*Stack research for: FreeSWITCH ASR/TTS Module Development*
*Researched: 2026-03-26*