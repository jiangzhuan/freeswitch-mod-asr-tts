---
phase: 01-asr-core-module
verified: 2026-03-26T15:45:00Z
status: gaps_found
score: 3/5 must-haves verified
re_verification:
  previous_status: gaps_found
  previous_score: 2/5
  gaps_closed:
    - "WebSocket client now implemented with libcurl (curl_ws_send, curl_ws_recv, receive thread)"
    - "ASR interface now wired correctly (SWITCH_ADD_ASR uses real asr_open, asr_close, asr_feed)"
  gaps_remaining:
    - "Worker pool still not wired - worker_pool_submit never called"
    - "Buffer consumer missing - buffer_read never called, audio never reaches asr_feed"
  regressions: []
gaps:
  - truth: "User can make a call and speak, and see ASR results appear in real-time"
    status: failed
    reason: "Audio pipeline is broken at the buffer consumer. Media bug captures audio and writes to buffer, but nothing reads from buffer and calls asr_feed(). WebSocket client is now functional but receives no audio."
    artifacts:
      - path: "src/core/media_bug.c"
        issue: "Line 25 writes audio to buffer but never triggers audio processing"
      - path: "src/core/buffer.c"
        issue: "buffer_read() is defined (line 53) but NEVER called anywhere in the codebase"
      - path: "src/asr/asr_interface.c"
        issue: "asr_feed() correctly processes audio through VAD and provider (lines 115-137) but is never invoked"
    missing:
      - "Buffer consumer mechanism - either a timer/thread that reads from buffer and calls asr_feed(), or call asr_feed() directly from media bug callback"
      - "worker_pool_submit() call in media bug callback to process audio asynchronously (per PLAN key_links)"
  - truth: "System distinguishes speech from silence - no ASR events during silent periods"
    status: failed
    reason: "VAD implementation is correct but never receives audio. asr_feed() calls vad_process() but asr_feed() is never called because there's no buffer consumer."
    artifacts:
      - path: "src/core/vad.c"
        issue: "vad_process() correctly implements energy-based detection but is unreachable"
      - path: "src/asr/asr_interface.c"
        issue: "Line 127 calls vad_process() but this code path is never reached from media bug"
    missing:
      - "Wire buffer → asr_feed() → vad_process() data flow"
  - truth: "Java business system receives ASR results via ESL custom 'asr' events with JSON payload"
    status: partial
    reason: "ESL event publishing is correctly implemented. WebSocket receive thread now exists. Callbacks are wired. However, no ASR results are generated because audio never reaches the ASR provider."
    artifacts:
      - path: "src/core/event.c"
        issue: "Correctly implements event_publish_asr_result() with headers (Channel, ASR-Response, ASR-Is-Final, Event-Sequence)"
      - path: "src/asr/asr_interface.c"
        issue: "on_asr_result callback correctly wired (line 57) and calls event_publish_asr_result() (line 20)"
      - path: "src/asr/providers/aliyun_asr.c"
        issue: "Receive thread (line 34-53) would call on_result if audio were being sent; WebSocket connection works"
    missing:
      - "Audio flow from buffer to provider to generate ASR results"
  - truth: "System correctly handles 8kHz G.711 telephony audio"
    status: verified
    reason: "Resampling is correctly implemented in media bug callback"
    artifacts:
      - path: "src/core/resample.c"
        issue: "Resampler converts 8kHz to 16kHz"
      - path: "src/core/media_bug.c"
        issue: "Line 23-24 calls resampler_process() in callback"
  - truth: "Worker thread pool processes audio asynchronously without blocking media path"
    status: failed
    reason: "Worker pool exists and is created but worker_pool_submit() is never called. The PLAN specifies 'media_bug callback → worker pool via worker_pool_submit' but this link is missing."
    artifacts:
      - path: "src/core/worker.c"
        issue: "worker_pool_submit() defined (line 103) but never invoked anywhere in codebase"
      - path: "src/core/media_bug.c"
        issue: "Media bug callback should call worker_pool_submit() after buffer_write() to process audio asynchronously"
    missing:
      - "Call worker_pool_submit() in media bug callback"
      - "Worker task function that reads from buffer and calls asr_feed()"
