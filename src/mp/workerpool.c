#if 1
#include <stdio.h>
#include <errno.h>
#include <string.h>
#endif

#include "workerpool.h"
#include "thread.h"
#include "cond.h"
#include "queue.h"

#include <stdlib.h>

#define KEY_UNASSIGNED	((const void*)(uintptr_t)0)
#define KEY_DEACTIVATED	((const void*)~(uintptr_t)0)

typedef enum worker_action_e {
	OP_INACTIVE		= 0x00000000,
	OP_SHUTDOWN		= 0xffffffff,
	OP_ACTIVATE		= 0x00000001,
	OP_DEACTIVATE		= 0x00000002,
	OP_WORK			= 0x00000003,
} worker_action;

typedef struct worker_s {
	atomic_ptr_t		key;
	atomic_ptr_t		activating_for_key;
	void*			worker_specific;

	thread_t		thread;
	struct worker_s*	next;

	mutex_t			lock;
	void*			action;
	worker_action		opcode;
	void*			oparg;

	cond_t			may_process_action;
	cond_t			may_invoke_action;
} worker_t;

typedef struct wp_i_s {
	mutex_t			lock;
	size_t			nworkers;
	size_t			max_queued_jobs;

	thread_t		distributor;
	queue_t			jobqueue;

	int32_t			shutdown;
	int32_t			is_shuttingdown;
	cond_t			may_fail;

	size_t			try_count;
	int32_t			retry_gracetime;

	struct worker_s*	first_worker;
	struct worker_s*	used_worker;

	worker_activate_fn	on_activate;
	worker_deactivate_fn	on_deactivate;
	worker_invoke_fn	on_invoke;

	worker_done_fn		on_job_done;
	worker_fail_fn		on_job_fail;
} wp_i_t;

typedef struct worker_args_s {
	wp_i_t*		pool;
	worker_t*	self;
} worker_args_t;

typedef struct worker_activate_args_s {
	void*		key;
	void*		user_arg;
} worker_activate_args_t;

static void worker_invoke_action(worker_t* w, worker_action opcode, void* arg)
{
	if (opcode == OP_INACTIVE)
		return;

	mutex_lock(&w->lock);
	while (w->opcode != OP_INACTIVE) {
		cond_wait(&w->may_invoke_action, &w->lock);
	}
	w->opcode = opcode;
	w->oparg = arg;
	cond_signal(&w->may_process_action);
	mutex_unlock(&w->lock);
}

static worker_action worker_get_action(worker_t* w, void** arg)
{
	worker_action opcode;

	mutex_lock(&w->lock);
	while (w->opcode == OP_INACTIVE) {
		cond_wait(&w->may_process_action, &w->lock);
	}
	opcode = w->opcode;
	w->opcode = OP_INACTIVE;
	if (arg)
		*arg = w->oparg;
	cond_broadcast(&w->may_invoke_action);
	mutex_unlock(&w->lock);
	return opcode;
}

wp_error wp_post_work(wp_t* pool, void* job)
{
	wp_error e = WP_ESUCCESS;
	wp_i_t* p = (wp_i_t*)*pool;
	int fail = 0;

	mutex_lock(&p->lock);
	if (p->shutdown) {
		mutex_unlock(&p->lock);
		return WP_ESHUTDOWN;
	}
	while (p->is_shuttingdown) {
		cond_wait(&p->may_fail, &p->lock);
		fail = 1;
	}
	mutex_unlock(&p->lock);
	if (fail)
		return WP_ESHUTDOWN;

	queue_write(&p->jobqueue, job);
	return e;
}

wp_error wp_activate_worker(wp_t* pool, const void* key, void* user_arg)
{
	wp_i_t* p = (wp_i_t*)*pool;
	worker_t* w = 0;
	worker_activate_args_t* a;

	a = (worker_activate_args_t*)calloc(1, sizeof(worker_activate_args_t));
	if (!a)
		return WP_ENOWORKER;

	a->key = (void*)key;
	a->user_arg = user_arg;

	mutex_lock(&p->lock);

	w = p->first_worker;
	while (w) {
		/* outstanding activations for specific key in progress */
		if (w->activating_for_key == key) {
			free(a);
			mutex_unlock(&p->lock);
			return WP_ESUCCESS;
		}

		/* worker is already activated */
		if (w->key == key) {
			free(a);
			mutex_unlock(&p->lock);
			return WP_ESUCCESS;
		}
		w = w->next;
	}

	w = p->first_worker;
	while (w) {
		if (w->key == KEY_UNASSIGNED) {
			w->key = KEY_DEACTIVATED;
			w->activating_for_key = key;
			worker_invoke_action(w, OP_ACTIVATE, (void*)a);
			mutex_unlock(&p->lock);
			return WP_ESUCCESS;
		}
		w = w->next;
	}
	free(a);
	mutex_unlock(&p->lock);
	return WP_ENOWORKER;
}

