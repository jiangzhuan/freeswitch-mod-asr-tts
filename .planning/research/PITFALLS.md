# Pitfalls Research

**Domain:** FreeSWITCH ASR/TTS Module Development
**Researched:** 2026-03-26
**Confidence:** HIGH (based on official documentation, reference implementations, and commercial project analysis)

## Critical Pitfalls

### Pitfall 1: Audio Codec Mismatch in Streaming ASR

**What goes wrong:**
ASR services typically expect 16kHz PCM audio, but telephony systems use 8kHz (G.711 PCMU/PCMA). Developers often send raw 8kHz audio to ASR without resampling, resulting in poor recognition accuracy or complete failure.

**Why it happens:**
FreeSWITCH default codecs (PCMU/PCMA) are 8kHz. Developers assume the audio is "good enough" and skip resampling, or they resample incorrectly without proper anti-aliasing filtering.

**How to avoid:**
- Always resample 8kHz telephony audio to 16kHz before sending to ASR
- Use FreeSWITCH's built-in resampling (`switch_resample_create` / `switch_resample_process`)
- For Alibaba Cloud ASR: explicitly request 16kHz sample rate
- Verify audio format at module initialization, reject mismatched streams early

**Warning signs:**
- ASR returns empty or garbage results
- Recognition accuracy significantly lower than expected
- ASR service reports "invalid audio format" errors

**Phase to address:**
Phase 1 (ASR Core Module) - Establish audio pipeline with proper resampling

---

### Pitfall 2: Memory Leaks in Audio Buffer Processing

**What goes wrong:**
Streaming ASR requires continuous audio buffer allocation/deallocation. Developers often forget to free buffers after sending to ASR, or create new buffers for each chunk instead of reusing, causing gradual memory exhaustion.

**Why it happens:**
C language memory management is manual. In the streaming loop, developers focus on audio data flow and forget cleanup. FreeSWITCH's reference-counted objects (switch_buffer) require explicit management.

**How to avoid:**
```c
// WRONG: Creating new buffer each iteration
while (reading) {
    switch_buffer_create(pool, &buf, chunk_size);  // Leak!
    process(buf);
}

// CORRECT: Reuse buffer, explicit cleanup
switch_buffer_create(pool, &buf, chunk_size);
while (reading) {
    switch_buffer_zero(buf);
    process(buf);
}
switch_buffer_destroy(&buf);
```
- Use FreeSWITCH's memory pool (`switch_memory_pool_t`) for allocations
- Pair every `*_create()` with corresponding `*_destroy()` in module shutdown
- Use valgrind during development to detect leaks
- Monitor FreeSWITCH memory usage with `fs_cli` commands

**Warning signs:**
- FreeSWITCH memory usage grows over time
- System becomes sluggish after hours of operation
- OOM killer terminates FreeSWITCH process

**Phase to address:**
Phase 1 (ASR Core Module) - Implement proper memory management from day one

---

### Pitfall 3: WebSocket Connection State Mishandling

**What goes wrong:**
Streaming ASR (Alibaba Cloud, FunASR) uses WebSocket connections. Developers treat WebSocket as "always connected" and don't handle reconnection, causing the ASR to silently fail after network hiccups or server timeouts.

**Why it happens:**
WebSocket connections can drop silently. Developers test in stable environments and don't encounter disconnections during development. ASR SDKs often don't provide clear error messages for connection state issues.

**How to avoid:**
- Implement connection state machine: CONNECTING → CONNECTED → DISCONNECTED → RECONNECTING
- Set connection timeout (Alibaba Cloud recommends 30s idle timeout)
- Implement exponential backoff reconnection
- Send periodic "heartbeat" frames to detect stale connections
- Queue audio during reconnection, don't drop
- On reconnect, reinitialize ASR session with proper token

```c
// Connection state handling pattern
typedef enum {
    WS_DISCONNECTED,
    WS_CONNECTING,
    WS_CONNECTED,
    WS_RECONNECTING
} ws_state_t;

ws_state_t state = WS_DISCONNECTED;

void on_audio_chunk(audio_data) {
    if (state != WS_CONNECTED) {
        queue_audio(audio_data);  // Buffer during reconnection
        if (state == WS_DISCONNECTED) {
            initiate_reconnect();
        }
        return;
    }
    send_to_asr(audio_data);
}
```

**Warning signs:**
- ASR stops returning results mid-call
- No error messages in logs
- ASR works initially but fails after ~30 seconds of silence

**Phase to address:**
Phase 1 (ASR Core Module) - WebSocket connection management

---

### Pitfall 4: Blocking ASR API Calls in Audio Thread

