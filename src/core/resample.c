#include "resample.h"

asr_resampler_t* resampler_create(switch_memory_pool_t *pool, uint32_t from_rate, uint32_t to_rate) {
    asr_resampler_t *rs;
    
    rs = switch_core_alloc(pool, sizeof(*rs));
    if (!rs) {
        return NULL;
    }
    
    rs->from_rate = from_rate;
    rs->to_rate = to_rate;
    
    if (switch_resample_create(&rs->resampler, from_rate, to_rate, 320, SWITCH_RESAMPLE_QUALITY, 1) 
        != SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, 
            "Failed to create resampler: %u -> %u\n", from_rate, to_rate);
        return NULL;
    }
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, 
        "Resampler created: %u Hz -> %u Hz\n", from_rate, to_rate);
    
    return rs;
}

switch_status_t resampler_process(asr_resampler_t *rs, int16_t *input, size_t input_samples, 
                                   int16_t *output, size_t *output_samples) {
    uint32_t out_len;
    
    if (!rs || !rs->resampler || !input || !output || !output_samples) {
        return SWITCH_STATUS_FALSE;
    }
    
    out_len = switch_resample_process(rs->resampler, input, (uint32_t)input_samples);
    
    if (out_len > 0 && out_len <= *output_samples) {
        memcpy(output, rs->resampler->to, out_len * sizeof(int16_t));
        *output_samples = out_len;
    } else {
        *output_samples = 0;
    }
    
    return SWITCH_STATUS_SUCCESS;
}

void resampler_destroy(asr_resampler_t **rs) {
    if (!rs || !*rs) {
        return;
    }
    
    switch_resample_destroy(&(*rs)->resampler);
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Resampler destroyed\n");
    
    *rs = NULL;
}