#include "event.h"

switch_status_t event_publish_asr_result(switch_core_session_t *session,
                                          const char *channel_uuid,
                                          const char *json_result,
                                          int is_final,
                                          uint32_t sequence) {
    switch_event_t *event;
    
    if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, "asr") == SWITCH_STATUS_SUCCESS) {
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Channel", channel_uuid);
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "ASR-Response", json_result);
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "ASR-Is-Final", is_final ? "true" : "false");
        switch_event_add_header(event, SWITCH_STACK_BOTTOM, "Event-Sequence", "%u", sequence);
        
        switch_event_fire(&event);
        
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, 
            "ASR event published: channel=%s, is_final=%s, seq=%u\n",
            channel_uuid, is_final ? "true" : "false", sequence);
        
        return SWITCH_STATUS_SUCCESS;
    }
    
    return SWITCH_STATUS_FALSE;
}

switch_status_t event_publish_asr_error(switch_core_session_t *session,
                                         const char *channel_uuid,
                                         int error_code,
                                         const char *error_message) {
    switch_event_t *event;
    
    if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, "asr") == SWITCH_STATUS_SUCCESS) {
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Channel", channel_uuid);
        switch_event_add_header(event, SWITCH_STACK_BOTTOM, "ASR-Error-Code", "%d", error_code);
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "ASR-Error-Message", error_message);
        
        switch_event_fire(&event);
        
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, 
            "ASR error event published: channel=%s, code=%d, msg=%s\n",
            channel_uuid, error_code, error_message);
        
        return SWITCH_STATUS_SUCCESS;
    }
    
    return SWITCH_STATUS_FALSE;
}

switch_status_t event_publish_asr_start(switch_core_session_t *session,
                                         const char *channel_uuid) {
    switch_event_t *event;
    
    if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, "asr") == SWITCH_STATUS_SUCCESS) {
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Channel", channel_uuid);
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "ASR-Event", "start");
        
        switch_event_fire(&event);
        
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, 
            "ASR start event published: channel=%s\n", channel_uuid);
        
        return SWITCH_STATUS_SUCCESS;
    }
    
    return SWITCH_STATUS_FALSE;
}

switch_status_t event_publish_asr_stop(switch_core_session_t *session,
                                        const char *channel_uuid) {
    switch_event_t *event;
    
    if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, "asr") == SWITCH_STATUS_SUCCESS) {
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Channel", channel_uuid);
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "ASR-Event", "stop");
        
        switch_event_fire(&event);
        
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, 
            "ASR stop event published: channel=%s\n", channel_uuid);
        
        return SWITCH_STATUS_SUCCESS;
    }
    
    return SWITCH_STATUS_FALSE;
}