**What goes wrong:**
Developers make synchronous ASR API calls in FreeSWITCH's media thread, blocking audio processing. This causes audio stuttering, delayed responses, and poor user experience.

**Why it happens:**
Synchronous APIs are simpler to understand. Developers don't realize FreeSWITCH's media thread must return quickly (within ~20ms for 50pps). Network calls to ASR services can take 100ms+.

**How to avoid:**
- Use separate thread pool for ASR API calls
- Use async/non-blocking WebSocket operations
- Never block in `switch_io_read_frame()` or `switch_io_write_frame()`
- Use FreeSWITCH's thread primitives: `switch_thread_create`, `switch_mutex`

```c
// WRONG: Blocking in read frame
switch_status_t read_frame(session, frame) {
    asr_result = sync_asr_call(frame->data);  // BLOCKS!
    return SWITCH_STATUS_SUCCESS;
}

// CORRECT: Async processing
switch_status_t read_frame(session, frame) {
    queue_audio_for_async_processing(frame->data);  // Non-blocking
    return SWITCH_STATUS_SUCCESS;
}

// Separate worker thread processes queue and sends to ASR
```

**Warning signs:**
- Audio stuttering during ASR processing
- "Media timer violation" warnings in FreeSWITCH logs
- Delayed ASR results (500ms+ latency)

**Phase to address:**
Phase 1 (ASR Core Module) - Threading architecture

---

### Pitfall 5: Missing VAD Integration

**What goes wrong:**
Developers send continuous audio stream to ASR including silence, wasting bandwidth and ASR quota, causing false positive recognitions from background noise.

**Why it happens:**
ASR "just works" without VAD during testing. Developers don't realize the cost and accuracy implications until production. Telephony audio has significant background noise.

**How to avoid:**
- Implement VAD before ASR using FunASR's `fsmn-vad` model or energy-based VAD
- Only send voice-active segments to ASR
- Use VAD end-of-speech detection to trigger ASR finalization
- Consider noise suppression for telephony audio

**Implementation pattern:**
```c
// VAD + ASR pipeline
vad_result = vad_process(audio_chunk);
if (vad_result == VOICE_ACTIVE) {
    send_to_asr(audio_chunk);
} else if (vad_result == SPEECH_END) {
    finalize_asr();  // Get final result
}
```

**Warning signs:**
- High ASR API costs (sending too much audio)
- False recognitions from background noise
- ASR returns results for "silence"

**Phase to address:**
Phase 1 (ASR Core Module) - VAD integration

---

### Pitfall 6: Incorrect ESL Event Handling for ASR Results

**What goes wrong:**
ASR results are sent via ESL events, but developers don't format them correctly or use wrong event types, causing Java business system to miss or misparse results.

**Why it happens:**
ESL event format is flexible but requires specific headers. Developers guess the format instead of following established patterns from reference implementations.

**How to avoid:**
- Use `CUSTOM` event type with proper subclass
- Include all required headers: `Channel`, `ASR-Response`, `Event-Sequence`
- Follow the format from FreeSWITCH-ASR reference implementation:

```c
// Correct ESL event format for ASR results
switch_event_t *event;
switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, "asr");
switch_event_add_header(event, SWITCH_STACK_BOTTOM, "Channel", "%s", channel_uuid);
switch_event_add_header(event, SWITCH_STACK_BOTTOM, "ASR-Response", "%s", json_result);
switch_event_fire(&event);
```

- Java side must subscribe to `custom asr` events:
```
/event custom asr
```

**Warning signs:**
- Java system doesn't receive ASR results
- ESL events appear but with missing data
- Inconsistent event format between calls

**Phase to address:**
Phase 1 (ASR Core Module) - ESL integration

---

### Pitfall 7: TTS Audio Playback Blocking on Network

**What goes wrong:**
TTS streaming (receiving audio chunks while playing) is implemented incorrectly - the module waits for complete TTS audio before playing, causing 1-2 second delays before user hears anything.

**Why it happens:**
Simple implementation: download all → play all. Developers don't implement proper streaming playback with buffering.

**How to avoid:**
- Implement streaming playback: receive chunk → buffer → play
- Use FreeSWITCH's `switch_ivr_play_file` with file descriptor for streaming
- Pre-buffer first 200-500ms of audio before starting playback
- Use circular buffer for continuous playback

```c
// Streaming TTS playback pattern
tts_stream = connect_tts_stream(text);
buffer_first_chunks();  // Pre-buffer 300ms

while (tts_stream_active) {
    audio_chunk = read_tts_chunk(tts_stream);
    if (audio_chunk) {
        write_to_playback_buffer(audio_chunk);
    }
    if (playback_buffer_has_data()) {
        feed_to_freeswitch_playback();
    }
}
```

