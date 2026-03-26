#ifndef CORE_MEDIA_BUG_H
#define CORE_MEDIA_BUG_H

#include <switch.h>
#include "buffer.h"
#include "resample.h"

typedef struct asr_session_ctx_t {
    switch_core_session_t *session;
    asr_ring_buffer_t *buffer;
    asr_resampler_t *resampler;
    switch_media_bug_t *bug;
    switch_memory_pool_t *pool;
    uint32_t frame_count;
    switch_bool_t active;
} asr_session_ctx_t;

switch_status_t media_bug_attach(asr_session_ctx_t **ctx, switch_core_session_t *session);
void media_bug_detach(asr_session_ctx_t **ctx);

#endif