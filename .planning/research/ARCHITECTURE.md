# Architecture Research

**Domain:** FreeSWITCH ASR/TTS Module Development
**Researched:** 2026-03-26
**Confidence:** HIGH

## Standard Architecture

### System Overview

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          FreeSWITCH Core                                 │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │                    switch_loadable_module_interface              │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐   │   │
│  │  │ ASR Interface│  │ TTS Interface│  │ Application Interface│   │   │
│  │  └──────┬───────┘  └──────┬───────┘  └──────────┬───────────┘   │   │
│  └─────────┼─────────────────┼─────────────────────┼───────────────┘   │
│            │                 │                     │                    │
├────────────┼─────────────────┼─────────────────────┼────────────────────┤
│            ▼                 ▼                     ▼                    │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │                    mod_asr_tts (Our Module)                      │   │
│  │  ┌──────────────────────────────────────────────────────────┐   │   │
│  │  │              Provider Abstraction Layer                    │   │   │
│  │  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐   │   │   │
│  │  │  │ Cloud ASR   │  │ Local ASR   │  │ Provider Manager │   │   │   │
│  │  │  │ (Alibaba)   │  │ (FunASR)    │  │ (Switching)      │   │   │   │
│  │  │  └──────┬──────┘  └──────┬──────┘  └────────┬────────┘   │   │   │
│  │  └─────────┼────────────────┼──────────────────┼────────────┘   │   │
│  │            │                │                  │                 │   │
│  │  ┌─────────┼────────────────┼──────────────────┼────────────┐   │   │
│  │  │         ▼                ▼                  ▼            │   │   │
│  │  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │   │   │
│  │  │  │ Cloud TTS   │  │ Local TTS   │  │ Health Monitor  │  │   │   │
│  │  │  │ (Alibaba)   │  │ (sherpa)    │  │ (Failover)      │  │   │   │
│  │  │  └─────────────┘  └─────────────┘  └─────────────────┘  │   │   │
│  │  │              TTS Provider Implementations                 │   │   │
│  │  └──────────────────────────────────────────────────────────┘   │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                                                                         │
├─────────────────────────────────────────────────────────────────────────┤
│                        External Services                                 │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐                   │
│  │ Alibaba ASR  │  │ FunASR API   │  │ Java System  │                   │
│  │ (WebSocket)  │  │ (HTTP/gRPC)  │  │ (ESL)        │                   │
│  └──────────────┘  └──────────────┘  └──────────────┘                   │
└─────────────────────────────────────────────────────────────────────────┘
```

### Component Responsibilities

| Component | Responsibility | Implementation |
|-----------|----------------|----------------|
| Module Entry Point | Module lifecycle, interface registration | `SWITCH_MODULE_LOAD_FUNCTION`, `SWITCH_MODULE_SHUTDOWN_FUNCTION` |
| ASR Interface | Speech recognition abstraction | `switch_asr_interface_t` callbacks |
| TTS Interface | Speech synthesis abstraction | `switch_speech_interface_t` callbacks |
| Application Interface | Dialplan applications (start_asr, play_and_asr) | `switch_application_interface_t` |
| Provider Manager | Multi-provider switching, failover logic | Custom provider registry |
| Cloud ASR Provider | Alibaba Cloud ASR WebSocket client | WebSocket client implementation |
| Local ASR Provider | FunASR streaming recognition | HTTP/gRPC client |
| Health Monitor | Service health checking, auto-failover | Background thread with health checks |
| Event Publisher | ESL event generation for Java integration | `switch_event_t` custom events |

## Recommended Project Structure

```
src/
├── mod_asr_tts.c              # Module entry point, interface registration
├── mod_asr_tts.h              # Public definitions, interface structs
│
├── asr/                       # ASR subsystem
│   ├── asr_interface.c        # ASR interface implementation
│   ├── asr_interface.h        # ASR interface declarations
│   ├── asr_provider.h         # Provider abstraction interface
│   ├── providers/
│   │   ├── aliyun_asr.c       # Alibaba Cloud ASR provider
│   │   ├── aliyun_asr.h
│   │   ├── funasr_asr.c       # FunASR local provider
│   │   └── funasr_asr.h
│   └── asr_manager.c          # Provider selection, failover
│
├── tts/                       # TTS subsystem
│   ├── tts_interface.c        # TTS interface implementation
│   ├── tts_interface.h        # TTS interface declarations
│   ├── tts_provider.h         # Provider abstraction interface
│   ├── providers/
│   │   ├── aliyun_tts.c       # Alibaba Cloud TTS provider
│   │   ├── aliyun_tts.h
│   │   ├── sherpa_tts.c       # sherpa-onnx local provider
│   │   └── sherpa_tts.h
│   └── tts_manager.c          # Provider selection, failover
│
├── core/                      # Core utilities
│   ├── config.c               # XML configuration parsing
│   ├── config.h
│   ├── buffer.c               # Audio buffer management
│   ├── buffer.h
│   ├── event.c                # ESL event publishing
│   └── event.h
│
├── health/                    # Health monitoring
│   ├── health_monitor.c       # Service health checking
│   └── health_monitor.h
│
└── providers/                 # Shared provider utilities
    ├── websocket_client.c     # WebSocket client for cloud services
    └── websocket_client.h