**Warning signs:**
- 1-2 second delay before TTS audio starts
- User thinks system is broken due to silence
- Works fine with short phrases, fails with long responses

**Phase to address:**
Phase 2 (TTS Module) - Streaming TTS implementation

---

### Pitfall 8: Service Failover Without Health Checking

**What goes wrong:**
Automatic fallback from cloud ASR/TTS to local is implemented, but without health checking. System falls back on first error (even temporary) or never detects cloud service recovery.

**Why it happens:**
Simple error handling: if error → fallback. Developers don't implement proper health monitoring and hysteresis.

**How to avoid:**
- Implement health check for each service endpoint
- Use hysteresis: require N consecutive failures before fallback
- Implement recovery: periodically test cloud service during fallback
- Log all state transitions for debugging

```c
// Failover with health checking
#define FAILURE_THRESHOLD 3
#define RECOVERY_CHECK_INTERVAL 30  // seconds

typedef struct {
    int consecutive_failures;
    time_t last_recovery_check;
    service_state_t state;  // ACTIVE, FALLBACK, RECOVERING
} service_health_t;

void on_asr_error() {
    health.consecutive_failures++;
    if (health.consecutive_failures >= FAILURE_THRESHOLD) {
        switch_to_local_asr();
        health.state = FALLBACK;
    }
}

void periodic_health_check() {
    if (health.state == FALLBACK) {
        if (time(NULL) - health.last_recovery_check > RECOVERY_CHECK_INTERVAL) {
            if (test_cloud_asr_health()) {
                switch_to_cloud_asr();
                health.state = ACTIVE;
                health.consecutive_failures = 0;
            }
            health.last_recovery_check = time(NULL);
        }
    }
}
```

**Warning signs:**
- Immediate fallback on single network timeout
- Never recovers to cloud service after outage
- Unpredictable service selection during partial outages

**Phase to address:**
Phase 3 (Multi-Provider Switching) - Failover logic

---

## Technical Debt Patterns

Shortcuts that seem reasonable but create long-term problems.

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| Hard-code ASR API credentials | Faster initial development | Security risk, requires code changes for rotation | Never - use config files |
| Single-threaded ASR processing | Simpler code | Poor performance under load | Development only |
| No audio resampling | Works with test audio | Recognition failure with real telephony audio | Never |
| Synchronous WebSocket calls | Easier to reason about | Blocks audio thread, stuttering | Never |
| Skip VAD integration | Simpler pipeline | Higher costs, noise issues | Never in production |
| Fixed buffer sizes | Simpler memory management | Memory waste or truncation | Only if buffer is proven sufficient |
| No reconnection logic | Simpler code | Silent failures in production | Never |

## Integration Gotchas

Common mistakes when connecting to external services.

| Integration | Common Mistake | Correct Approach |
|-------------|----------------|------------------|
| Alibaba Cloud ASR | Sending 8kHz audio directly | Resample to 16kHz before sending |
| Alibaba Cloud ASR | Ignoring token expiration | Implement token refresh before expiry |
| FunASR WebSocket | No ping/pong heartbeat | Send periodic heartbeats, detect stale connections |
| FunASR | Not setting `is_final=True` on last chunk | Always set is_final on speech end for complete recognition |
| ESL to Java | Using wrong event subclass | Use `CUSTOM` with `asr` subclass, subscribe with `/event custom asr` |
| FreeSWITCH config | Using "auto" for IP addresses | Always specify explicit IP addresses in MRCP profiles |
| TTS streaming | Waiting for complete audio | Stream chunks with pre-buffering |
| sherpa-onnx TTS | Single-threaded inference | Use ONNX runtime with multi-threading for better performance |

## Performance Traps

Patterns that work at small scale but fail as usage grows.

| Trap | Symptoms | Prevention | When It Breaks |
|------|----------|------------|----------------|
| Per-call WebSocket connections | Connection exhaustion, slow startup | Connection pooling, reuse connections | 50+ concurrent calls |
| Unlimited audio queue memory | Memory growth, eventual OOM | Bounded queues with backpressure | Long calls or many concurrent |
| Single ASR processing thread | Processing latency increases | Thread pool for parallel processing | 20+ concurrent calls |
| No connection timeout | Threads hang waiting for ASR | Set explicit timeouts (5s connect, 30s idle) | Network issues |
| Audio buffer without bounds | Memory exhaustion | Ring buffer with fixed size | Any production use |
| Synchronous config reload | Blocks all calls during reload | Async config reload with atomic swap | Any production use |

## Security Mistakes

Domain-specific security issues beyond general web security.

