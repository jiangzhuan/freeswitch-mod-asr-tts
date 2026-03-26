# FreeSWITCH ASR/TTS Modules

## What This Is

基于 FreeSWITCH 的语音识别(ASR)和语音合成(TTS)模块扩展，用于智能客服场景。支持云端服务（阿里云 ASR/TTS）和本地服务（FunASR、sherpa-onnx TTS），提供流式语音识别能力，通过 ESL 与 Java 业务系统集成。

## Core Value

提供高性能、可动态切换、支持自动降级的实时语音识别和语音合成能力，确保智能客服系统的语音交互稳定可靠。

## Requirements

### Validated

(None yet — ship to validate)

### Active

- [ ] ASR 模块开发 - 阿里云 ASR 流式识别
- [ ] ASR 模块开发 - 本地 FunASR 流式识别
- [ ] TTS 模块开发 - 阿里云 TTS
- [ ] TTS 模块开发 - 本地 sherpa-onnx TTS
- [ ] 多服务商动态切换机制
- [ ] 云端服务不可用时自动降级到本地服务
- [ ] 通过 ESL 将识别结果传递给 Java 业务系统
- [ ] 支持 API/ESL 动态修改配置
- [ ] 模块配置文件支持（XML 格式）

### Out of Scope

- MRCP 协议支持 — 采用直接 API 调用方式，性能更优
- 非 Linux 平台支持 — 仅支持 Debian 12
- 非 C 语言实现 — 不考虑脚本语言模块

## Context

### 项目背景
- **场景**: 智能客服语音交互
- **目标**: FreeSWITCH ASR/TTS 模块扩展开发

### 服务商支持

| 类型 | 云端服务 | 本地服务 |
|------|----------|----------|
| ASR | 阿里云 ASR | FunASR |
| TTS | 阿里云 TTS | sherpa-onnx TTS |

### 技术决策

| 决策 | 选择 | 理由 |
|------|------|------|
| FreeSWITCH 版本 | 最新稳定版 | 保持与上游同步 |
| 开发语言 | C 模块 | 性能最优 |
| 部署平台 | Linux Debian 12 | 生产环境要求 |
| 对接方式 | 直接 API 调用 | 性能最优，避免 MRCP 协议开销 |
| 识别模式 | 流式识别 | 边说边识别，实时交互 |

### 集成需求
- **业务系统集成**: 通过 ESL 将识别结果传递给 Java 业务系统
- **配置管理**: 支持 API/ESL 动态修改配置
- **降级策略**: 云端不可用时自动降级到本地服务

### 开发优先级
1. ASR 模块优先于 TTS 模块
2. 阿里云 ASR 最先实现（用户已有 key 和 secret 可测试）

### 技术背景
- FreeSWITCH 是开源的电话交换平台，支持模块化扩展
- 智能客服场景需要实时语音交互能力
- 云端服务（阿里云）提供高质量 ASR/TTS，但存在网络延迟和服务可用性风险
- 本地服务（FunASR、sherpa-onnx）提供离线能力，可作为降级方案

### 参考项目
- FreeSWITCH 官方: https://github.com/signalwire/freeswitch
- FreeSWITCH 模块文档: https://developer.signalwire.com/freeswitch/FreeSWITCH-Explained/Modules/
- 模块化设计模式: https://zread.ai/signalwire/freeswitch/12-modular-design-pattern
- FreeSWITCH-ASR 参考实现: https://github.com/cdevelop/FreeSWITCH-ASR
- MRCP 插件参考: https://github.com/wangkaisine/mrcp-plugin-with-freeswitch
- Java 业务系统集成参考: https://github.com/easycallcenter365/easycallcenter365

### 已知挑战
- 流式语音识别需要处理音频分块和实时返回
- 多服务商切换需要统一抽象层
- 降级策略需要监控服务健康状态

## Constraints

- **Tech Stack**: C 语言开发 FreeSWITCH 模块 — 性能最优，与 FreeSWITCH 核心一致
- **Platform**: Linux Debian 12 — 生产环境要求
- **FreeSWITCH Version**: 最新稳定版 — 保持与上游同步
- **Integration**: ESL 与 Java 业务系统通信 — 已有业务系统架构
- **ASR Mode**: 流式识别（边说边识别）— 智能客服实时交互需求

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| 直接 API 调用而非 MRCP 协议 | 性能更优，减少协议开销 | ✅ Confirmed |
| ASR 优先于 TTS 开发 | ASR 是智能客服核心能力，TTS 可后续补充 | ✅ Confirmed |
| 阿里云 ASR 最先实现 | 用户已有 key 和 secret 可测试 | ✅ Confirmed |
| C 模块开发 | 性能最优，与 FreeSWITCH 核心技术栈一致 | ✅ Confirmed |
| 流式识别模式 | 智能客服实时交互需求，边说边识别 | ✅ Confirmed |

---

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `/gsd-transition`):
1. Requirements invalidated? → Move to Out of Scope with reason
2. Requirements validated? → Move to Validated with phase reference
3. New requirements emerged? → Add to Active
4. Decisions to log? → Add to Key Decisions
5. "What This Is" still accurate? → Update if drifted

**After each milestone** (via `/gsd-complete-milestone`):
1. Full review of all sections
2. Core Value check — still the right priority?
3. Audit Out of Scope — reasons still valid?
4. Update Context with current state

---
*Last updated: 2026-03-26 after user requirements collection*