---
phase: 01-asr-core-module
plan: 01.1
type: gap_closure
status: complete
completed: 2026-03-26
commit: 225215c
requirements:
  - ASR-01
---

## Summary

Implemented real WebSocket client using libcurl for Aliyun Cloud ASR connection.

## What Was Built

**libcurl WebSocket Integration**
- Added `-lcurl` to Makefile
- Real WebSocket connection via `curl_easy_init` + `CURLOPT_CONNECT_ONLY`
- Proper WebSocket upgrade headers

**WebSocket Connection**
- Connection with 5s timeout
- State management: DISCONNECTED → CONNECTING → CONNECTED
- URL with appkey and token parameters

**Audio Sending**
- Binary frame transmission via `curl_ws_send`
- Backpressure handling
- 30s heartbeat with `CURLWS_PING`

**Result Receiving**
- JSON response parsing
- `is_final` detection for end-of-utterance
- Callback invocation for results

## Acceptance Criteria
- [x] WebSocket connects to Aliyun Cloud ASR
- [x] Audio frames transmitted successfully
- [x] ASR results received and parsed
- [x] Heartbeat maintains connection