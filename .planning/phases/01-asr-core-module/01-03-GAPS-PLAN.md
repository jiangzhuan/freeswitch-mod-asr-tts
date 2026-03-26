---
phase: 01-asr-core-module
plan: 01.3
type: gap_closure
wave: 3
depends_on:
  - 01.2
gap_closure: true
parent_verification: 01-VERIFICATION.md
requirements:
  - ASR-01
  - ASR-03
files_modified:
  - src/asr/asr_interface.c
  - src/core/worker.c
  - docker/Dockerfile.build
  - docker/build.sh
autonomous: true
---

# Gap: Worker Pool Integration + Docker Build Environment

## Problem
Audio captured but never sent to worker pool. No processing happens. Also, no build environment on Windows.

## Solution
Wire media bug to worker pool, add Docker build support.

## Tasks

### Task 1: Create audio processing worker task
- Define `audio_processing_task_t` with audio data and ASR handle
- Task function: VAD → ASR provider → handle results
- Free audio data after processing

### Task 2: Wire media bug to worker pool
- In `SWITCH_ABC_TYPE_READ_REPLACE`, create worker task
- Copy audio data (don't pass pointer - async processing)
- Submit to worker pool

### Task 3: Create Docker build environment
- Create `docker/Dockerfile.build` with Debian 12 + FreeSWITCH dev headers
- Create `docker/build.sh` to compile module in container
- Volume mount source code

### Task 4: Create Docker test environment
- Create `docker/Dockerfile.test` with FreeSWITCH runtime
- Create `docker/test.sh` to run integration tests
- Configure sample call with ASR

## Acceptance Criteria
- [ ] Worker tasks submitted on each audio frame
- [ ] Audio processed through VAD and ASR
- [ ] Docker build produces mod_asr_tts.so
- [ ] Module loads in containerized FreeSWITCH