```

### Structure Rationale

- **mod_asr_tts.c/h:** Single entry point for module registration, keeps core FreeSWITCH integration isolated
- **asr/ and tts/ directories:** Clear separation between ASR and TTS subsystems, each with its own provider abstraction
- **providers/ subdirectories:** Each cloud/local service is isolated, making it easy to add new providers
- **core/ directory:** Shared utilities used across ASR/TTS (config, buffers, events)
- **health/ directory:** Independent health monitoring system that can run as background thread

## Architectural Patterns

### Pattern 1: Provider Abstraction Pattern

**What:** Define a common interface for ASR/TTS providers, allowing transparent switching between implementations.

**When to use:** When supporting multiple ASR/TTS services with dynamic switching requirements.

**Trade-offs:**
- Pros: Easy to add new providers, clean separation, testable
- Cons: Additional abstraction layer, may need provider-specific workarounds

**Example:**

```c
/* asr_provider.h - Provider abstraction interface */
typedef struct asr_provider_interface {
    const char *name;
    switch_status_t (*open)(switch_asr_handle_t *ah, const char *codec, int rate, const char *dest);
    switch_status_t (*close)(switch_asr_handle_t *ah, switch_asr_handle_t **new_ah);
    switch_status_t (*feed)(switch_asr_handle_t *ah, void *data, unsigned int len, 
                            switch_asr_flag_t *flags);
    switch_status_t (*results)(switch_asr_handle_t *ah, switch_asr_flag_t *flags);
    switch_status_t (*pause)(switch_asr_handle_t *ah);
    switch_status_t (*resume)(switch_asr_handle_t *ah);
    switch_status_t (*check_health)(void);  /* Health check */
} asr_provider_interface_t;

