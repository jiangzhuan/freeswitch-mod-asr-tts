#include "buffer.h"

asr_ring_buffer_t* buffer_create(switch_memory_pool_t *pool, size_t capacity) {
    asr_ring_buffer_t *rb;
    
    rb = switch_core_alloc(pool, sizeof(*rb));
    if (!rb) {
        return NULL;
    }
    
    rb->pool = pool;
    rb->capacity = capacity;
    rb->chunk_size = 320;
    rb->sequence = 0;
    
    if (switch_buffer_create(pool, &rb->buffer, capacity) != SWITCH_STATUS_SUCCESS) {
        return NULL;
    }
    
    if (switch_mutex_init(&rb->mutex, SWITCH_MUTEX_NESTED, pool) != SWITCH_STATUS_SUCCESS) {
        return NULL;
    }
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, 
        "Ring buffer created: capacity=%zu bytes\n", capacity);
    
    return rb;
}

switch_status_t buffer_write(asr_ring_buffer_t *rb, void *data, size_t len) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    
    if (!rb || !data || len == 0) {
        return SWITCH_STATUS_FALSE;
    }
    
    switch_mutex_lock(rb->mutex);
    
    if (switch_buffer_inuse(rb->buffer) + len > rb->capacity) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, 
            "Ring buffer full, dropping %zu bytes\n", len);
        status = SWITCH_STATUS_FALSE;
    } else {
        switch_buffer_write(rb->buffer, data, len);
        rb->sequence++;
    }
    
    switch_mutex_unlock(rb->mutex);
    
    return status;
}

switch_status_t buffer_read(asr_ring_buffer_t *rb, void *data, size_t *len) {
    switch_size_t read_len;
    
    if (!rb || !data || !len) {
        return SWITCH_STATUS_FALSE;
    }
    
    switch_mutex_lock(rb->mutex);
    
    read_len = switch_buffer_read(rb->buffer, data, *len);
    *len = (size_t)read_len;
    
    switch_mutex_unlock(rb->mutex);
    
    return (read_len > 0) ? SWITCH_STATUS_SUCCESS : SWITCH_STATUS_FALSE;
}

size_t buffer_available(asr_ring_buffer_t *rb) {
    size_t available;
    
    if (!rb) {
        return 0;
    }
    
    switch_mutex_lock(rb->mutex);
    available = switch_buffer_inuse(rb->buffer);
    switch_mutex_unlock(rb->mutex);
    
    return available;
}

void buffer_reset(asr_ring_buffer_t *rb) {
    if (!rb) {
        return;
    }
    
    switch_mutex_lock(rb->mutex);
    switch_buffer_zero(rb->buffer);
    rb->sequence = 0;
    switch_mutex_unlock(rb->mutex);
}

void buffer_destroy(asr_ring_buffer_t **rb) {
    if (!rb || !*rb) {
        return;
    }
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, 
        "Ring buffer destroyed: sequence=%u\n", (*rb)->sequence);
    
    *rb = NULL;
}