#ifndef LIBFAB_SCHEDULER_H
#define LIBFAB_SCHEDULER_H

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "common.h"

typedef void (*koko_scheduler_job_func_t)(void *data);

struct koko_scheduler_job_t {
	uint8_t priority, 
        job_id;
	struct timespec start_time;
	koko_scheduler_job_func_t func;
	void * data;
};
struct koko_scheduler_worker_thread_ops_t {
	void (*constructor)(void *data);
	void (*destructor)(void *data);
	void *data;
};
struct koko_scheduler_ctx_t {
	pthread_mutex_t mutex, cond_mutex; 
	pthread_cond_t work_ready_cond;
	struct koko_scheduler_worker_thread_ops_t *ops;
	uint8_t num_worker_threads, job_count,
        queue_size, waiting_jobs;
	struct koko_scheduler_thread_t **thread;
	struct koko_scheduler_job_t **queue;
    int (*pthread_create_wrapper)(pthread_t *, 
            const pthread_attr_t *,void *(*)(void *), void *);
};
struct koko_scheduler_thread_t {
    pthread_t thread_id;
	pthread_mutex_t mutex, 
        job_mutex;
	uint8_t thread_num;
	bool keep_running;
	struct koko_scheduler_ctx_t *ctx;
	struct koko_scheduler_job_t *job; 
};

KOKO_API int koko_scheduler_get_queue_len(struct koko_scheduler_ctx_t* ctx);
KOKO_API void koko_scheduler_destroy(struct koko_scheduler_ctx_t *ctx);
KOKO_API struct koko_scheduler_ctx_t *koko_scheduler_init(unsigned int queue_size, unsigned int num_worker_threads, 
            struct koko_scheduler_worker_thread_ops_t *ops);
KOKO_API int koko_scheduler_add_work(struct koko_scheduler_ctx_t* ctx, int priority, 
        unsigned int miliseconds, koko_scheduler_job_func_t callback_fn, void *data);
KOKO_API struct koko_scheduler_ctx_t * koko_scheduler_init_pth_wapper(unsigned int queue_size, unsigned int num_worker_threads,
	 int (*pthread_create_wrapper)(pthread_t *, const pthread_attr_t *,void *(*)(void *), void *));
KOKO_API int koko_scheduler_show_status(struct koko_scheduler_ctx_t* ctx, FILE *fp);
KOKO_API int koko_scheduler_job_queued(struct koko_scheduler_ctx_t* ctx, int job_id);
KOKO_API int koko_scheduler_job_running(struct koko_scheduler_ctx_t* ctx, int job_id);
KOKO_API int koko_scheduler_job_queued_or_running(struct koko_scheduler_ctx_t* ctx, int job_id);
KOKO_API int koko_scheduler_kill(struct koko_scheduler_ctx_t* ctx, int job_id);
KOKO_API int koko_scheduler_cancel(struct koko_scheduler_ctx_t* ctx, int job_id);
KOKO_API int koko_scheduler_dequeue(struct koko_scheduler_ctx_t* ctx, int job_id);
KOKO_API int koko_scheduler_empty(struct koko_scheduler_ctx_t *ctx);int koko_scheduler_empty_wait(struct koko_scheduler_ctx_t *ctx);

#endif 
