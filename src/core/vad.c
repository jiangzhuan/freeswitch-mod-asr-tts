#include "vad.h"
#include <math.h>

static float calculate_energy_db(int16_t *audio, size_t len) {
    double sum = 0;
    size_t i;
    double rms, db;
    
    if (!audio || len == 0) {
        return -100.0;
    }
    
    for (i = 0; i < len; i++) {
        sum += (double)audio[i] * audio[i];
    }
    
    rms = sqrt(sum / len);
    
    if (rms < 1.0) {
        return -100.0;
    }
    
    db = 20 * log10(rms / 32768.0);
    
    return (float)db;
}

vad_context_t* vad_create(int threshold_db) {
    vad_context_t *vad;
    
    vad = malloc(sizeof(*vad));
    if (!vad) {
        return NULL;
    }
    
    vad->threshold_db = threshold_db;
    vad->speech_duration_ms = 0;
    vad->silence_duration_ms = 0;
    vad->min_speech_ms = 100;
    vad->min_silence_ms = 500;
    vad->state = VAD_SILENCE;
    vad->frame_size_ms = 20;
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, 
        "VAD created: threshold=%d dB, min_speech=%d ms, min_silence=%d ms\n",
        threshold_db, vad->min_speech_ms, vad->min_silence_ms);
    
    return vad;
}

void vad_destroy(vad_context_t **vad) {
    if (!vad || !*vad) {
        return;
    }
    
    free(*vad);
    *vad = NULL;
}

vad_state_t vad_process(vad_context_t *vad, int16_t *audio, size_t len) {
    float energy_db;
    vad_state_t prev_state;
    
    if (!vad || !audio) {
        return VAD_SILENCE;
    }
    
    energy_db = calculate_energy_db(audio, len);
    prev_state = vad->state;
    
    if (energy_db > vad->threshold_db) {
        vad->silence_duration_ms = 0;
        vad->speech_duration_ms += vad->frame_size_ms;
        
        if (vad->state == VAD_SILENCE && vad->speech_duration_ms >= vad->min_speech_ms) {
            vad->state = VAD_SPEECH;
        }
    } else {
        vad->speech_duration_ms = 0;
        vad->silence_duration_ms += vad->frame_size_ms;
        
        if (vad->state == VAD_SPEECH && vad->silence_duration_ms >= vad->min_silence_ms) {
            vad->state = VAD_END;
        }
    }
    
    if (vad->state == VAD_END) {
        vad->state = VAD_SILENCE;
        return VAD_END;
    }
    
    return vad->state;
}