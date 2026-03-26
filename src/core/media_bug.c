#include "media_bug.h"
#include "worker.h"

static void* buffer_consumer_thread(switch_thread_t *thread, void *data) {
    asr_session_ctx_t *ctx = (asr_session_ctx_t *)data;
    int16_t audio_buf[1600];
    size_t audio_len;
    
    while (ctx->running && ctx->active) {
        audio_len = sizeof(audio_buf);
        if (buffer_read(ctx->buffer, audio_buf, &audio_len) == SWITCH_STATUS_SUCCESS && audio_len > 0) {
            if (ctx->audio_processor) {
                ctx->audio_processor(ctx->processor_ctx, audio_buf, audio_len / sizeof(int16_t));
            }
        } else {
            switch_yield(10000);
        }
    }
    
    return NULL;
}

static switch_bool_t asr_media_bug_callback(switch_media_bug_t *bug, void *user_data, switch_abc_type_t type) {
    asr_session_ctx_t *ctx = (asr_session_ctx_t *)user_data;
    
    switch (type) {
        case SWITCH_ABC_TYPE_INIT:
            ctx->active = SWITCH_TRUE;
            ctx->running = SWITCH_TRUE;
            ctx->frame_count = 0;
            
            if (ctx->buffer) {
                switch_threadattr_t *thd_attr = NULL;
                switch_memory_pool_t *pool = switch_core_session_get_pool(ctx->session);
                if (pool && switch_threadattr_create(&thd_attr, pool) == SWITCH_STATUS_SUCCESS) {
                    switch_threadattr_detach_set(thd_attr, 1);
                    switch_thread_create(&ctx->consumer_thread, thd_attr, buffer_consumer_thread, ctx, pool);
                }
            }
            
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
            ctx->running = SWITCH_FALSE;
            ctx->active = SWITCH_FALSE;
            
            if (ctx->consumer_thread) {
                switch_status_t retval;
                switch_thread_join(&retval, ctx->consumer_thread);
                ctx->consumer_thread = NULL;
            }
            
            switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(ctx->session), SWITCH_LOG_INFO, 
                "ASR media bug closed, frames processed: %u, buffer available: %zu\n", 
                ctx->frame_count, buffer_available(ctx->buffer));
            break;
            
        default:
            break;
    }
    
    return SWITCH_TRUE;
}

switch_status_t media_bug_attach(asr_session_ctx_t **ctx, switch_core_session_t *session,
                                  audio_processor_fn processor, void *processor_ctx) {
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
    new_ctx->running = SWITCH_FALSE;
    new_ctx->frame_count = 0;
    new_ctx->consumer_thread = NULL;
    new_ctx->audio_processor = processor;
    new_ctx->processor_ctx = processor_ctx;
    
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
                                        new_ctx, 0, SMBF_READ_REPLACE, &new_ctx->bug);
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
    
    c->running = SWITCH_FALSE;
    
    if (c->bug) {
        switch_core_media_bug_remove(c->session, &c->bug);
        c->bug = NULL;
    }
    
    if (c->consumer_thread) {
        switch_status_t retval;
        switch_thread_join(&retval, c->consumer_thread);
        c->consumer_thread = NULL;
    }
    
    buffer_destroy(&c->buffer);
    resampler_destroy(&c->resampler);
    
    if (c->pool) {
        switch_core_destroy_memory_pool(&c->pool);
    }
    
    *ctx = NULL;
}