/* Registration function */
switch_status_t asr_provider_register(asr_provider_interface_t *provider);
```

### Pattern 2: Media Bug Integration Pattern

**What:** Use FreeSWITCH's media bug mechanism to intercept audio frames for ASR processing.

**When to use:** For streaming audio capture during calls without blocking the media path.

**Trade-offs:**
- Pros: Non-blocking, works with any codec, clean integration
- Cons: Requires careful memory management, frame timing considerations

**Example:**

```c
/* Media bug callback for audio interception */
static switch_bool_t asr_media_bug_callback(switch_media_bug_t *bug, void *user_data, 
                                             switch_abc_type_t type)
{
    switch_session_t *session = switch_core_media_bug_get_session(bug);
    asr_session_ctx_t *ctx = (asr_session_ctx_t *)user_data;
    
    switch (type) {
        case SWITCH_ABC_TYPE_INIT:
            /* Initialize ASR session */
            break;
        case SWITCH_ABC_TYPE_READ_REPLACE:
            /* Process incoming audio frame */
            {
                switch_frame_t *frame = switch_core_media_bug_get_read_replace_frame(bug);
                asr_feed_audio(ctx, frame->data, frame->datalen);
            }
            break;
        case SWITCH_ABC_TYPE_CLOSE:
            /* Cleanup ASR session */
            break;
    }
    return SWITCH_TRUE;
}
```

### Pattern 3: Event-Based Result Publishing

**What:** Use FreeSWITCH's event system to publish ASR results to external systems via ESL.

**When to use:** For integration with Java business systems through ESL event subscription.

**Trade-offs:**
- Pros: Decoupled communication, multiple subscribers possible, standard FreeSWITCH pattern
- Cons: Event serialization overhead, async by nature

**Example:**

```c
/* Publish ASR result as custom event */
switch_status_t publish_asr_result(switch_core_session_t *session, const char *result)
{
    switch_event_t *event;
    
    if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, "asr::result") == SWITCH_STATUS_SUCCESS) {
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "ASR-Result", result);
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Channel", 
                                        switch_channel_get_name(switch_core_session_get_channel(session)));
        switch_event_fire(&event);
        return SWITCH_STATUS_SUCCESS;
    }
    return SWITCH_STATUS_FALSE;
}
```

### Pattern 4: Streaming WebSocket Client

**What:** Maintain persistent WebSocket connection for cloud ASR/TTS services with streaming audio.

**When to use:** For cloud ASR services (like Alibaba) that support streaming recognition.

**Trade-offs:**
- Pros: Low latency streaming, efficient for real-time recognition
- Cons: Connection management complexity, reconnection logic needed

## Data Flow

### ASR Streaming Flow

```
Caller Audio
     │
     ▼
┌─────────────┐    ┌─────────────────┐
│  Media Bug  │───▶│  Audio Buffer   │
└─────────────┘    │  (Ring Buffer)  │
                   └────────┬────────┘
                            │
         ┌──────────────────┼──────────────────┐
         ▼                  ▼                  ▼
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│ Cloud Provider  │ │ Local Provider  │ │ Provider Manager│
│ (Alibaba WS)    │ │ (FunASR HTTP)   │ │ (Selection)     │
└────────┬────────┘ └────────┬────────┘ └─────────────────┘
         │                  │
         │    ┌─────────────┘
         ▼    ▼
┌─────────────────┐
│ Recognition     │
│ Results         │
└────────┬────────┘
         │
         ▼
┌─────────────────┐    ┌─────────────────┐
│ Custom Event    │───▶│ ESL Subscribers │
│ (asr::result)   │    │ (Java System)   │
└─────────────────┘    └─────────────────┘
```

### TTS Synthesis Flow

```
Application Request (speak/playback)
     │
     ▼
┌─────────────────┐
│ TTS Interface   │
│ (switch_speech) │
└────────┬────────┘
         │
         ▼
┌─────────────────┐    ┌─────────────────┐
│ Provider Manager│───▶│ Health Check    │
│ (Select Best)   │    │ (Failover)      │
└────────┬────────┘    └─────────────────┘
         │
    ┌────┴────┐
    ▼         ▼
┌────────┐ ┌────────┐
│ Cloud  │ │ Local  │
│ TTS    │ │ TTS    │
└───┬────┘ └───┬────┘
    │          │
    └────┬─────┘
         ▼
┌─────────────────┐
│ Audio Stream    │
│ (PCM/Opus)      │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Media Frame     │
│ (to caller)     │
└─────────────────┘
```

### Failover Flow

```
┌─────────────────────────────────────────────────────────┐
│                    Health Monitor Thread                 │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Periodic Check (every 5s)                       │  │
│  │  ┌─────────────┐  ┌─────────────┐                │  │
│  │  │ Ping Cloud  │  │ Ping Local  │                │  │
│  │  │ ASR/TTS     │  │ ASR/TTS     │                │  │
│  │  └──────┬──────┘  └──────┬──────┘                │  │
│  └─────────┼────────────────┼───────────────────────┘  │
│            │                │                           │
│            ▼                ▼                           │
│  ┌──────────────────────────────────────────────────┐  │
│  │              Provider State Machine               │  │
│  │                                                   │  │
│  │   [CLOUD_ACTIVE] ──(fail)──▶ [LOCAL_ACTIVE]      │  │
│  │         ▲                          │              │  │
│  │         └────────(recover)─────────┘              │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

## Module Interface Registration

### Required FreeSWITCH Interfaces

