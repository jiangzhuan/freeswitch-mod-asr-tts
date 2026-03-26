#include "asr_interface.h"
#include "../core/event.h"
#include "providers/aliyun_asr.h"

static asr_provider_interface_t aliyun_provider = {
    .name = "aliyun",
    .create = (void* (*)(const char*, const char*, const char*))aliyun_asr_create,
    .destroy = (void (*)(void**))aliyun_asr_destroy,
    .connect = (switch_status_t (*)(void*))aliyun_asr_connect,
    .send_audio = (switch_status_t (*)(void*, int16_t*, size_t))aliyun_asr_send_audio,
    .send_final = (switch_status_t (*)(void*))aliyun_asr_send_final,
    .disconnect = (void (*)(void*))aliyun_asr_disconnect,
    .set_callbacks = (void (*)(void*, void*, void(*)(void*,const char*,int), void(*)(void*,int,const char*)))aliyun_asr_set_callbacks
};

static void on_asr_result(void *ctx, const char *json, int is_final) {
    asr_handle_t *ah = (asr_handle_t *)ctx;
    if (ah && ah->channel_uuid) {
        ah->result_sequence++;
        event_publish_asr_result(ah->session_ctx ? ah->session_ctx->session : NULL,
                                  ah->channel_uuid, json, is_final, ah->result_sequence);
    }
}

static void on_asr_error(void *ctx, int code, const char *message) {
    asr_handle_t *ah = (asr_handle_t *)ctx;
    if (ah && ah->channel_uuid) {
        event_publish_asr_error(ah->session_ctx ? ah->session_ctx->session : NULL,
                                 ah->channel_uuid, code, message);
    }
}

switch_status_t asr_open(switch_asr_handle_t *ah, const char *codec, int rate, const char *dest, switch_asr_flag_t *flags) {
    asr_handle_t *handle;
    switch_memory_pool_t *pool;
    
    if (switch_core_new_memory_pool(&pool) != SWITCH_STATUS_SUCCESS) {
        return SWITCH_STATUS_FALSE;
    }
    
    handle = switch_core_alloc(pool, sizeof(*handle));
    if (!handle) {
        switch_core_destroy_memory_pool(&pool);
        return SWITCH_STATUS_FALSE;
    }
    
    handle->pool = pool;
    handle->running = SWITCH_TRUE;
    handle->paused = SWITCH_FALSE;
    handle->result_sequence = 0;
    handle->channel_uuid = switch_core_strdup(pool, dest ? dest : "unknown");
    
    handle->provider = &aliyun_provider;
    
    handle->provider_ctx = handle->provider->create(NULL, NULL, NULL);
    if (handle->provider_ctx) {
        handle->provider->set_callbacks(handle->provider_ctx, handle, on_asr_result, on_asr_error);
    }
    
    handle->vad = vad_create(-40);
    if (!handle->vad) {
        switch_core_destroy_memory_pool(&pool);
        return SWITCH_STATUS_FALSE;
    }
    
    handle->worker_pool = worker_pool_create(pool, 4);
    if (!handle->worker_pool) {
        vad_destroy(&handle->vad);
        switch_core_destroy_memory_pool(&pool);
        return SWITCH_STATUS_FALSE;
    }
    
    ah->private_info = handle;
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "ASR handle opened: channel=%s\n", handle->channel_uuid);
    
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t asr_close(switch_asr_handle_t *ah, switch_asr_flag_t *flags) {
    asr_handle_t *handle = (asr_handle_t *)ah->private_info;
    
    if (!handle) {
        return SWITCH_STATUS_FALSE;
    }
    
    handle->running = SWITCH_FALSE;
    
    if (handle->session_ctx) {
        media_bug_detach(&handle->session_ctx);
    }
    
    if (handle->provider && handle->provider_ctx) {
        handle->provider->disconnect(handle->provider_ctx);
        handle->provider->destroy(&handle->provider_ctx);
    }
    
    worker_pool_destroy(&handle->worker_pool);
    vad_destroy(&handle->vad);
    
    event_publish_asr_stop(NULL, handle->channel_uuid);
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "ASR handle closed: channel=%s, results=%u\n", 
        handle->channel_uuid, handle->result_sequence);
    
    if (handle->pool) {
        switch_core_destroy_memory_pool(&handle->pool);
    }
    
    ah->private_info = NULL;
    
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t asr_feed(switch_asr_handle_t *ah, void *data, unsigned int len, switch_asr_flag_t *flags) {
    asr_handle_t *handle = (asr_handle_t *)ah->private_info;
    vad_state_t vad_state;
    
    if (!handle || !handle->running || handle->paused) {
        return SWITCH_STATUS_FALSE;
    }
    
    if (!data || len == 0) {
        return SWITCH_STATUS_SUCCESS;
    }
    
    vad_state = vad_process(handle->vad, (int16_t*)data, len / 2);
    
    if (vad_state == VAD_SPEECH && handle->provider && handle->provider_ctx) {
        handle->provider->send_audio(handle->provider_ctx, (int16_t*)data, len / 2);
    } else if (vad_state == VAD_END && handle->provider && handle->provider_ctx) {
        handle->provider->send_final(handle->provider_ctx);
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "VAD end detected, sent final\n");
    }
    
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t asr_check_results(switch_asr_handle_t *ah, switch_asr_flag_t *flags) {
    asr_handle_t *handle = (asr_handle_t *)ah->private_info;
    
    if (!handle || !handle->running) {
        return SWITCH_STATUS_FALSE;
    }
    
    return SWITCH_STATUS_BREAK;
}

switch_status_t asr_get_results(switch_asr_handle_t *ah, char **xmlstr, switch_asr_flag_t *flags) {
    asr_handle_t *handle = (asr_handle_t *)ah->private_info;
    
    if (!handle || !xmlstr) {
        return SWITCH_STATUS_FALSE;
    }
    
    *xmlstr = NULL;
    return SWITCH_STATUS_BREAK;
}

switch_status_t asr_pause(switch_asr_handle_t *ah) {
    asr_handle_t *handle = (asr_handle_t *)ah->private_info;
    
    if (!handle) {
        return SWITCH_STATUS_FALSE;
    }
    
    handle->paused = SWITCH_TRUE;
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t asr_resume(switch_asr_handle_t *ah) {
    asr_handle_t *handle = (asr_handle_t *)ah->private_info;
    
    if (!handle) {
        return SWITCH_STATUS_FALSE;
    }
    
    handle->paused = SWITCH_FALSE;
    return SWITCH_STATUS_SUCCESS;
}