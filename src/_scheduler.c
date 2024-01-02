#include "_scheduler.h"

static int 
_time_gt (struct timespec *x, struct timespec *y) {
	return (x->tv_sec > y->tv_sec || 
        (x->tv_sec == y->tv_sec && x->tv_nsec > y->tv_nsec)) ? 1 : 0;
}
static int 
job_compare(const void *ptr1, const void *ptr2) {
  	struct koko_scheduler_job_t **j1, **j2;
	struct koko_scheduler_job_t *job1, *job2;

	j1 = (struct koko_scheduler_job_t **) ptr1;
	j2 = (struct koko_scheduler_job_t **) ptr2;
	job1 = *j1;
	job2 = *j2;

	if ( job1 == NULL && job2 == NULL)
		return 0;
	else if ( job1 == NULL)
	  	return 1;
	else if (job2 == NULL)
	  	return -1;

	if (job1->priority < job2->priority)
		return -1;
	else if (job1->priority > job2->priority)
		return 1;

	if (job1->start_time.tv_sec < job2->start_time.tv_sec)
	  	return -1;
	else if (job1->start_time.tv_sec > job2->start_time.tv_sec)
	  	return 1;

	if (job1->start_time.tv_nsec < job2->start_time.tv_nsec)
	  	return -1;
	else if (job1->start_time.tv_nsec > job2->start_time.tv_nsec)
	  	return 1;

	if (job1->job_id < job2->job_id)
	  	return -1;
	else if (job1->job_id > job2->job_id)
	  	return 1;

	return 0;
}
static long long 
time_diff_ms(struct timespec *x, struct timespec *y) {
	long long diff;
	diff = (x->tv_sec - y->tv_sec)*1000;
	diff += x->tv_sec - y->tv_sec;
	return diff;
}
static struct koko_scheduler_job_t*  
_koko_scheduler_get_job(struct koko_scheduler_thread_t *thread, struct timespec *wait_time, long long *wait_ms) {
	int i;
	struct koko_scheduler_job_t *job = NULL;
	struct koko_scheduler_ctx_t *ctx = thread->ctx;
	struct timespec now;
	if (!thread->keep_running)
		return NULL;
	clock_gettime(CLOCK_REALTIME, &now);
	*wait_time = now;
	wait_time->tv_sec += 9999;
	*wait_ms = 5000;
	assert(ctx);
	pthread_mutex_lock(&ctx->mutex);
	assert(ctx->queue);
	for(i = 0; thread->keep_running && i < ctx->queue_size; i++) {
		if (ctx->queue[i]) {
			if (_time_gt(&now, &ctx->queue[i]->start_time))  {
				job = ctx->queue[i];
				ctx->queue[i] = NULL;
				ctx->waiting_jobs--;
				break;
			} else if (_time_gt(wait_time, &ctx->queue[i]->start_time)) {
				*wait_time = ctx->queue[i]->start_time;
				*wait_ms = time_diff_ms(&ctx->queue[i]->start_time, &now);
			} else {
			}
		}
	}
	pthread_mutex_unlock(&ctx->mutex);
	return job;
}