```c
/* Module load function - registers all interfaces */
SWITCH_MODULE_LOAD_FUNCTION(mod_asr_tts_load)
{
    switch_api_interface_t *api_interface;
    switch_application_interface_t *app_interface;
    
    /* Connect to FreeSWITCH core */
    *module_interface = switch_loadable_module_create_module_interface(pool, modname);
    
    /* Register ASR interface */
    switch_asr_interface_t *asr_interface;
    SWITCH_ADD_ASR(asr_interface, "aliyun", aliyun_asr_open, aliyun_asr_close,
                   aliyun_asr_feed, aliyun_asr_results, aliyun_asr_pause,
                   aliyun_asr_resume, NULL, NULL, NULL);
    
    /* Register TTS interface */
    switch_speech_interface_t *speech_interface;
    SWITCH_ADD_SPEECH(speech_interface, "aliyun_tts", aliyun_tts_open, aliyun_tts_close,
                      aliyun_tts_feed_tts, aliyun_tts_read_tts, NULL, NULL, NULL, NULL);
    
    /* Register dialplan applications */
    SWITCH_ADD_APP(app_interface, "start_asr", "Start ASR recognition", 
                   "Start background ASR recognition", start_asr_function, "", SAF_NONE);
    SWITCH_ADD_APP(app_interface, "play_and_asr", "Play and recognize",
                   "Play audio while performing ASR", play_and_asr_function, "", SAF_NONE);
    
    /* Register API commands */
    SWITCH_ADD_API(api_interface, "asr_status", "Show ASR status", asr_status_function, "");
    
    return SWITCH_STATUS_SUCCESS;
}
```

### Interface Callback Signatures

| Interface | Function | Signature |
|-----------|----------|-----------|
| ASR | open | `switch_status_t (*)(switch_asr_handle_t *ah, const char *codec, int rate, const char *dest)` |
| ASR | close | `switch_status_t (*)(switch_asr_handle_t *ah, switch_asr_handle_t **new_ah)` |
| ASR | feed | `switch_status_t (*)(switch_asr_handle_t *ah, void *data, unsigned int len, switch_asr_flag_t *flags)` |
| ASR | results | `switch_status_t (*)(switch_asr_handle_t *ah, switch_asr_flag_t *flags)` |
| TTS | open | `switch_status_t (*)(switch_speech_handle_t *sh, const char *voice_name, int rate, int channels, switch_speech_flag_t *flags)` |
| TTS | close | `switch_status_t (*)(switch_speech_handle_t *sh, switch_speech_flag_t *flags)` |
| TTS | feed_tts | `switch_status_t (*)(switch_speech_handle_t *sh, char *text, switch_speech_flag_t *flags)` |
| TTS | read_tts | `switch_status_t (*)(switch_speech_handle_t *sh, void *data, switch_size_t *datalen, switch_speech_flag_t *flags)` |

## Scaling Considerations

| Scale | Architecture Adjustments |
|-------|--------------------------|
| 0-100 concurrent calls | Single module instance, single WebSocket connection per provider |
| 100-1000 concurrent calls | Connection pooling for cloud services, multiple local ASR instances |
| 1000+ concurrent calls | Horizontal scaling with multiple FreeSWITCH instances, dedicated ASR/TTS proxy layer |

### Scaling Priorities

1. **First bottleneck:** WebSocket connection limits to cloud ASR - implement connection pooling early
2. **Second bottleneck:** Local ASR CPU usage - consider GPU acceleration or multiple instances
3. **Third bottleneck:** Memory for audio buffers - implement buffer pooling and reuse

## Anti-Patterns

### Anti-Pattern 1: Synchronous Cloud Calls in Audio Path

**What people do:** Making HTTP requests synchronously for each audio chunk.

**Why it's wrong:** Blocks the media path, causes audio stuttering, increases latency dramatically.

**Do this instead:** Use asynchronous WebSocket streaming for cloud services, or local buffering with dedicated worker threads.

### Anti-Pattern 2: No Graceful Degradation

**What people do:** Let the call fail when cloud ASR is unavailable.

**Why it's wrong:** Poor user experience, no resilience to network issues.

