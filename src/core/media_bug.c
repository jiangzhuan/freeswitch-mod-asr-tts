#include "media_bug.h"

static switch_bool_t asr_media_bug_callback(switch_media_bug_t *bug, void *user_data, switch_abc_type_t type) {
    asr_session_ctx_t *ctx = (asr_session_ctx_t *)user_data;
    
    switch (type) {
        case SWITCH_ABC_TYPE_INIT:
            ctx->active = SWITCH_TRUE;
            ctx->frame_count = 0;
            switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(ctx->session), SWITCH_LOG_INFO, 
                "ASR media bug initialized\n");
            break;
            
        case SWITCH_ABC_TYPE_READ_REPLACE:
            if (ctx->active && ctx->buffer && ctx->resampler) {
                switch_frame_t *frame = switch_core_media_bug_get_read_replace_frame(bug);
                
                if (frame && frame->data && frame->datalen > 0) {
                    int16_t resampled[640];
                    size_t resampled_samples = sizeof(resampled) / sizeof(int16_t);
                    size_t input_samples = frame->datalen / sizeof(int16_t);
                    
                    if (resampler_process(ctx->resampler, (int16_t*)frame->data, input_samples, 
                                          resampled, &resampled_samples) == SWITCH_STATUS_SUCCESS) {
                        buffer_write(ctx->buffer, resampled, resampled_samples * sizeof(int16_t));
                        ctx->frame_count++;
                    }
                }
            }
            break;
            
        case SWITCH_ABC_TYPE_CLOSE:
            ctx->active = SWITCH_FALSE;
            switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(ctx->session), SWITCH_LOG_INFO, 
                "ASR media bug closed, frames processed: %u, buffer available: %zu\n", 
                ctx->frame_count, buffer_available(ctx->buffer));
            break;
            
        default:
            break;
    }
    
    return SWITCH_TRUE;
}

switch_status_t media_bug_attach(asr_session_ctx_t **ctx, switch_core_session_t *session) {
    switch_memory_pool_t *pool = NULL;
    asr_session_ctx_t *new_ctx = NULL;
    switch_status_t status;
    switch_codec_implementation_t read_impl = {0};
    
    if (!ctx || !session) {
        return SWITCH_STATUS_FALSE;
    }
    
    if (switch_core_new_memory_pool(&pool) != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, 
            "Failed to create memory pool\n");
        return SWITCH_STATUS_FALSE;
    }
    
    new_ctx = switch_core_alloc(pool, sizeof(*new_ctx));
    if (!new_ctx) {
        switch_core_destroy_memory_pool(&pool);
        return SWITCH_STATUS_FALSE;
    }
    
    new_ctx->pool = pool;
    new_ctx->session = session;
    new_ctx->active = SWITCH_FALSE;
    new_ctx->frame_count = 0;
    
    new_ctx->buffer = buffer_create(pool, 320000);
    if (!new_ctx->buffer) {
        switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, 
            "Failed to create ring buffer\n");
        switch_core_destroy_memory_pool(&pool);
        return SWITCH_STATUS_FALSE;
    }
    
    switch_core_session_get_read_impl(session, &read_impl);
    uint32_t from_rate = read_impl.actual_samples_per_second;
    if (from_rate == 0) {
        from_rate = 8000;
    }
    
    new_ctx->resampler = resampler_create(pool, from_rate, 16000);
    if (!new_ctx->resampler) {
        switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, 
            "Failed to create resampler\n");
        switch_core_destroy_memory_pool(&pool);
        return SWITCH_STATUS_FALSE;
    }
    
    status = switch_core_media_bug_add(session, "asr", NULL, asr_media_bug_callback, 
                                        new_ctx, 0, SMBF_READ_STREAM_REPLACE, &new_ctx->bug);
    if (status != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, 
            "Failed to add media bug\n");
        switch_core_destroy_memory_pool(&pool);
        return SWITCH_STATUS_FALSE;
    }
    
    *ctx = new_ctx;
    switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, 
        "ASR media bug attached successfully\n");
    
    return SWITCH_STATUS_SUCCESS;
}

void media_bug_detach(asr_session_ctx_t **ctx) {
    if (!ctx || !*ctx) {
        return;
    }
    
    asr_session_ctx_t *c = *ctx;
    
    if (c->bug) {
        switch_core_media_bug_remove(c->session, &c->bug);
        c->bug = NULL;
    }
    
    buffer_destroy(&c->buffer);
    resampler_destroy(&c->resampler);
    
    if (c->pool) {
        switch_core_destroy_memory_pool(&c->pool);
    }
    
    *ctx = NULL;
}