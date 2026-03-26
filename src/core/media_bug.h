#ifndef CORE_MEDIA_BUG_H
#define CORE_MEDIA_BUG_H

#include <switch.h>
#include "buffer.h"
#include "resample.h"

typedef struct asr_session_ctx_t asr_session_ctx_t;

typedef void (*audio_processor_fn)(void *ctx, int16_t *audio, size_t samples);

struct asr_session_ctx_t {
    switch_core_session_t *session;
    asr_ring_buffer_t *buffer;
    asr_resampler_t *resampler;
    switch_media_bug_t *bug;
    switch_memory_pool_t *pool;
    switch_thread_t *consumer_thread;
    uint32_t frame_count;
    switch_bool_t active;
    switch_bool_t running;
    void *processor_ctx;
    audio_processor_fn audio_processor;
};

switch_status_t media_bug_attach(asr_session_ctx_t **ctx, switch_core_session_t *session,
                                  audio_processor_fn processor, void *processor_ctx);
void media_bug_detach(asr_session_ctx_t **ctx);

#endif