void wp_deactivate_worker(wp_t* pool, const void* key)
{
	wp_i_t* p = (wp_i_t*)*pool;
	worker_t* w = 0;

	mutex_lock(&p->lock);
	w = p->first_worker;
	while (w) {
		if (w->key == key) {
			w->key = KEY_DEACTIVATED;
			worker_invoke_action(w, OP_DEACTIVATE, 0);
			mutex_unlock(&p->lock);
			return;
		}
		w = w->next;
	}
	mutex_unlock(&p->lock);
}

static wp_error _wp_try_invoke_work(wp_i_t* p, void* work)
{
	worker_t* w = 0;
	size_t tries = 0;

	mutex_lock(&p->lock);
	w = p->used_worker;
	tries = p->nworkers;
	while (w && tries--) {
		if ((w->key != KEY_DEACTIVATED)
		&& (w->key != KEY_UNASSIGNED)) {
			if (w->next)
				p->used_worker = w->next;
			else
				p->used_worker = p->first_worker;
			worker_invoke_action(w, OP_WORK, work);
			mutex_unlock(&p->lock);
			return WP_ESUCCESS;
		}
		w = w->next;
		if (!w)
			w = p->first_worker;
	}
	mutex_unlock(&p->lock);
	return WP_ENOWORKER;
}

static void distribute_jobs(void* arg)
{
	wp_i_t* p = (wp_i_t*)arg;
	void* job;
	size_t i = 0;
	int32_t s = 0;
	int is_shutting_down = 0;

	for (;;) {
		s = 0;
		i = 0;

		if (!is_shutting_down) {
			queue_read(&p->jobqueue, &job);
		} else {
			if (queue_try_read(&p->jobqueue, &job))
				break;
			/* this is optional, let fail all remaining
			 * jobs so that shutdown is done quicker */
			#if 1
			if (p->on_job_fail)
				p->on_job_fail(job);
			continue;
			#endif
		}
		if (!job) {
			is_shutting_down = 1;
			continue;
		}
		while (i < p->try_count) {
			if (i)
				thread_msleep(p->retry_gracetime);
			/* fails if no workers are active */
			if (!_wp_try_invoke_work(p, job)) {
				s = 1;
				break;
			}
			++i;
		}
		if (!s) {
			if (p->on_job_fail)
				p->on_job_fail(job);
		}
	}
}

static void _activate_worker
(wp_i_t* p, worker_t* w, const void* new_key, void* user_arg)
{
	if (!p->on_activate) {
		thread_set_ptr(&w->key, new_key);
		return;
	}

	if (!p->on_activate(&w->worker_specific, new_key, user_arg))
		thread_set_ptr(&w->key, new_key);
	else
		thread_set_ptr(&w->key, KEY_UNASSIGNED);

	thread_set_ptr(&w->activating_for_key, KEY_UNASSIGNED);
}

static void _deactivate_worker(wp_i_t* p, worker_t* w)
{
	if (p->on_deactivate)
		p->on_deactivate(w->worker_specific);
	thread_set_ptr(&w->key, KEY_UNASSIGNED);
}

static void _invoke_worker(wp_i_t* p, worker_t* w, void* work)
{
	if (!p->on_invoke || !p->on_job_done) {
		return;
	}

	/* this might happen if we deactivate ourself */
	if (w->key == KEY_UNASSIGNED) {
		queue_pwrite(&p->jobqueue, work);
		return;
	}

	if (!p->on_invoke(w->worker_specific, work)) {
		p->on_job_done(work);
	} else {
		_deactivate_worker(p, w);
		queue_pwrite(&p->jobqueue, work);
	}
}

