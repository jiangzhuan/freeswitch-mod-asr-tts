---
phase: 01-asr-core-module
plan: 01.2
type: gap_closure
wave: 2
depends_on:
  - 01.1
gap_closure: true
parent_verification: 01-VERIFICATION.md
requirements:
  - ASR-01
  - ASR-03
files_modified:
  - src/mod_asr_tts.c
  - src/asr/asr_interface.c
autonomous: true
---

# Gap: Wire ASR Interface to Real Implementation

## Problem
`mod_asr_tts.c` uses stub callbacks that bypass `asr_interface.c`. `start_asr` app does nothing but log.

## Solution
Connect module entry point to real ASR interface implementation.

## Tasks

### Task 1: Import real ASR functions
- Remove stub callbacks from mod_asr_tts.c
- Include asr_interface.h
- Use real `asr_open`, `asr_close`, `asr_feed`, `asr_results` from asr_interface.c

### Task 2: Implement start_asr application
- Get session from app data
- Create ASR handle with provider config
- Call `asr_open` with session channel UUID
- Store ASR handle in channel variable

### Task 3: Wire media bug to ASR feed
- In media_bug callback, get ASR handle from session
- Submit audio processing task to worker pool
- Worker task calls `asr_feed` with resampled audio

### Task 4: Handle ASR results
- `on_result` callback publishes ESL events
- Store results for `asr_results` retrieval
- Handle is_final flag for end-of-utterance

## Acceptance Criteria
- [ ] `start_asr` app creates ASR session
- [ ] Audio flows through media bug → worker → ASR provider
- [ ] ESL events published on ASR results
- [ ] `asr_status` shows active sessions