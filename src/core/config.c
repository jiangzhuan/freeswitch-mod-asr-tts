#include "config.h"
#include <string.h>

static const char* find_param(switch_xml_t xml, const char *name) {
    switch_xml_t param;
    
    for (param = switch_xml_child(xml, "param"); param; param = param->next) {
        const char *pname = switch_xml_attr(param, "name");
        if (pname && strcasecmp(pname, name) == 0) {
            return switch_xml_attr(param, "value");
        }
    }
    return NULL;
}

const char* config_get_string(switch_xml_t xml, const char *name, const char *default_val) {
    const char *val = find_param(xml, name);
    return val ? val : default_val;
}

int config_get_int(switch_xml_t xml, const char *name, int default_val) {
    const char *val = find_param(xml, name);
    return val ? atoi(val) : default_val;
}

switch_status_t config_load(mod_asr_tts_config_t *config) {
    switch_xml_t xml, cfg, settings;
    const char *val;
    
    memset(config, 0, sizeof(*config));
    
    config->sample_rate = 16000;
    config->vad_threshold = -40;
    config->thread_pool_size = 4;
    config->reconnect_max_interval = 30;
    
    if (switch_xml_open_cfg("mod_asr_tts.conf", &xml, NULL) != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to open config file\n");
        return SWITCH_STATUS_FALSE;
    }
    
    cfg = switch_xml_child(xml, "configuration");
    if (!cfg) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid config: missing configuration element\n");
        switch_xml_free(xml);
        return SWITCH_STATUS_FALSE;
    }
    
    settings = switch_xml_child(cfg, "settings");
    if (!settings) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid config: missing settings element\n");
        switch_xml_free(xml);
        return SWITCH_STATUS_FALSE;
    }
    
    val = config_get_string(settings, "appkey", NULL);
    if (!val || strlen(val) == 0 || strcmp(val, "YOUR_APPKEY") == 0) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing required config: appkey\n");
        switch_xml_free(xml);
        return SWITCH_STATUS_FALSE;
    }
    switch_set_string(config->appkey, val);
    
    val = config_get_string(settings, "token", NULL);
    if (!val || strlen(val) == 0 || strcmp(val, "YOUR_TOKEN") == 0) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing required config: token\n");
        switch_xml_free(xml);
        return SWITCH_STATUS_FALSE;
    }
    switch_set_string(config->token, val);
    
    val = config_get_string(settings, "websocket-url", NULL);
    if (!val || strlen(val) == 0) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing required config: websocket-url\n");
        switch_xml_free(xml);
        return SWITCH_STATUS_FALSE;
    }
    switch_set_string(config->websocket_url, val);
    
    config->sample_rate = config_get_int(settings, "sample-rate", 16000);
    config->vad_threshold = config_get_int(settings, "vad-threshold", -40);
    config->thread_pool_size = config_get_int(settings, "thread-pool-size", 4);
    config->reconnect_max_interval = config_get_int(settings, "reconnect-max-interval", 30);
    
    switch_xml_free(xml);
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, 
        "Config loaded: appkey=%s, websocket_url=%s, sample_rate=%d, thread_pool_size=%d\n",
        config->appkey, config->websocket_url, config->sample_rate, config->thread_pool_size);
    
    return SWITCH_STATUS_SUCCESS;
}