static void invoke_worker(void* arg)
{
	worker_args_t* a = (worker_args_t*)arg;
	worker_activate_args_t* aa;
	wp_i_t* p;
	worker_t* w;
	int running = 1;
	void* oparg;
	worker_action op;

	p = a->pool;
	w = a->self;
	free(a);
	while (running) {
		op = worker_get_action(w, &oparg);
		switch (op) {
			case OP_SHUTDOWN:
				running = 0;
				break;
			case OP_ACTIVATE:
				aa = (worker_activate_args_t*)oparg;
				_activate_worker(p, w, aa->key, aa->user_arg);
				free(aa);
				break;
			case OP_DEACTIVATE:
				_deactivate_worker(p, w);
				break;
			case OP_WORK:
				_invoke_worker(p, w, (void*)oparg);
				break;
			default:
				break;
		}
	}
}

static int _wp_add_worker(wp_i_t* p)
{
	worker_t* w;
	worker_args_t* a;
	int res = -1;

	w = (worker_t*)calloc(1, sizeof(worker_t));

	a = (worker_args_t*)calloc(1, sizeof(worker_args_t));
	a->pool = p;
	a->self = w;

	cond_init(&w->may_process_action);
	cond_init(&w->may_invoke_action);
	mutex_init(&w->lock, 0);

	if (thread_create(&w->thread, &invoke_worker, a)) {
		goto out;
	}

	w->next = p->first_worker;
	p->first_worker = w;
	p->used_worker = p->first_worker;
	res = 0;
out:
	return res;
}

void wp_deactivate_all(wp_t* pool)
{
	wp_i_t* p = (wp_i_t*)*pool;
	worker_t* n = 0;
	worker_t* w = 0;

	mutex_lock(&p->lock);
	w = p->first_worker;
	while (w) {
		n = w->next;
		if ((w->key != KEY_DEACTIVATED)
		&& (w->key != KEY_UNASSIGNED)) {
			w->key = KEY_DEACTIVATED;
			worker_invoke_action(w, OP_DEACTIVATE, 0);
		}
		w = n;
	}
	mutex_unlock(&p->lock);
}

void wp_shutdown(wp_t* pool)
{
	wp_i_t* p = (wp_i_t*)*pool;
	worker_t* n = 0;
	worker_t* w = 0;

	mutex_lock(&p->lock);
	p->is_shuttingdown = 1;
	mutex_unlock(&p->lock);

	queue_notify(&p->jobqueue, 0);
	thread_join(&p->distributor);

	mutex_lock(&p->lock);
	w = p->first_worker;
	while (w) {
		n = w->next;
		if ((w->key != KEY_DEACTIVATED)
		&& (w->key != KEY_UNASSIGNED)) {
			w->key = KEY_DEACTIVATED;
			worker_invoke_action(w, OP_DEACTIVATE, 0);
		}
		worker_invoke_action(w, OP_SHUTDOWN, 0);
		thread_join(&w->thread);
		cond_destroy(&w->may_process_action);
		cond_destroy(&w->may_invoke_action);
		mutex_destroy(&w->lock);
		free(w);
		w = n;
	}
	p->first_worker = 0;
	p->used_worker = 0;
	p->shutdown = 1;
	p->is_shuttingdown = 0;
	cond_broadcast(&p->may_fail);
	mutex_unlock(&p->lock);
}

void wp_destroy(wp_t* pool)
{
	wp_i_t* p = (wp_i_t*)*pool;

	queue_destroy(&p->jobqueue);
	cond_destroy(&p->may_fail);
	mutex_destroy(&p->lock);
	free(p);
	*pool = 0;
}

wp_error wp_init(wp_t* pool, wp_opts_t* opts)
{
	wp_i_t* p = 0;
	wp_error e = WP_ESUCCESS;
	size_t i;

	p = (wp_i_t*) calloc(1, sizeof(wp_i_t));
	if (!p) {
		e = WP_ENOMEM;
		goto err;
	}

	p->nworkers		= opts->max_workers;
	p->max_queued_jobs	= opts->max_queued_jobs;
	p->try_count		= opts->queue_try_count;
	p->retry_gracetime	= opts->retry_gracetime;

	p->on_activate		= opts->on_activate;
	p->on_deactivate	= opts->on_deactivate;
	p->on_invoke		= opts->on_invoke;
	p->on_job_done		= opts->on_job_done;
	p->on_job_fail		= opts->on_job_fail;

	mutex_init(&p->lock, 0);
	cond_init(&p->may_fail);

	for (i = 0; i < p->nworkers; ++i) {
		_wp_add_worker(p);
	}

	queue_init(&p->jobqueue, p->max_queued_jobs, p->nworkers*2);

	thread_create(&p->distributor, &distribute_jobs, p);

	*pool = p;
	goto out;

err:
	*pool = 0;
out:
	return e;
}
