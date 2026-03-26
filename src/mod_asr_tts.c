#include "mod_asr_tts.h"
#include "core/config.h"

static mod_asr_tts_config_t g_config;

SWITCH_MODULE_LOAD_FUNCTION(mod_asr_tts_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_asr_tts_shutdown);
SWITCH_MODULE_DEFINITION(mod_asr_tts, mod_asr_tts_load, NULL, mod_asr_tts_shutdown);

static switch_status_t asr_open(switch_asr_handle_t *ah, const char *codec, int rate, const char *dest, switch_asr_flag_t *flags) {
    return SWITCH_STATUS_SUCCESS;
}

static switch_status_t asr_close(switch_asr_handle_t *ah, switch_asr_flag_t *flags) {
    return SWITCH_STATUS_SUCCESS;
}

static switch_status_t asr_feed(switch_asr_handle_t *ah, void *data, unsigned int len, switch_asr_flag_t *flags) {
    return SWITCH_STATUS_SUCCESS;
}

static switch_status_t asr_results(switch_asr_handle_t *ah, switch_asr_result_t **result, switch_asr_flag_t *flags) {
    return SWITCH_STATUS_BREAK;
}

static switch_status_t asr_pause(switch_asr_handle_t *ah) {
    return SWITCH_STATUS_SUCCESS;
}

static switch_status_t asr_resume(switch_asr_handle_t *ah) {
    return SWITCH_STATUS_SUCCESS;
}

static switch_status_t asr_status_api(const char *cmd, switch_core_session_t *session, switch_stream_handle_t *stream) {
    stream->write_function(stream, "+OK ASR module loaded (version %s)\n", MOD_ASR_TTS_VERSION);
    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_asr_tts_load) {
    switch_asr_interface_t *asr_interface;
    switch_api_interface_t *api_interface;

    *module_interface = switch_loadable_module_create_module_interface(pool, modname);

    if (config_load(&g_config) != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to load configuration\n");
        return SWITCH_STATUS_FALSE;
    }

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "mod_asr_tts loaded (version %s)\n", MOD_ASR_TTS_VERSION);
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Config: appkey=%s, sample_rate=%d\n", g_config.appkey, g_config.sample_rate);

    SWITCH_ADD_ASR(asr_interface, "aliyun", asr_open, asr_close, asr_feed, asr_results, asr_pause, asr_resume, NULL, NULL, NULL);
    SWITCH_ADD_API(api_interface, "asr_status", "Show ASR status", asr_status_api, "");

    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_asr_tts_shutdown) {
    memset(&g_config, 0, sizeof(g_config));
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "mod_asr_tts unloaded\n");
    return SWITCH_STATUS_SUCCESS;
}