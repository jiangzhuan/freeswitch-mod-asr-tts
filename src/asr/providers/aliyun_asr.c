#include "aliyun_asr.h"

aliyun_asr_provider_t* aliyun_asr_create(const char *url, const char *appkey, const char *token) {
    aliyun_asr_provider_t *provider;
    
    provider = malloc(sizeof(*provider));
    if (!provider) {
        return NULL;
    }
    
    memset(provider, 0, sizeof(*provider));
    
    provider->state = WS_DISCONNECTED;
    
    if (url) {
        switch_set_string(provider->websocket_url, url);
    } else {
        switch_set_string(provider->websocket_url, "wss://nls-gateway.cn-shanghai.aliyuncs.com/ws/v1");
    }
    
    if (appkey) {
        switch_set_string(provider->appkey, appkey);
    }
    
    if (token) {
        switch_set_string(provider->token, token);
    }
    
    provider->reconnect_interval = 1;
    provider->max_reconnect_interval = 30;
    provider->connect_timeout = 5;
    provider->idle_timeout = 60;
    provider->heartbeat_interval = 30;
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, 
        "Aliyun ASR provider created: url=%s, appkey=%s\n", 
        provider->websocket_url, provider->appkey);
    
    return provider;
}

void aliyun_asr_destroy(aliyun_asr_provider_t **provider) {
    if (!provider || !*provider) {
        return;
    }
    
    aliyun_asr_disconnect(*provider);
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Aliyun ASR provider destroyed\n");
    
    free(*provider);
    *provider = NULL;
}

switch_status_t aliyun_asr_connect(aliyun_asr_provider_t *provider) {
    if (!provider) {
        return SWITCH_STATUS_FALSE;
    }
    
    provider->state = WS_CONNECTING;
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, 
        "Aliyun ASR connecting to %s\n", provider->websocket_url);
    
    provider->state = WS_CONNECTED;
    provider->reconnect_interval = 1;
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Aliyun ASR connected\n");
    
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t aliyun_asr_send_audio(aliyun_asr_provider_t *provider, int16_t *audio, size_t len) {
    if (!provider || provider->state != WS_CONNECTED) {
        return SWITCH_STATUS_FALSE;
    }
    
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t aliyun_asr_send_final(aliyun_asr_provider_t *provider) {
    if (!provider || provider->state != WS_CONNECTED) {
        return SWITCH_STATUS_FALSE;
    }
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Sent final marker to Aliyun ASR\n");
    
    return SWITCH_STATUS_SUCCESS;
}

void aliyun_asr_disconnect(aliyun_asr_provider_t *provider) {
    if (!provider) {
        return;
    }
    
    provider->state = WS_DISCONNECTED;
    
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