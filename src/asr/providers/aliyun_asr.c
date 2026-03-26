#include "aliyun_asr.h"
#include <curl/curl.h>
#include <pthread.h>

static size_t ws_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    aliyun_asr_provider_t *provider = (aliyun_asr_provider_t *)userdata;
    size_t total = size * nmemb;
    const struct curl_ws_frame *meta;
    size_t rlen;
    char *buf;
    
    meta = curl_ws_meta(provider->curl);
    if (!meta) {
        return total;
    }
    
    buf = malloc(total + 1);
    if (!buf) {
        return total;
    }
    
    memcpy(buf, ptr, total);
    buf[total] = '\0';
    
    if (provider->on_result && meta->flags & CURLWS_TEXT) {
        int is_final = (strstr(buf, "\"is_final\":true") || strstr(buf, "\"is_final\": true")) ? 1 : 0;
        provider->on_result(provider->callback_ctx, buf, is_final);
    }
    
    free(buf);
    return total;
}

static void* recv_thread_func(void *data) {
    aliyun_asr_provider_t *provider = (aliyun_asr_provider_t *)data;
    char buf[4096];
    size_t rlen;
    const struct curl_ws_frame *meta;
    
    while (provider->running && provider->state == WS_CONNECTED) {
        CURLcode res = curl_ws_recv(provider->curl, buf, sizeof(buf), &rlen, &meta);
        if (res == CURLE_OK && rlen > 0 && meta) {
            buf[rlen] = '\0';
            if (provider->on_result && (meta->flags & CURLWS_TEXT)) {
                int is_final = (strstr(buf, "\"is_final\":true") || strstr(buf, "\"is_final\": true")) ? 1 : 0;
                provider->on_result(provider->callback_ctx, buf, is_final);
            }
        }
        switch_yield(10000);
    }
    
    return NULL;
}

static void send_heartbeat(aliyun_asr_provider_t *provider) {
    size_t sent;
    if (provider->curl && provider->state == WS_CONNECTED) {
        curl_ws_send(provider->curl, NULL, 0, &sent, 0, CURLWS_PING);
        provider->last_heartbeat = time(NULL);
    }
}

aliyun_asr_provider_t* aliyun_asr_create(const char *url, const char *appkey, const char *token) {
    aliyun_asr_provider_t *provider;
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    provider = malloc(sizeof(*provider));
    if (!provider) {
        return NULL;
    }
    
    memset(provider, 0, sizeof(*provider));
    
    provider->state = WS_DISCONNECTED;
    provider->running = SWITCH_FALSE;
    
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
    char url[1024];
    struct curl_slist *headers = NULL;
    CURLcode res;
    
    if (!provider) {
        return SWITCH_STATUS_FALSE;
    }
    
    provider->state = WS_CONNECTING;
    
    provider->curl = curl_easy_init();
    if (!provider->curl) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to init curl\n");
        provider->state = WS_DISCONNECTED;
        return SWITCH_STATUS_FALSE;
    }
    
    snprintf(url, sizeof(url), "%s?appkey=%s&token=%s", 
        provider->websocket_url, provider->appkey, provider->token);
    
    curl_easy_setopt(provider->curl, CURLOPT_URL, url);
    curl_easy_setopt(provider->curl, CURLOPT_CONNECT_ONLY, 2L);
    curl_easy_setopt(provider->curl, CURLOPT_TIMEOUT, provider->connect_timeout);
    
    headers = curl_slist_append(headers, "Upgrade: websocket");
    headers = curl_slist_append(headers, "Connection: Upgrade");
    headers = curl_slist_append(headers, "Sec-WebSocket-Version: 13");
    headers = curl_slist_append(headers, "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==");
    curl_easy_setopt(provider->curl, CURLOPT_HTTPHEADER, headers);
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, 
        "Aliyun ASR connecting to %s\n", url);
    
    res = curl_easy_perform(provider->curl);
    
    if (res != CURLE_OK) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, 
            "WebSocket connect failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(provider->curl);
        provider->curl = NULL;
        provider->state = WS_RECONNECTING;
        return SWITCH_STATUS_FALSE;
    }
    
    provider->state = WS_CONNECTED;
    provider->reconnect_interval = 1;
    provider->running = SWITCH_TRUE;
    provider->last_heartbeat = time(NULL);
    
    curl_slist_free_all(headers);
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Aliyun ASR connected via WebSocket\n");
    
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t aliyun_asr_send_audio(aliyun_asr_provider_t *provider, int16_t *audio, size_t len) {
    size_t sent;
    CURLcode res;
    
    if (!provider || provider->state != WS_CONNECTED || !provider->curl) {
        return SWITCH_STATUS_FALSE;
    }
    
    res = curl_ws_send(provider->curl, audio, len * sizeof(int16_t), &sent, 0, CURLWS_BINARY);
    if (res != CURLE_OK) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, 
            "WebSocket send failed: %s\n", curl_easy_strerror(res));
        return SWITCH_STATUS_FALSE;
    }
    
    if (time(NULL) - provider->last_heartbeat >= provider->heartbeat_interval) {
        send_heartbeat(provider);
    }
    
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t aliyun_asr_send_final(aliyun_asr_provider_t *provider) {
    const char *final_msg = "{\"type\":\"stop\"}";
    size_t sent;
    CURLcode res;
    
    if (!provider || provider->state != WS_CONNECTED || !provider->curl) {
        return SWITCH_STATUS_FALSE;
    }
    
    res = curl_ws_send(provider->curl, final_msg, strlen(final_msg), &sent, 0, CURLWS_TEXT);
    if (res != CURLE_OK) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, 
            "WebSocket send final failed: %s\n", curl_easy_strerror(res));
        return SWITCH_STATUS_FALSE;
    }
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Sent final marker to Aliyun ASR\n");
    
    return SWITCH_STATUS_SUCCESS;
}

void aliyun_asr_disconnect(aliyun_asr_provider_t *provider) {
    if (!provider) {
        return;
    }
    
    provider->running = SWITCH_FALSE;
    
    if (provider->recv_thread) {
        switch_thread_join(&provider->recv_thread);
        provider->recv_thread = NULL;
    }
    
    if (provider->curl) {
        size_t sent;
        curl_ws_send(provider->curl, NULL, 0, &sent, 0, CURLWS_CLOSE);
        curl_easy_cleanup(provider->curl);
        provider->curl = NULL;
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