**Do this instead:** Implement health checks and automatic failover to local ASR. Cache results when possible.

### Anti-Pattern 3: Tight Coupling to Single Provider

**What people do:** Hardcode provider-specific APIs throughout the code.

**Why it's wrong:** Cannot switch providers without rewriting code, vendor lock-in.

**Do this instead:** Use the Provider Abstraction Pattern with a common interface for all providers.

### Anti-Pattern 4: Ignoring Audio Format Differences

**What people do:** Assume all audio is 8kHz PCMU.

**Why it's wrong:** Caller may use Opus or other codecs, causing recognition failures.

**Do this instead:** Implement transcoding support or negotiate codec with provider requirements.

## Integration Points

### External Services

| Service | Integration Pattern | Notes |
|---------|---------------------|-------|
| Alibaba Cloud ASR | WebSocket streaming | Requires AccessKey/Secret authentication, streaming protocol |
| Alibaba Cloud TTS | HTTP/WebSocket | Text input, PCM audio output |
| FunASR | HTTP/gRPC | Self-hosted option, Chinese language optimization |
| sherpa-onnx TTS | Local API | CPU/GPU inference, ONNX runtime |
| Java Business System | ESL custom events | Subscribe to `asr::result` events |

### Internal Boundaries

| Boundary | Communication | Notes |
|----------|---------------|-------|
| Module Core ↔ ASR Providers | Function calls via interface | Synchronous for feed/results |
| Module Core ↔ TTS Providers | Function calls via interface | Async for feed/read |
| ASR Results ↔ ESL | Event system | Fire-and-forget events |
| Health Monitor ↔ Providers | Shared state | Atomic flags for provider status |

## Build Order Recommendations

Based on architectural dependencies, suggested implementation order:

### Phase 1: Core Infrastructure
1. **Module skeleton** - Entry point, interface registration structure
2. **Configuration system** - XML parsing for provider settings
3. **Audio buffer** - Ring buffer for streaming audio
4. **Event publishing** - ESL custom event framework

### Phase 2: ASR Core
5. **ASR interface** - `switch_asr_interface_t` implementation
6. **Provider abstraction** - Common provider interface
7. **Media bug integration** - Audio capture from calls
8. **ASR application** - `start_asr`, `play_and_asr` dialplan apps

### Phase 3: Cloud ASR
9. **WebSocket client** - Reusable WebSocket connection handling
10. **Alibaba ASR provider** - Cloud streaming recognition
11. **Result parsing** - JSON parsing for recognition results

### Phase 4: Local ASR
12. **FunASR provider** - HTTP/gRPC client for local ASR
13. **Audio format handling** - Transcoding if needed

### Phase 5: Provider Management
14. **Provider registry** - Dynamic provider registration
15. **Health monitor** - Background health checking
16. **Failover logic** - Automatic provider switching

### Phase 6: TTS Implementation
17. **TTS interface** - `switch_speech_interface_t` implementation
18. **Alibaba TTS provider** - Cloud synthesis
19. **sherpa-onnx TTS provider** - Local synthesis
20. **TTS provider manager** - Same pattern as ASR

## Sources

- FreeSWITCH Modular Design Pattern: https://zread.ai/signalwire/freeswitch/12-modular-design-pattern (HIGH confidence)
- FreeSWITCH mod_unimrcp Documentation: https://developer.signalwire.com/freeswitch/FreeSWITCH-Explained/Modules/mod_unimrcp_6586728 (HIGH confidence - official docs)
- FreeSWITCH mod_pocketsphinx Reference: https://developer.signalwire.com/freeswitch/FreeSWITCH-Explained/Modules/mod_pocketsphinx_13173540 (HIGH confidence - official docs)
- cdevelop/FreeSWITCH-ASR Implementation: https://github.com/cdevelop/FreeSWITCH-ASR (MEDIUM confidence - community implementation)
- wangkaisine/mrcp-plugin-with-freeswitch: https://github.com/wangkaisine/mrcp-plugin-with-freeswitch (MEDIUM confidence - reference architecture, MRCP approach)

---
*Architecture research for: FreeSWITCH ASR/TTS Module Development*
*Researched: 2026-03-26*