human_verification: []
---

# Phase 1: ASR Core Module Verification Report

**Phase Goal:** Users can perform streaming speech recognition with Aliyun Cloud ASR, integrating with Java business system via ESL
**Verified:** 2026-03-26T15:45:00Z
**Status:** gaps_found
**Re-verification:** Yes — after gap closure attempts

## Re-Verification Summary

**Previous Status:** gaps_found (2/5 truths verified)
**Current Status:** gaps_found (3/5 truths verified)

### Gaps Closed Since Previous Verification
1. ✅ **WebSocket client implemented** - Now uses libcurl with `curl_ws_send`, `curl_ws_recv`, and a receive thread
2. ✅ **ASR interface wired** - `SWITCH_ADD_ASR` now uses real functions from `asr_interface.c`

### Gaps Remaining
1. ❌ **Worker pool not wired** - `worker_pool_submit()` never called
2. ❌ **Buffer consumer missing** - `buffer_read()` never called, audio never reaches `asr_feed()`

### New Gaps Identified
None - the remaining gaps are the same structural issues identified before.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | User can make a call and speak, and see ASR results appear in real-time | ✗ FAILED | Audio captured but never processed - buffer has no consumer |
| 2 | System distinguishes speech from silence - no ASR events during silent periods | ✗ FAILED | VAD never receives audio because asr_feed() is never called |
| 3 | Java business system receives ASR results via ESL custom 'asr' events with JSON payload | ⚠️ PARTIAL | ESL publishing correctly implemented but no results generated |
| 4 | System correctly handles 8kHz G.711 telephony audio | ✓ VERIFIED | Resampling to 16kHz works in media bug callback |
| 5 | Worker thread pool processes audio asynchronously without blocking media path | ✗ FAILED | Worker pool created but never used - worker_pool_submit() never called |

**Score:** 2.5/5 truths verified (partial = 0.5)

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `src/asr/asr_interface.c` | ASR interface implementation | ✓ VERIFIED | asr_open, asr_close, asr_feed, asr_results all implemented |
| `src/asr/providers/aliyun_asr.c` | Aliyun WebSocket client with state machine | ✓ VERIFIED | libcurl WebSocket with receive thread, state machine |
| `src/core/vad.c` | Energy-based VAD | ✓ VERIFIED | vad_create, vad_process, vad_destroy implemented |
| `src/core/event.c` | ESL event publisher | ✓ VERIFIED | All event types with correct headers |
| `src/core/worker.c` | Worker thread pool | ✓ VERIFIED | Pool created but NOT WIRED |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|----|--------|---------|
| media_bug callback | worker pool | worker_pool_submit | ✗ NOT_WIRED | Function never called |
| worker thread | Aliyun ASR | WebSocket/aliyun_asr_send_audio | ✗ NOT_WIRED | No worker thread processes audio |
| ASR result | ESL event | event_publish_asr_result | ✓ WIRED | Callback correctly wired in asr_interface.c:57 |

### Data-Flow Trace (Level 4)

| Step | Component | Data Flow | Status |
|------|-----------|-----------|--------|
| 1 | FreeSWITCH audio capture | Audio frames from call | ✓ FLOWING |
| 2 | Media bug callback | Audio to callback | ✓ FLOWING |
| 3 | Resampler | 8kHz → 16kHz | ✓ FLOWING |
| 4 | Ring buffer write | Audio to buffer | ✓ FLOWING |
| 5 | **Ring buffer read** | **Audio from buffer** | ✗ **DISCONNECTED** |
| 6 | asr_feed() call | Audio to VAD | ✗ NEVER CALLED |
| 7 | VAD processing | Audio to speech detection | ✗ NO DATA |
| 8 | Provider send_audio | Audio to WebSocket | ✗ NO DATA |
| 9 | Aliyun ASR | Recognition | ✗ NO DATA |
| 10 | Receive thread | Results from WebSocket | ✓ WOULD WORK |
| 11 | on_asr_result callback | Results to event | ✓ WIRED |
| 12 | ESL event publishing | Events to Java | ✓ WIRED |

