#include "worker.h"

static void* worker_thread_func(switch_thread_t *thread, void *data) {
    worker_pool_t *pool = (worker_pool_t *)data;
    worker_task_t *task = NULL;
    
    while (pool->running) {
        switch_mutex_lock(pool->queue_mutex);
        
        while (switch_queue_size(pool->task_queue) == 0 && pool->running) {
            switch_thread_cond_wait(pool->queue_cond, pool->queue_mutex);
        }
        
        if (!pool->running) {
            switch_mutex_unlock(pool->queue_mutex);
            break;
        }
        
        switch_queue_pop(pool->task_queue, (void**)&task);
        switch_mutex_unlock(pool->queue_mutex);
        
        if (task) {
            task->func(task->data);
            free(task);
        }
    }
    
    return NULL;
}

worker_pool_t* worker_pool_create(switch_memory_pool_t *pool, int thread_count) {
    worker_pool_t *wp;
    switch_threadattr_t *thd_attr;
    int i;
    
    wp = switch_core_alloc(pool, sizeof(*wp));
    if (!wp) {
        return NULL;
    }
    
    wp->pool = pool;
    wp->thread_count = thread_count;
    wp->running = SWITCH_TRUE;
    
    if (switch_mutex_init(&wp->queue_mutex, SWITCH_MUTEX_NESTED, pool) != SWITCH_STATUS_SUCCESS) {
        return NULL;
    }
    
    if (switch_thread_cond_create(&wp->queue_cond, pool) != SWITCH_STATUS_SUCCESS) {
        return NULL;
    }
    
    if (switch_queue_create(&wp->task_queue, 1024, pool) != SWITCH_STATUS_SUCCESS) {
        return NULL;
    }
    
    wp->threads = switch_core_alloc(pool, sizeof(switch_thread_t*) * thread_count);
    if (!wp->threads) {
        return NULL;
    }
    
    for (i = 0; i < thread_count; i++) {
        switch_threadattr_create(&thd_attr, pool);
        switch_threadattr_detach_set(thd_attr, 1);
        switch_threadattr_stacksize_set(thd_attr, SWITCH_THREAD_STACKSIZE);
        
        if (switch_thread_create(&wp->threads[i], thd_attr, worker_thread_func, wp, pool) != SWITCH_STATUS_SUCCESS) {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to create worker thread %d\n", i);
        }
    }
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Worker pool created with %d threads\n", thread_count);
    
    return wp;
}

void worker_pool_destroy(worker_pool_t **pool) {
    worker_pool_t *p;
    int i;
    
    if (!pool || !*pool) {
        return;
    }
    
    p = *pool;
    p->running = SWITCH_FALSE;
    
    switch_mutex_lock(p->queue_mutex);
    switch_thread_cond_broadcast(p->queue_cond);
    switch_mutex_unlock(p->queue_mutex);
    
    for (i = 0; i < p->thread_count; i++) {
        if (p->threads[i]) {
            switch_thread_join(&p->threads[i]);
        }
    }
    
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Worker pool destroyed\n");
    
    *pool = NULL;
}

switch_status_t worker_pool_submit(worker_pool_t *pool, task_func_t func, void *data) {
    worker_task_t *task;
    
    if (!pool || !func || !pool->running) {
        return SWITCH_STATUS_FALSE;
    }
    
    task = malloc(sizeof(*task));
    if (!task) {
        return SWITCH_STATUS_FALSE;
    }
    
    task->func = func;
    task->data = data;
    
    switch_mutex_lock(pool->queue_mutex);
    switch_queue_push(pool->task_queue, task);
    switch_thread_cond_signal(pool->queue_cond);
    switch_mutex_unlock(pool->queue_mutex);
    
    return SWITCH_STATUS_SUCCESS;
}