| Mistake | Risk | Prevention |
|---------|------|------------|
| Hard-coded API keys in source | Credential exposure in git history | Use FreeSWITCH vars.xml or environment variables |
| Logging full ASR audio/text | Privacy violation, compliance issues | Log metadata only, redact sensitive content |
| No TLS for ASR WebSocket | Audio interception, credential theft | Always use wss:// for cloud services |
| No rate limiting per call | ASR quota exhaustion attack | Implement per-channel rate limits |
| Trusting ASR results without validation | Injection attacks via malicious audio | Sanitize ASR output before use in downstream systems |

## UX Pitfalls

Common user experience mistakes in this domain.

| Pitfall | User Impact | Better Approach |
|---------|-------------|-----------------|
| Delayed TTS response | User thinks system broken | Stream TTS with pre-buffering, show "thinking" indicator |
| No audio feedback during processing | User talks over response | Play subtle tone when switching from listening to speaking |
| ASR timeout too short | User cut off mid-sentence | Use VAD-based timeout, not fixed duration |
| ASR timeout too long | Awkward silence | Adjust based on context (question vs statement) |
| No error recovery | Call terminates on error | Graceful degradation, offer retry or alternative |
| Robot voice without normalization | Jarring audio experience | Apply audio normalization to TTS output |

## "Looks Done But Isn't" Checklist

Things that appear complete but are missing critical pieces.

- [ ] **ASR Module:** Often missing proper cleanup on channel hangup — verify all resources freed in `on_hangup` handler
- [ ] **ASR Module:** Often missing token refresh logic — verify tokens refresh before expiry, not on error
- [ ] **ASR Module:** Often missing connection state recovery — verify reconnection works after network drop
- [ ] **TTS Module:** Often missing streaming playback — verify audio starts within 200ms of TTS request
- [ ] **TTS Module:** Often missing playback interruption — verify TTS stops immediately on user speech detection
- [ ] **Multi-provider:** Often missing recovery from fallback — verify system returns to cloud after recovery
- [ ] **Configuration:** Often missing runtime reload — verify config changes apply without restart
- [ ] **ESL Integration:** Often missing event formatting — verify Java receives all required fields

## Recovery Strategies

When pitfalls occur despite prevention, how to recover.

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| Memory leak | MEDIUM | Identify leak source with valgrind, add missing free/destroy calls |
| WebSocket connection stuck | LOW | Force reconnect on timeout, clear audio queue |
| Thread deadlock | HIGH | Identify with gdb, redesign locking order |
| Audio codec mismatch | LOW | Add resampling step, no code changes needed |
| Config error | LOW | Reload config via ESL `reloadxml` command |
| ASR service outage | MEDIUM | Switch to local fallback, monitor for recovery |
| TTS streaming stuck | MEDIUM | Reset TTS session, resume from last text position |

## Pitfall-to-Phase Mapping

How roadmap phases should address these pitfalls.

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| Audio Codec Mismatch | Phase 1 (ASR Core) | Test with 8kHz telephony audio, verify 16kHz output |
| Memory Leaks | Phase 1 (ASR Core) | Run valgrind overnight, verify no leaks |
| WebSocket State Mishandling | Phase 1 (ASR Core) | Disconnect network mid-call, verify reconnection |
| Blocking ASR Calls | Phase 1 (ASR Core) | Monitor audio latency, verify < 100ms |
| Missing VAD | Phase 1 (ASR Core) | Send silence to ASR, verify no API calls |
| Incorrect ESL Events | Phase 1 (ASR Core) | Verify Java receives correctly formatted events |
| TTS Blocking | Phase 2 (TTS Module) | Measure time-to-first-audio, verify < 300ms |
| Failover Without Health Check | Phase 3 (Multi-Provider) | Simulate cloud outage, verify fallback and recovery |

## Sources

- FreeSWITCH official documentation: https://developer.signalwire.com/freeswitch/
- FreeSWITCH modular design pattern: https://zread.ai/signalwire/freeswitch/12-modular-design-pattern
- mod_unimrcp documentation: https://developer.signalwire.com/freeswitch/FreeSWITCH-Explained/Modules/mod_unimrcp_6586728
- FreeSWITCH-ASR reference implementation: https://github.com/cdevelop/FreeSWITCH-ASR
- MRCP plugin with FreeSWITCH: https://github.com/wangkaisine/mrcp-plugin-with-freeswitch
- FunASR documentation: https://github.com/modelscope/FunASR
- easycallcenter365 commercial implementation: https://github.com/easycallcenter365/easycallcenter365

---
*Pitfalls research for: FreeSWITCH ASR/TTS Module Development*
*Researched: 2026-03-26*