#ifndef MOD_ASR_TTS_H
#define MOD_ASR_TTS_H

#define MOD_ASR_TTS_VERSION "1.0.0"

#include <switch.h>
#include "core/config.h"
#include "core/event.h"

switch_status_t asr_open(switch_asr_handle_t *ah, const char *codec, int rate, const char *dest, switch_asr_flag_t *flags);
switch_status_t asr_close(switch_asr_handle_t *ah, switch_asr_flag_t *flags);
switch_status_t asr_feed(switch_asr_handle_t *ah, void *data, unsigned int len, switch_asr_flag_t *flags);
switch_status_t asr_check_results(switch_asr_handle_t *ah, switch_asr_flag_t *flags);
switch_status_t asr_get_results(switch_asr_handle_t *ah, char **xmlstr, switch_asr_flag_t *flags);
switch_status_t asr_pause(switch_asr_handle_t *ah);
switch_status_t asr_resume(switch_asr_handle_t *ah);

#endif