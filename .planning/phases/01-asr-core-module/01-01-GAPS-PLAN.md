---
phase: 01-asr-core-module
plan: 01.1
type: gap_closure
wave: 1
depends_on:
  - 01-03
gap_closure: true
parent_verification: 01-VERIFICATION.md
requirements:
  - ASR-01
files_modified:
  - src/asr/providers/aliyun_asr.c
  - src/asr/providers/aliyun_asr.h
  - src/Makefile
autonomous: true
---

# Gap: WebSocket Client Implementation

## Problem
`aliyun_asr_connect()` sets state without actual WebSocket connection. `aliyun_asr_send_audio()` returns without sending audio. No receive loop to get ASR results.

## Solution
Implement real WebSocket client using libcurl WebSocket API.

## Tasks

### Task 1: Add libcurl WebSocket support
- Update Makefile to link `-lcurl`
- Include `<curl/curl.h>` in aliyun_asr.c
- Initialize curl with `curl_global_init()`

### Task 2: Implement WebSocket connection
- Use `CURLOPT_CONNECT_ONLY` and `CURLOPT_HTTP09_ALLOWED`
- Set WebSocket upgrade headers
- Handle connection timeout (5s)

### Task 3: Implement audio sending
- Use `curl_ws_send()` to transmit audio frames
- Format as binary WebSocket frames
- Handle backpressure

### Task 4: Implement receive loop
- Create background thread for receiving
- Use `curl_ws_recv()` to get ASR responses
- Parse JSON responses and invoke `on_result` callback

### Task 5: Implement heartbeat
- Send WebSocket ping every 30 seconds
- Handle pong responses
- Trigger reconnection on timeout

## Acceptance Criteria
- [ ] WebSocket connects to Aliyun Cloud ASR
- [ ] Audio frames transmitted successfully
- [ ] ASR results received and parsed
- [ ] Heartbeat maintains connection
- [ ] Exponential backoff reconnection works