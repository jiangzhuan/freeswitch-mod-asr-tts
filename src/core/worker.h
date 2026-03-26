#ifndef CORE_WORKER_H
#define CORE_WORKER_H

#include <switch.h>

typedef struct worker_pool_t worker_pool_t;
typedef void (*task_func_t)(void *data);

typedef struct worker_task_t {
    task_func_t func;
    void *data;
} worker_task_t;

struct worker_pool_t {
    switch_thread_t **threads;
    int thread_count;
    switch_mutex_t *queue_mutex;
    switch_thread_cond_t *queue_cond;
    switch_queue_t *task_queue;
    switch_memory_pool_t *pool;
    switch_bool_t running;
};

worker_pool_t* worker_pool_create(switch_memory_pool_t *pool, int thread_count);
void worker_pool_destroy(worker_pool_t **pool);
switch_status_t worker_pool_submit(worker_pool_t *pool, task_func_t func, void *data);

#endif