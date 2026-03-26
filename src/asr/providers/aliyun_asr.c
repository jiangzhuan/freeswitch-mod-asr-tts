#include "aliyun_asr.h"
#include <string.h>

static void* recv_thread_func(switch_thread_t *thread, void *data) {
    aliyun_asr_provider_t *provider = (aliyun_asr_provider_t *)data;
    
    while (provider->running) {
        switch_yield(1000000);
    }
    
    return NULL;
}

aliyun_asr_provider_t* aliyun_asr_create(const char *url, const char *appkey, const char *token) {
    aliyun_asr_provider_t *provider;
    switch_memory_pool_t *pool;
    
    if (switch_core_new_memory_pool(&pool) != SWITCH_STATUS_SUCCESS) {
        return NULL;
    }
    
    provider = switch_core_alloc(pool, sizeof(*provider));
    if (!provider) {
        switch_core_destroy_memory_pool(&pool);
        return NULL;
    }
    
    provider->state = WS_DISCONNECTED;
    provider->running = SWITCH_FALSE;
    provider->reconnect_interval = 1;
    provider->max_reconnect_interval = 60;
    provider->connect_timeout = 10;
    provider->idle_timeout = 300;
    provider->heartbeat_interval = 15;
    
    if (url) {
        strncpy(provider->websocket_url, url, sizeof(provider->websocket_url) - 1);
    } else {
        strncpy(provider->websocket_url, "wss://nls-gateway.cn-shanghai.aliyuncs.com/ws/v1", 
                sizeof(provider->websocket_url) - 1);
    }
    if (appkey) {
        strncpy(provider->appkey, appkey, sizeof(provider->appkey) - 1);
    }
    if (token) {
        strncpy(provider->token, token, sizeof(provider->token) - 1);
    }
    
    switch_mutex_init(&provider->mutex, SWITCH_MUTEX_NESTED, pool);
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, 
        "Aliyun ASR provider created (WebSocket requires curl 7.86+)\n");
    
    return provider;
}

void aliyun_asr_destroy(aliyun_asr_provider_t **provider) {
    if (!provider || !*provider) {
        return;
    }
    
    aliyun_asr_disconnect(*provider);
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Aliyun ASR provider destroyed\n");
    
    *provider = NULL;
}

switch_status_t aliyun_asr_connect(aliyun_asr_provider_t *provider) {
    switch_threadattr_t *thd_attr = NULL;
    switch_memory_pool_t *pool;
    
    if (!provider) {
        return SWITCH_STATUS_FALSE;
    }
    
    switch_mutex_lock(provider->mutex);
    
    if (provider->state == WS_CONNECTED) {
        switch_mutex_unlock(provider->mutex);
        return SWITCH_STATUS_SUCCESS;
    }
    
    provider->running = SWITCH_TRUE;
    provider->state = WS_CONNECTING;
    
    if (switch_core_new_memory_pool(&pool) == SWITCH_STATUS_SUCCESS) {
        if (switch_threadattr_create(&thd_attr, pool) == SWITCH_STATUS_SUCCESS) {
            switch_threadattr_detach_set(thd_attr, 1);
            switch_thread_create(&provider->recv_thread, thd_attr, recv_thread_func, provider, pool);
        }
    }
    
    provider->state = WS_CONNECTED;
    
    switch_mutex_unlock(provider->mutex);
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, 
        "Aliyun ASR connected (stub mode)\n");
    
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t aliyun_asr_send_audio(aliyun_asr_provider_t *provider, int16_t *audio, size_t len) {
    if (!provider || !audio || len == 0) {
        return SWITCH_STATUS_FALSE;
    }
    
    if (provider->state != WS_CONNECTED) {
        return SWITCH_STATUS_FALSE;
    }
    
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t aliyun_asr_send_final(aliyun_asr_provider_t *provider) {
    if (!provider) {
        return SWITCH_STATUS_FALSE;
    }
    
    if (provider->state != WS_CONNECTED) {
        return SWITCH_STATUS_FALSE;
    }
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Aliyun ASR final sent\n");
    
    return SWITCH_STATUS_SUCCESS;
}

void aliyun_asr_disconnect(aliyun_asr_provider_t *provider) {
    if (!provider) {
        return;
    }
    
    switch_mutex_lock(provider->mutex);
    
    provider->running = SWITCH_FALSE;
    
    if (provider->recv_thread) {
        switch_status_t retval;
        switch_thread_join(&retval, provider->recv_thread);
        provider->recv_thread = NULL;
    }
    
    provider->state = WS_DISCONNECTED;
    
    switch_mutex_unlock(provider->mutex);
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Aliyun ASR disconnected\n");
}

void aliyun_asr_set_callbacks(aliyun_asr_provider_t *provider, void *ctx,
                               void (*on_result)(void*, const char*, int),
                               void (*on_error)(void*, int, const char*)) {
    if (!provider) {
        return;
    }
    
    provider->callback_ctx = ctx;
    provider->on_result = on_result;
    provider->on_error = on_error;
}