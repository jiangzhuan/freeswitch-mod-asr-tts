#ifndef ASR_PROVIDERS_ALIYUN_ASR_H
#define ASR_PROVIDERS_ALIYUN_ASR_H

#include <switch.h>

typedef enum {
    WS_DISCONNECTED,
    WS_CONNECTING,
    WS_CONNECTED,
    WS_RECONNECTING
} ws_state_t;

typedef struct aliyun_asr_provider_t {
    ws_state_t state;
    char websocket_url[512];
    char appkey[128];
    char token[256];
    
    int reconnect_interval;
    int max_reconnect_interval;
    time_t last_reconnect_time;
    
    int connect_timeout;
    int idle_timeout;
    int heartbeat_interval;
    
    void *callback_ctx;
    void (*on_result)(void *ctx, const char *json, int is_final);
    void (*on_error)(void *ctx, int code, const char *message);
} aliyun_asr_provider_t;

aliyun_asr_provider_t* aliyun_asr_create(const char *url, const char *appkey, const char *token);
void aliyun_asr_destroy(aliyun_asr_provider_t **provider);
switch_status_t aliyun_asr_connect(aliyun_asr_provider_t *provider);
switch_status_t aliyun_asr_send_audio(aliyun_asr_provider_t *provider, int16_t *audio, size_t len);
switch_status_t aliyun_asr_send_final(aliyun_asr_provider_t *provider);
void aliyun_asr_disconnect(aliyun_asr_provider_t *provider);
void aliyun_asr_set_callbacks(aliyun_asr_provider_t *provider, void *ctx,
                               void (*on_result)(void*, const char*, int),
                               void (*on_error)(void*, int, const char*));

#endif