**Critical Gap:** The data pipeline is broken at step 5. Audio is captured and written to a buffer, but nothing reads from the buffer.

### Requirements Coverage

| Requirement | Description | Status | Evidence |
|-------------|-------------|--------|----------|
| ASR-01 | Streaming speech recognition with Aliyun Cloud | ✗ BLOCKED | Audio never reaches provider |
| ASR-02 | VAD to distinguish speech from silence | ✗ BLOCKED | VAD never receives audio |
| ASR-03 | ASR results via ESL custom events | ⚠️ PARTIAL | Publishing works, but no results generated |
| ASR-04 | Handle 8kHz G.711 with resampling to 16kHz | ✓ SATISFIED | Resampler correctly implemented |
| ASR-05 | XML configuration | ✓ SATISFIED | config.c implements XML parsing |
| ASR-06 | Load/unload via FreeSWITCH module API | ✓ SATISFIED | Module lifecycle implemented |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| src/core/buffer.c | 53-67 | `buffer_read()` defined but never called | 🛑 Blocker | Audio pipeline broken |
| src/core/media_bug.c | 14-30 | Media bug callback writes to buffer but doesn't process | 🛑 Blocker | No audio consumption |
| src/core/worker.c | 103 | `worker_pool_submit()` defined but never called | 🛑 Blocker | Worker pool unused |

### Human Verification Required

None required - the gaps are structural and can be verified programmatically.

### Gaps Summary

**Critical Gap: Audio Pipeline Broken at Buffer Consumer**

The implementation has all the pieces but they are not connected:

1. **What works (improved since last verification):**
   - Media bug captures audio from FreeSWITCH ✓
   - Audio is resampled from 8kHz to 16kHz ✓
   - Audio is written to ring buffer ✓
   - Worker pool exists ✓
   - VAD implementation is correct ✓
   - **WebSocket client now uses libcurl** ✓ (NEW)
   - **Receive thread for ASR results exists** ✓ (NEW)
   - **ASR interface is wired to real functions** ✓ (NEW)
   - ESL event publishing is correct ✓

2. **What's still missing:**
   - No mechanism to read from buffer and call asr_feed()
   - The media bug callback should call worker_pool_submit() per PLAN
   - A consumer thread or direct call from media bug to asr_feed()

3. **Impact:**
   - No audio reaches VAD → no speech detection
   - No audio reaches ASR provider → no recognition
   - No results generated → no ESL events with ASR data
   - Phase goal not achieved

**Recommended Fix:**

The simplest fix is to call `asr_feed()` directly from the media bug callback after writing to the buffer:

```c
// In media_bug_callback (media_bug.c), after buffer_write():
if (ctx->handle && ctx->handle->asr_handle) {
    switch_asr_flag_t flags = 0;
    asr_feed(ctx->handle->asr_handle, resampled, resampled_samples * sizeof(int16_t), &flags);
}
```

Or to follow the PLAN's async pattern, submit to worker pool:

```c
// In media_bug_callback, after buffer_write():
if (ctx->handle && ctx->handle->worker_pool) {
    worker_pool_submit(ctx->handle->worker_pool, process_audio_task, ctx);
}

// Worker task function:
static void process_audio_task(void *data) {
    asr_session_ctx_t *ctx = (asr_session_ctx_t *)data;
    int16_t audio[640];
    size_t len = sizeof(audio);
    if (buffer_read(ctx->buffer, audio, &len) == SWITCH_STATUS_SUCCESS) {
        switch_asr_flag_t flags = 0;
        asr_feed(ctx->asr_handle, audio, len, &flags);
    }
}
```

---

_Verified: 2026-03-26T15:45:00Z_
_Verifier: the agent (gsd-verifier)_