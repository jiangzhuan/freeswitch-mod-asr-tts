#ifndef ASR_ASR_INTERFACE_H
#define ASR_ASR_INTERFACE_H

#include <switch.h>
#include "asr_provider.h"
#include "../core/media_bug.h"
#include "../core/vad.h"
#include "../core/worker.h"

typedef struct asr_handle_t {
    asr_provider_interface_t *provider;
    void *provider_ctx;
    
    asr_session_ctx_t *session_ctx;
    vad_context_t *vad;
    worker_pool_t *worker_pool;
    
    char *channel_uuid;
    uint32_t result_sequence;
    
    switch_bool_t running;
    switch_bool_t paused;
    
    switch_memory_pool_t *pool;
} asr_handle_t;

#endif