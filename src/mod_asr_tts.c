#include "mod_asr_tts.h"
#include "core/config.h"
#include "core/worker.h"
#include "core/media_bug.h"
#include "asr/asr_interface.h"

static mod_asr_tts_config_t g_config;
static worker_pool_t *g_worker_pool = NULL;

SWITCH_MODULE_LOAD_FUNCTION(mod_asr_tts_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_asr_tts_shutdown);
SWITCH_MODULE_DEFINITION(mod_asr_tts, mod_asr_tts_load, NULL, mod_asr_tts_shutdown);

static void audio_processor_callback(void *ctx, int16_t *audio, size_t samples) {
    asr_handle_t *handle = (asr_handle_t *)ctx;
    switch_asr_handle_t ah;
    switch_asr_flag_t flags = 0;
    
    if (!handle || !handle->running) {
        return;
    }
    
    ah.private_info = handle;
    asr_feed(&ah, audio, samples * sizeof(int16_t), &flags);
}

static switch_status_t asr_status_api(const char *cmd, switch_core_session_t *session, switch_stream_handle_t *stream) {
    stream->write_function(stream, "+OK ASR module loaded (version %s)\n", MOD_ASR_TTS_VERSION);
    stream->write_function(stream, "Config: appkey=%s, sample_rate=%d, thread_pool_size=%d\n", 
        g_config.appkey, g_config.sample_rate, g_config.thread_pool_size);
    return SWITCH_STATUS_SUCCESS;
}

static void start_asr_app(switch_core_session_t *session, const char *data) {
    switch_channel_t *channel = switch_core_session_get_channel(session);
    switch_asr_handle_t *ah = NULL;
    asr_handle_t *handle = NULL;
    switch_asr_flag_t flags = 0;
    const char *uuid = switch_channel_get_uuid(channel);
    
    switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, 
        "Starting ASR on channel %s\n", uuid);
    
    ah = switch_core_session_alloc(session, sizeof(*ah));
    if (!ah) {
        switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, 
            "Failed to allocate ASR handle\n");
        return;
    }
    
    if (asr_open(ah, "L16", 16000, uuid, &flags) != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, 
            "Failed to open ASR handle\n");
        return;
    }
    
    handle = (asr_handle_t *)ah->private_info;
    if (handle) {
        if (media_bug_attach(&handle->session_ctx, session, audio_processor_callback, handle) != SWITCH_STATUS_SUCCESS) {
            switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, 
                "Failed to attach media bug\n");
        }
        
        if (handle->provider && handle->provider_ctx) {
            handle->provider->connect(handle->provider_ctx);
        }
    }
    
    switch_channel_set_variable(channel, "asr_handle", (const char *)ah);
    
    event_publish_asr_start(session, uuid);
    
    switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, 
        "ASR started successfully on channel %s\n", uuid);
}

static void stop_asr_app(switch_core_session_t *session, const char *data) {
    switch_channel_t *channel = switch_core_session_get_channel(session);
    switch_asr_handle_t *ah = (switch_asr_handle_t *)switch_channel_get_variable(channel, "asr_handle");
    switch_asr_flag_t flags = 0;
    
    if (ah) {
        asr_close(ah, &flags);
        switch_channel_set_variable(channel, "asr_handle", NULL);
        switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_INFO, "ASR stopped\n");
    }
}

SWITCH_MODULE_LOAD_FUNCTION(mod_asr_tts_load) {
    switch_asr_interface_t *asr_interface;
    switch_api_interface_t *api_interface;
    switch_application_interface_t *app_interface;

    *module_interface = switch_loadable_module_create_module_interface(pool, modname);

    if (config_load(&g_config) != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to load configuration\n");
        return SWITCH_STATUS_FALSE;
    }

    g_worker_pool = worker_pool_create(pool, g_config.thread_pool_size);
    if (!g_worker_pool) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Failed to create worker pool\n");
    }

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "mod_asr_tts loaded (version %s)\n", MOD_ASR_TTS_VERSION);

    SWITCH_ADD_ASR(asr_interface, "aliyun", asr_open, asr_close, asr_feed, asr_results, asr_pause, asr_resume, NULL, NULL, NULL);
    SWITCH_ADD_API(api_interface, "asr_status", "Show ASR status", asr_status_api, "");
    SWITCH_ADD_APP(app_interface, "start_asr", "Start ASR", "Start ASR recognition", start_asr_app, "", SAF_NONE);
    SWITCH_ADD_APP(app_interface, "stop_asr", "Stop ASR", "Stop ASR recognition", stop_asr_app, "", SAF_NONE);

    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_asr_tts_shutdown) {
    worker_pool_destroy(&g_worker_pool);
    memset(&g_config, 0, sizeof(g_config));
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "mod_asr_tts unloaded\n");
    return SWITCH_STATUS_SUCCESS;
}