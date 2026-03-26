# Phase 1: ASR Core Module - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-03-26
**Phase:** 01-asr-core-module
**Areas discussed:** 音频处理架构, WebSocket 连接管理, VAD 实现, ESL 事件格式, 配置设计, 错误处理
**Mode:** Auto-decision based on research documentation

---

## 音频处理架构

| Option | Description | Selected |
|--------|-------------|----------|
| Media Bug 机制 | 非阻塞音频拦截，标准 FreeSWITCH 模式 | ✓ |
| Read Frame 回调 | 直接在 IO 循环中处理，可能阻塞 | |
| 独立录音线程 | 复杂度高，资源消耗大 | |

**Decision:** 使用 Media Bug 机制捕获音频
**Rationale:** Media bug 是 FreeSWITCH 标准的非阻塞音频拦截方式，不影响正常通话。配合 ring buffer 和 worker 线程池实现异步处理。

---

## 音频线程模型

| Option | Description | Selected |
|--------|-------------|----------|
| Ring Buffer + Worker 线程池 | 异步解耦，支持并发 | ✓ |
| 单线程同步处理 | 简单但阻塞媒体路径 | |
| Per-call 专用线程 | 资源消耗大，难以扩展 | |

**Decision:** Ring Buffer + Worker 线程池（初始 4 线程）
**Rationale:** 解耦音频捕获和 ASR 处理，避免在媒体线程中阻塞。支持多路并发通话，避免单线程瓶颈。

---

## WebSocket 连接管理

| Option | Description | Selected |
|--------|-------------|----------|
| 状态机 + 指数退避 | 清晰生命周期，自动恢复 | ✓ |
| 简单重连（固定间隔） | 实现简单，可能导致服务器压力 | |
| 无自动重连 | 需要手动干预 | |

**Decision:** 状态机（DISCONNECTED → CONNECTING → CONNECTED → RECONNECTING）+ 指数退避重连
**Rationale:** 清晰的连接生命周期管理。指数退避（1s, 2s, 4s, 8s, ... 最大 30s）避免频繁重连导致服务器压力。

---

## VAD 实现

| Option | Description | Selected |
|--------|-------------|----------|
| 能量检测 VAD | 简单、低延迟、无依赖 | ✓ |
| FunASR fsmn-vad | 更准确，但有延迟和依赖 | |
| 无 VAD | 浪费资源，不推荐 | |

**Decision:** Phase 1 使用能量检测 VAD
**Rationale:** 实现简单、延迟低、无外部依赖。后续 Phase 可升级为 FunASR fsmn-vad。

---

## ESL 事件格式

| Option | Description | Selected |
|--------|-------------|----------|
| CUSTOM 事件 + asr 子类 | 标准模式，兼容性好 | ✓ |
| 自定义事件类型 | 非标准，可能有问题 | |
| MESSAGE 事件 | 不够语义化 | |

**Decision:** SWITCH_EVENT_CUSTOM，子类 "asr"
**Headers:** Channel, ASR-Response, ASR-Is-Final, Event-Sequence
**Payload:** JSON 格式，包含 text, confidence, is_final, begin_time, end_time, words

---

## 配置文件格式

| Option | Description | Selected |
|--------|-------------|----------|
| XML（FreeSWITCH 标准） | 运营商熟悉，工具支持好 | ✓ |
| JSON | 非标准，需额外解析 | |
| 配置硬编码 | 无法灵活调整 | |

**Decision:** XML 格式，路径 autoload_configs/mod_asr_tts.conf.xml
**Required:** appkey, token, websocket-url
**Optional:** sample-rate, vad-threshold, thread-pool-size, reconnect-max-interval

---

## 错误处理策略

| Option | Description | Selected |
|--------|-------------|----------|
| 记录日志 + ESL 事件 + 继续运行 | 不中断通话，允许恢复 | ✓ |
| 记录日志 + 终止通话 | 用户体验差 | |
| 静默忽略 | 无法排查问题 | |

**Decision:** 记录日志 + 发送 ESL 错误事件 + 继续运行（不终止通话）
**Rationale:** 保持通话稳定，错误可能是临时的。让 Java 业务系统决定如何处理错误。

---

## the agent's Discretion

以下细节在实现时由执行者决定：
- Ring buffer 大小（建议 10 秒音频缓冲）
- 线程池大小的具体值（可在性能测试后调整）
- JSON payload 中 words 数组的详细格式（参考阿里云 ASR 返回格式）

---

## Deferred Ideas

无 — 讨论始终保持在 Phase 1 范围内

---

*Discussion log for: Phase 01-asr-core-module*
*Date: 2026-03-26*