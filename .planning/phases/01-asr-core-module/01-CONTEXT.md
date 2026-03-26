# Phase 1: ASR Core Module - Context

**Gathered:** 2026-03-26
**Status:** Ready for planning

<domain>
## Phase Boundary

实现阿里云 ASR 流式语音识别核心模块，通过 ESL 与 Java 业务系统集成。本阶段建立完整的音频处理管道、WebSocket 连接管理、VAD 集成和事件发布机制。

**In Scope:**
- 阿里云 ASR WebSocket 流式识别
- 音频捕获和处理（8kHz G.711 → 16kHz PCM）
- VAD 语音活动检测
- ESL 自定义事件发布
- XML 配置文件
- FreeSWITCH 模块生命周期

**Out of Scope:**
- TTS 功能（Phase 2）
- 多服务商切换（v2）
- 本地 FunASR 集成（v2）
- Barge-In（Phase 3）

</domain>

<decisions>
## Implementation Decisions

### 音频处理架构

- **D-01:** 使用 **Media Bug 机制**捕获音频，避免阻塞媒体路径
  - 理由：Media bug 是 FreeSWITCH 标准的非阻塞音频拦截方式，不影响正常通话
  - 参考：ARCHITECTURE.md - Media Bug Integration Pattern

- **D-02:** 使用 **Ring Buffer** 存储音频块，worker 线程异步消费
  - 理由：解耦音频捕获和 ASR 处理，避免在媒体线程中阻塞
  - 参考：PITFALLS.md - Blocking ASR API Calls in Audio Thread

- **D-03:** 使用 **线程池**处理 ASR API 调用（初始大小：4 线程）
  - 理由：支持多路并发通话，避免单线程瓶颈
  - 参考：STACK.md - Scaling Considerations

- **D-04:** 音频重采样使用 FreeSWITCH 内置的 `switch_resample_create/process`
  - 理由：标准 API，稳定可靠
  - 参考：PITFALLS.md - Audio Codec Mismatch

### WebSocket 连接管理

- **D-05:** 实现 **连接状态机**：DISCONNECTED → CONNECTING → CONNECTED → RECONNECTING
  - 理由：清晰的连接生命周期管理，便于调试和维护
  - 参考：PITFALLS.md - WebSocket Connection State Mishandling

- **D-06:** 使用 **指数退避重连**：1s, 2s, 4s, 8s, ... 最大 30s
  - 理由：避免频繁重连导致服务器压力，逐步恢复
  - 参考：PITFALLS.md - WebSocket Connection State Mishandling

- **D-07:** 心跳间隔 **30 秒**，连接超时 **5 秒**，空闲超时 **60 秒**
  - 理由：保持连接活跃，及时发现断连
  - 参考：STACK.md - Version Compatibility

- **D-08:** 重连期间音频数据缓存到 ring buffer，不丢弃
  - 理由：保证识别完整性，避免丢失语音片段

### VAD 实现

- **D-09:** Phase 1 使用 **能量检测 VAD**（Energy-based VAD）
  - 理由：实现简单、延迟低、无外部依赖；后续可升级为 FunASR fsmn-vad
  - 参考：FEATURES.md - Feature Dependencies

- **D-10:** VAD 阈值可配置（默认 -40dB），支持运行时调整
  - 理由：不同环境噪声水平不同，需要灵活配置

- **D-11:** VAD 检测到语音结束后发送 `is_final=True` 到 ASR
  - 理由：触发阿里云 ASR 返回最终识别结果
  - 参考：PITFALLS.md - Integration Gotchas

### ESL 事件格式

- **D-12:** 事件类型：`SWITCH_EVENT_CUSTOM`，子类：`"asr"`
  - 理由：FreeSWITCH 标准的自定义事件方式
  - 参考：PITFALLS.md - Incorrect ESL Event Handling

- **D-13:** 事件 Headers 必须包含：
  - `Channel` - 通话 UUID
  - `ASR-Response` - JSON 格式识别结果
  - `ASR-Is-Final` - "true" 或 "false"
  - `Event-Sequence` - 事件序号
  - 理由：Java 业务系统需要完整上下文信息

