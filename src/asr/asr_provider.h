#ifndef ASR_ASR_PROVIDER_H
#define ASR_ASR_PROVIDER_H

#include <switch.h>

typedef struct asr_provider_interface_t {
    const char *name;
    void* (*create)(const char *url, const char *appkey, const char *token);
    void (*destroy)(void **provider);
    switch_status_t (*connect)(void *provider);
    switch_status_t (*send_audio)(void *provider, int16_t *audio, size_t len);
    switch_status_t (*send_final)(void *provider);
    void (*disconnect)(void *provider);
    void (*set_callbacks)(void *provider, void *ctx, 
                          void (*on_result)(void*, const char*, int),
                          void (*on_error)(void*, int, const char*));
} asr_provider_interface_t;

#endif