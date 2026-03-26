#ifndef CORE_EVENT_H
#define CORE_EVENT_H

#include <switch.h>

typedef enum {
    ASR_EVENT_RESULT,
    ASR_EVENT_ERROR,
    ASR_EVENT_START,
    ASR_EVENT_STOP
} asr_event_type_t;

typedef struct asr_result_t {
    char *text;
    float confidence;
    int is_final;
    int begin_time;
    int end_time;
} asr_result_t;

switch_status_t event_publish_asr_result(switch_core_session_t *session,
                                          const char *channel_uuid,
                                          const char *json_result,
                                          int is_final,
                                          uint32_t sequence);

switch_status_t event_publish_asr_error(switch_core_session_t *session,
                                         const char *channel_uuid,
                                         int error_code,
                                         const char *error_message);

switch_status_t event_publish_asr_start(switch_core_session_t *session,
                                         const char *channel_uuid);

switch_status_t event_publish_asr_stop(switch_core_session_t *session,
                                        const char *channel_uuid);

#endif