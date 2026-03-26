#ifndef MOD_ASR_TTS_H
#define MOD_ASR_TTS_H

#define MOD_ASR_TTS_VERSION "1.0.0"

#include <switch.h>

typedef struct mod_asr_tts_config_t {
    char appkey[128];
    char token[256];
    char websocket_url[256];
    int sample_rate;
    int vad_threshold;
    int thread_pool_size;
    int reconnect_max_interval;
} mod_asr_tts_config_t;

switch_status_t asr_open(switch_asr_handle_t *ah, const char *codec, int rate, const char *dest, switch_asr_flag_t *flags);
switch_status_t asr_close(switch_asr_handle_t *ah, switch_asr_flag_t *flags);
switch_status_t asr_feed(switch_asr_handle_t *ah, void *data, unsigned int len, switch_asr_flag_t *flags);
switch_status_t asr_results(switch_asr_handle_t *ah, switch_asr_result_t **result, switch_asr_flag_t *flags);
switch_status_t asr_pause(switch_asr_handle_t *ah);
switch_status_t asr_resume(switch_asr_handle_t *ah);

#endif