#ifndef CORE_RESAMPLE_H
#define CORE_RESAMPLE_H

#include <switch.h>

typedef struct asr_resampler_t {
    switch_audio_resampler_t *resampler;
    uint32_t from_rate;
    uint32_t to_rate;
} asr_resampler_t;

asr_resampler_t* resampler_create(switch_memory_pool_t *pool, uint32_t from_rate, uint32_t to_rate);
switch_status_t resampler_process(asr_resampler_t *rs, int16_t *input, size_t input_samples, 
                                   int16_t *output, size_t *output_samples);
void resampler_destroy(asr_resampler_t **rs);

#endif