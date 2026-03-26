#ifndef CORE_BUFFER_H
#define CORE_BUFFER_H

#include <switch.h>

typedef struct asr_ring_buffer_t {
    switch_buffer_t *buffer;
    switch_mutex_t *mutex;
    switch_memory_pool_t *pool;
    size_t capacity;
    size_t chunk_size;
    uint32_t sequence;
} asr_ring_buffer_t;

asr_ring_buffer_t* buffer_create(switch_memory_pool_t *pool, size_t capacity);
switch_status_t buffer_write(asr_ring_buffer_t *rb, void *data, size_t len);
switch_status_t buffer_read(asr_ring_buffer_t *rb, void *data, size_t *len);
void buffer_destroy(asr_ring_buffer_t **rb);
size_t buffer_available(asr_ring_buffer_t *rb);
void buffer_reset(asr_ring_buffer_t *rb);

#endif