#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

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

switch_status_t config_load(mod_asr_tts_config_t *config);
const char* config_get_string(switch_xml_t xml, const char *name, const char *default_val);
int config_get_int(switch_xml_t xml, const char *name, int default_val);

#endif