- **D-14:** JSON payload 结构：
  ```json
  {
    "text": "识别文本",
    "confidence": 0.95,
    "is_final": true,
    "begin_time": 0,
    "end_time": 1500,
    "words": [...]
  }
  ```
  - 理由：结构化数据便于 Java 系统解析和使用

### 配置设计

- **D-15:** 配置文件路径：`autoload_configs/mod_asr_tts.conf.xml`
  - 理由：FreeSWITCH 标准配置目录

- **D-16:** 必须配置项：
  - `appkey` - 阿里云 ASR 应用 key
  - `token` - 阿里云访问 token
  - `websocket-url` - ASR 服务地址
  - 理由：连接阿里云 ASR 必需

- **D-17:** 可选配置项（有默认值）：
  - `sample-rate` - 采样率（默认 16000）
  - `vad-threshold` - VAD 能量阈值（默认 -40）
  - `thread-pool-size` - 工作线程数（默认 4）
  - `reconnect-max-interval` - 最大重连间隔（默认 30 秒）
  - 理由：提供灵活性，同时有合理默认值

### 错误处理

- **D-18:** WebSocket 连接失败：记录日志 + 指数退避重连 + 发送 ESL 错误事件
  - 理由：不中断通话，允许后续恢复

- **D-19:** ASR 识别错误：记录日志 + 发送 ESL 错误事件（包含错误码和描述）
  - 理由：让 Java 业务系统决定如何处理

- **D-20:** 音频处理错误：记录日志 + 继续运行（不终止通话）
  - 理由：保持通话稳定，错误可能是临时的

### the agent's Discretion

- **A-01:** 具体的 ring buffer 大小可以在实现时根据内存约束确定（建议 10 秒音频缓冲）
- **A-02:** 线程池大小的具体值可以在性能测试后调整
- **A-03:** JSON payload 中的 `words` 数组格式可以参考阿里云 ASR 返回格式确定

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### 核心架构参考

- `.planning/research/ARCHITECTURE.md` — FreeSWITCH 模块架构模式、组件设计、数据流
- `.planning/research/STACK.md` — 技术栈选择、版本兼容性、编译配置
- `.planning/research/PITFALLS.md` — 8 个关键陷阱及避免方法

### 需求和路线图

- `.planning/REQUIREMENTS.md` — ASR-01 至 ASR-06 需求定义
- `.planning/ROADMAP.md` — Phase 1 目标和成功标准
- `.planning/PROJECT.md` — 项目背景、技术决策、约束条件

### 外部参考（HIGH confidence）

- **FreeSWITCH 官方文档** — https://developer.signalwire.com/freeswitch/ — Module API, mod_skel 模板
- **FreeSWITCH-ASR 参考实现** — https://github.com/cdevelop/FreeSWITCH-ASR — 阿里云 ASR 集成模式
- **阿里云 NLS SDK** — https://help.aliyun.com/product/30413.html — WebSocket 流式识别协议

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets

项目处于初始阶段，无现有代码。参考实现可供借鉴：

- **cdevelop/FreeSWITCH-ASR**: 阿里云 ASR + VAD 集成参考
  - 媒体 bug 音频捕获模式
  - WebSocket 连接管理
  - ESL 事件发布格式

### Established Patterns

- **Provider Abstraction Pattern**: ASR/TTS 服务商抽象接口（为 Phase 2/3 准备）
- **Media Bug Integration Pattern**: 非阻塞音频拦截
- **Event-Based Result Publishing**: ESL 自定义事件发布

### Integration Points

- **FreeSWITCH Core**: `switch_loadable_module_interface` 注册模块
- **FreeSWITCH Media**: `switch_media_bug` 捕获音频
- **FreeSWITCH Events**: `switch_event_t` 发布 ASR 结果
- **External**: 阿里云 ASR WebSocket API
- **External**: Java 业务系统（ESL 事件订阅）

</code_context>

<specifics>
## Specific Ideas

- 参考 cdevelop/FreeSWITCH-ASR 的媒体 bug 实现模式
- 使用 FreeSWITCH 内置的 `switch_resample_*` API 进行音频重采样
- ESL 事件格式与 easycallcenter365 项目保持兼容
- 配置文件结构参考 FreeSWITCH 标准模块（如 mod_event_socket）

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 01-asr-core-module*
*Context gathered: 2026-03-26*