#ifndef CORE_VAD_H
#define CORE_VAD_H

#include <switch.h>

typedef enum {
    VAD_SILENCE,
    VAD_SPEECH,
    VAD_END
} vad_state_t;

typedef struct vad_context_t {
    int threshold_db;
    int speech_duration_ms;
    int silence_duration_ms;
    int min_speech_ms;
    int min_silence_ms;
    vad_state_t state;
    int frame_size_ms;
} vad_context_t;

vad_context_t* vad_create(int threshold_db);
void vad_destroy(vad_context_t **vad);
vad_state_t vad_process(vad_context_t *vad, int16_t *audio, size_t len);

#endif