int 
koko_scheduler_get_queue_len(struct koko_scheduler_ctx_t* ctx) {
	int ret;
	pthread_mutex_lock(&ctx->mutex);
	ret = ctx->waiting_jobs;
	pthread_mutex_unlock(&ctx->mutex);
	return ret;
}
static void * 
_koko_scheduler_job_scheduler(void *data){
  	struct koko_scheduler_thread_t *thread = (struct koko_scheduler_thread_t *) data;
	struct koko_scheduler_ctx_t *ctx;
	struct timespec wait_time;
	int ret;
	long long wait_ms = 1000;
	assert(thread);
	ctx = thread->ctx;
	if (ctx->ops && ctx->ops->constructor) {
		ctx->ops->constructor(ctx->ops->data);
	}
	pthread_mutex_lock(&thread->mutex);
	while (thread->keep_running) {
		thread->job = _koko_scheduler_get_job(thread, &wait_time, &wait_ms);
		if (thread->job) {
			pthread_mutex_lock(&thread->job_mutex);
			pthread_mutex_unlock(&thread->mutex);
			thread->job->func(thread->job->data);
			free(thread->job);
			thread->job = NULL;
			pthread_mutex_unlock(&thread->job_mutex);
			pthread_mutex_lock(&thread->mutex);
		} else {
			pthread_mutex_unlock(&thread->mutex);
			pthread_mutex_lock(&ctx->cond_mutex);
			ret = pthread_cond_timedwait(&ctx->work_ready_cond, &ctx->cond_mutex, &wait_time);
			pthread_mutex_unlock(&ctx->cond_mutex);
			pthread_mutex_lock(&thread->mutex);
			if (!thread->keep_running) {
				break;
			}
			if (ret == ETIMEDOUT) {
			  	continue;
			} else if (ret == EINVAL) {
				usleep(wait_ms);
			} else if (ret) { }
		}
	}
	pthread_mutex_unlock(&thread->mutex);
	if (ctx->ops && ctx->ops->destructor) {
		ctx->ops->destructor(ctx->ops->data);
	}
	return NULL;
}
static int 
_empty_queue(struct koko_scheduler_ctx_t *ctx) {
	int count = 0;
	int i;
	if(ctx->queue) {
		for (i = 0; i < ctx->queue_size; i++) {
			if (ctx->queue[i]) {
				free(ctx->queue[i]);
				ctx->queue[i] = NULL;
				count++;
			}
		}
	}
	return count;
}
void 
koko_scheduler_destroy(struct koko_scheduler_ctx_t *ctx) {
	int i;
	pthread_mutex_lock(&ctx->mutex);
	if (ctx->thread) {
		for (i = 0; i < ctx->num_worker_threads; i++) {
			if (ctx->thread[i]) {
				ctx->thread[i]->keep_running = false;
			}
		}
		pthread_cond_broadcast(&ctx->work_ready_cond);
		pthread_mutex_unlock(&ctx->mutex);
		for (i = 0; i < ctx->num_worker_threads; i++) {
			if (ctx->thread[i]) {
				pthread_join(ctx->thread[i]->thread_id, NULL);
			}
		}
		pthread_mutex_lock(&ctx->mutex);
		for (i = 0; i < ctx->num_worker_threads; i++) {
			if (ctx->thread[i]) {
				free(ctx->thread[i]);
				ctx->thread[i] = NULL;
			}
		}
		free(ctx->thread);
	}
	_empty_queue(ctx);
	free(ctx->queue);
	ctx->queue = NULL;
	pthread_mutex_unlock(&ctx->mutex);
	pthread_mutex_destroy(&ctx->mutex);
	pthread_mutex_destroy(&ctx->cond_mutex);
	free(ctx);
}
static struct koko_scheduler_ctx_t *
__koko_scheduler_init(struct koko_scheduler_ctx_t *ctx, unsigned int queue_size, unsigned int num_worker_threads) {
	unsigned int i;
	struct koko_scheduler_thread_t *thread;
	int ret = 0;
	if (!ctx)
		return NULL;
	ctx->queue_size = queue_size;
	ret = pthread_mutex_init(&ctx->mutex, NULL);
	if (ret);
	ret = pthread_mutex_init(&ctx->cond_mutex, NULL);
	if (ret);
	ctx->queue = (struct koko_scheduler_job_t **) 
        calloc( queue_size + 1, sizeof(struct koko_scheduler_job_t *));
	if (!ctx->queue) {
		goto free_ctx;
	}
	ctx->thread = (struct koko_scheduler_thread_t **) 
        calloc( num_worker_threads + 1, sizeof(struct koko_scheduler_thread_t *));
	if (!ctx->thread)
		goto free_queue;
	ret = pthread_cond_init(&ctx->work_ready_cond, NULL);
	if (ret);
	ctx->num_worker_threads = num_worker_threads;
	for (i = 0; i < num_worker_threads; i++) {
		ctx->thread[i] = thread = (struct koko_scheduler_thread_t *) 
            calloc ( 1, sizeof(struct koko_scheduler_thread_t));
		if (!ctx->thread[i]) {
			goto free_threads;
		}
		thread->thread_num = i;
		thread->ctx = ctx;
		thread->keep_running = true;
		pthread_mutex_init(&thread->mutex, NULL);
		if (ret);
		ret = pthread_create(&thread->thread_id, NULL, _koko_scheduler_job_scheduler, thread);
		if (ret);
	}
	return ctx;
	free_threads:
	for (i = 0; i < num_worker_threads; i++) {
		if (ctx->thread[i]) {
			free(ctx->thread[i]);
		}
	}
	free_queue:
	free(ctx->queue);
	free_ctx:
	free(ctx);
	fprintf(stderr, "Error initializing\n");
	return NULL;
}
struct koko_scheduler_ctx_t *
koko_scheduler_init(unsigned int queue_size, unsigned int num_worker_threads, 
    struct koko_scheduler_worker_thread_ops_t *ops) {
	struct koko_scheduler_ctx_t *ctx;
	if (!queue_size || !num_worker_threads)
	  	return NULL;
	ctx = (struct koko_scheduler_ctx_t *) calloc(1, sizeof (struct koko_scheduler_ctx_t));
	ctx->ops = ops;
	return __koko_scheduler_init(ctx, queue_size, num_worker_threads);
}
int 
koko_scheduler_add_work(struct koko_scheduler_ctx_t* ctx, int priority, 
    unsigned int miliseconds, koko_scheduler_job_func_t callback_fn, void *data) {
	int i;
	int ret;
	struct koko_scheduler_job_t *job = NULL;
	struct timespec sched_time;
	clock_gettime(CLOCK_REALTIME, &sched_time);
	pthread_mutex_lock(&ctx->mutex);
	for (i = 0; i < ctx->queue_size; i++) {
		if (ctx->queue[i])
			continue;
		job = (struct koko_scheduler_job_t *) calloc(1, sizeof(struct koko_scheduler_job_t));
		if (!job) {
			pthread_mutex_unlock(&ctx->mutex);
			return -ENOMEM;
		}
		if (miliseconds) {
			clock_gettime(CLOCK_REALTIME, &job->start_time);
			job->start_time.tv_sec += miliseconds / 1000;
			job->start_time.tv_nsec += (miliseconds % 1000) * 1000000;
			if (job->start_time.tv_nsec > 1000000000) {
				job->start_time.tv_nsec -= 1000000000;
				job->start_time.tv_sec += 1;
			}
		}
		job->priority = priority;
		job->data = data;
		job->func = callback_fn;
		ret = job->job_id = ctx->job_count++;
		if (ctx->job_count < 0)
			ctx->job_count = 0;
		ctx->queue[i] = job;
		ctx->waiting_jobs++;
		qsort(ctx->queue, ctx->queue_size,
		      sizeof(struct koko_scheduler_job *),
		      job_compare);
		if (pthread_cond_signal(&ctx->work_ready_cond)) {

		}
		pthread_mutex_unlock(&ctx->mutex);
		return ret;
	}
	pthread_mutex_unlock(&ctx->mutex);
	return -EBUSY;
}
struct koko_scheduler_ctx_t * 
koko_scheduler_init_pth_wapper(unsigned int queue_size, unsigned int num_worker_threads,
	 int (*pthread_create_wrapper)(pthread_t *, const pthread_attr_t *,void *(*)(void *), void *)) {
	struct koko_scheduler_ctx_t *ctx;
	if (!queue_size || !num_worker_threads)
		return NULL;
	ctx = (struct koko_scheduler_ctx_t *) calloc(1, sizeof (struct koko_scheduler_ctx_t));
	ctx->pthread_create_wrapper = pthread_create_wrapper;
	return __koko_scheduler_init(ctx, queue_size, num_worker_threads);
}
int 
koko_scheduler_show_status(struct koko_scheduler_ctx_t* ctx, FILE *fp) {
	int i;
	long long time_ms;
	struct timespec now_time;
	clock_gettime(CLOCK_REALTIME, &now_time);
	pthread_mutex_lock(&ctx->mutex);
	for (i = 0; i < ctx->queue_size; i++) {
		if (!ctx->queue[i])
			continue; 
		if ((ctx->queue[i]->start_time.tv_sec) ||
			(ctx->queue[i]->start_time.tv_nsec / 1000000)) {
			time_ms = time_diff_ms(&ctx->queue[i]->start_time, &now_time);
		} else {
			time_ms = 0;
		}
		fprintf(fp,"%3d | %8d | %4d | %6lld ms\n", i,
			ctx->queue[i]->job_id,
			ctx->queue[i]->priority, time_ms);
	}
	pthread_mutex_unlock(&ctx->mutex);
	fflush(fp);
	return 0;
}
static int 
_is_job_queued(struct koko_scheduler_ctx_t* ctx, int job_id) {
	int i;
	if(ctx->queue) {
		for (i = 0; i < ctx->queue_size; i++) {
			if (ctx->queue[i] && ctx->queue[i]->job_id == job_id)
				return 1;
		}
	}
	return 0;
}
static int 
_is_job_running(struct koko_scheduler_ctx_t* ctx, int job_id) {
	int i;
	int ret = 0;
	int rc;
	for (i = 0; i < ctx->num_worker_threads && !ret; i++) {
		if (ctx->thread[i]) {
			rc = pthread_mutex_trylock(&ctx->thread[i]->mutex);
			if (rc == EBUSY)
				return -EBUSY;
			if (ctx->thread[i]->job && ctx->thread[i]->job->job_id == job_id) {
				ret = 1;
			}
			pthread_mutex_unlock(&ctx->thread[i]->mutex);
		}
	}
	return ret;
}
int 
koko_scheduler_job_queued(struct koko_scheduler_ctx_t* ctx, int job_id) {
	int ret;
	if (!ctx)
		return -1;
	pthread_mutex_lock(&ctx->mutex);
	ret = _is_job_queued(ctx, job_id);
	pthread_mutex_unlock(&ctx->mutex);
	return ret;
}
int 
koko_scheduler_job_running(struct koko_scheduler_ctx_t* ctx, int job_id) {
	int ret = 0;
	if (!ctx)
		return -1;
	do {
		pthread_mutex_lock(&ctx->mutex);
		ret = _is_job_running(ctx, job_id);
		pthread_mutex_unlock(&ctx->mutex);
	} while (ret == -EBUSY);
	return ret;
}
int 
koko_scheduler_job_queued_or_running(struct koko_scheduler_ctx_t* ctx, int job_id) {
	int ret;
	if (!ctx)
		return -1;
	pthread_mutex_lock(&ctx->mutex);
	ret = _is_job_queued(ctx, job_id);
	pthread_mutex_unlock(&ctx->mutex);
	if (!ret) {
		do {
			pthread_mutex_lock(&ctx->mutex);
			ret = _is_job_running(ctx, job_id);

			pthread_mutex_unlock(&ctx->mutex);
		} while (ret == -EBUSY);
	}
	return ret;
}
static int 
_dequeue(struct koko_scheduler_ctx_t* ctx, int job_id) {
	int i;
	if(ctx->queue) {
		for (i = 0; i < ctx->queue_size; i++) {
			if (ctx->queue[i] && ctx->queue[i]->job_id == job_id) {
				free(ctx->queue[i]);
				ctx->queue[i] = NULL;
				ctx->waiting_jobs--;
				return 0;
			}
		}
	}
	return -ENOENT;
}
static int _kill_job(struct koko_scheduler_ctx_t* ctx, int job_id) {
	int i;
	int ret = -ENOENT;
	for (i = 0; i < ctx->num_worker_threads; i++) {
		if (ctx->thread[i]) {
			pthread_mutex_lock(&ctx->thread[i]->mutex);
			if (ctx->thread[i]->job && ctx->thread[i]->job->job_id == job_id) {
				if ( (ret = pthread_kill(ctx->thread[i]->thread_id , SIGTERM)) );
				pthread_create(&ctx->thread[i]->thread_id, NULL,
					_koko_scheduler_job_scheduler, ctx->thread[i]);
				pthread_mutex_unlock(&ctx->thread[i]->mutex);
				return 0;
			}
			pthread_mutex_unlock(&ctx->thread[i]->mutex);
		}
	}
	return -ENOENT;
}
int 
koko_scheduler_kill(struct koko_scheduler_ctx_t* ctx, int job_id) {
	int ret;
	pthread_mutex_lock(&ctx->mutex);
	ret = _kill_job(ctx, job_id);
	pthread_mutex_unlock(&ctx->mutex);
	return ret;
}
int 
koko_scheduler_cancel(struct koko_scheduler_ctx_t* ctx, int job_id) {
	int ret;
	pthread_mutex_lock(&ctx->mutex);
	ret = _dequeue(ctx, job_id);
	if (ret == -ENOENT)  {
		ret = _kill_job(ctx, job_id);
	}
	pthread_mutex_unlock(&ctx->mutex);
	return ret;
}
int 
koko_scheduler_dequeue(struct koko_scheduler_ctx_t* ctx, int job_id) {
	int ret;
	pthread_mutex_lock(&ctx->mutex);
	ret = _dequeue(ctx, job_id);
	pthread_mutex_unlock(&ctx->mutex);
	return ret;
}
int 
koko_scheduler_empty(struct koko_scheduler_ctx_t *ctx) {
	int count;
	pthread_mutex_lock(&ctx->mutex);
	count = _empty_queue(ctx);
	pthread_mutex_unlock(&ctx->mutex);
	return count;
}
int koko_scheduler_empty_wait(struct koko_scheduler_ctx_t *ctx) {
	int count;
	int i;
	int num_workers = ctx->num_worker_threads;
	pthread_mutex_lock(&ctx->mutex);
	count = _empty_queue(ctx);
	num_workers = ctx->num_worker_threads;
	pthread_mutex_unlock(&ctx->mutex);
	for (i = 0; i < num_workers; i++) {
		if (ctx->thread[i]) {
			pthread_mutex_lock(&ctx->thread[i]->mutex);
			pthread_mutex_lock(&ctx->thread[i]->job_mutex);
			if (ctx->thread[i]->job) {
			}
			pthread_mutex_unlock(&ctx->thread[i]->job_mutex);
			pthread_mutex_unlock(&ctx->thread[i]->mutex);
		}
	}
	return count;
}