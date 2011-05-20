#include "workbench.h"
#include "utils/mem.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __WINNT__
#include <windows.h>
#endif

struct worker_t {
	pthread_t		thread;
};

struct workbench_t {
	worker_fn		workerfn;
	pthread_mutex_t		lock;
	size_t			n_workers;
	struct worker_t*	workers;
	ssize_t			njobs;
	ssize_t			nworking;
	pthread_cond_t		notfull, notempty, notworking;
	void*			job_buf[QUEUED_JOBS];
};

static size_t get_online_cpu_count(void)
{
	size_t ncpus;
#ifndef __WINNT__
	if ((ncpus = sysconf(_SC_NPROCESSORS_ONLN)) == -1)
		ncpus = 1;
#else
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	if ((ncpus = si.dwNumberOfProcessors) == 0)
		ncpus = 1;
#endif
	return ncpus;
}

static void* _worker_fn(void* arg)
{
	void* worker_arg;
	struct workbench_t* wb = arg;

	while (1) {
		pthread_mutex_lock(&wb->lock);
		while (!wb->njobs)
			pthread_cond_wait(&wb->notempty, &wb->lock);
		if (wb->njobs < 0) {
			pthread_mutex_unlock(&wb->lock);
			return 0;
		}
		worker_arg = wb->job_buf[wb->njobs-1];
		wb->njobs--;
		pthread_cond_signal(&wb->notfull);
		pthread_mutex_unlock(&wb->lock);

		if (wb->workerfn)
			wb->workerfn(worker_arg);

		pthread_mutex_lock(&wb->lock);
		wb->nworking--;
		pthread_cond_signal(&wb->notworking);
		pthread_mutex_unlock(&wb->lock);
	}
	return 0;
}

struct workbench_t*
workbench_create(size_t n_workers)
{
	size_t running = 0;
	struct workbench_t* wb = k_calloc(1, sizeof(struct workbench_t));
	if (!wb)
		return 0;

	wb->n_workers = n_workers ? n_workers : get_online_cpu_count();
	wb->workers = k_calloc(wb->n_workers, sizeof(struct worker_t));
	if (!wb->workers) {
		k_free(wb);
		return 0;
	}


	pthread_mutex_init(&wb->lock, 0);
	pthread_cond_init(&wb->notfull, 0);
	pthread_cond_init(&wb->notempty, 0);
	pthread_cond_init(&wb->notworking, 0);

	for (size_t i = 0; i < wb->n_workers; ++i) {
		if (!pthread_create(&wb->workers[running].thread, 0,
			_worker_fn, wb)) {
			running++;
		}
	}
	wb->n_workers = running;
	if (!running) {
		k_free(wb->workers);
		k_free(wb);
		wb = 0;
	}
	return wb;
}

void workbench_set_worker(struct workbench_t* wb, worker_fn fn)
{
	wb->workerfn = fn;
}

void workbench_join(struct workbench_t* wb)
{
	pthread_mutex_lock(&wb->lock);
	while (wb->nworking > 0)
		pthread_cond_wait(&wb->notworking, &wb->lock);
	pthread_mutex_unlock(&wb->lock);
}

void workbench_delete(struct workbench_t* wb)
{
	pthread_mutex_lock(&wb->lock);
	while (wb->nworking > 0)
		pthread_cond_wait(&wb->notworking, &wb->lock);
	wb->njobs = -1;
	pthread_cond_broadcast(&wb->notempty);
	pthread_mutex_unlock(&wb->lock);

	for (size_t i = 0; i < wb->n_workers; ++i) {
		pthread_join(wb->workers[i].thread, 0);
	}
	pthread_mutex_destroy(&wb->lock);
	pthread_cond_destroy(&wb->notfull);
	pthread_cond_destroy(&wb->notempty);
	pthread_cond_destroy(&wb->notworking);
	k_free(wb->workers);
	k_free(wb);
}

void workbench_commit(struct workbench_t* wb, void* arg)
{
	pthread_mutex_lock(&wb->lock);
	while (wb->njobs == QUEUED_JOBS)
		pthread_cond_wait(&wb->notfull, &wb->lock);
	wb->job_buf[wb->njobs] = arg;
	wb->njobs++;
	wb->nworking++;
	pthread_cond_signal(&wb->notempty);
	pthread_mutex_unlock(&wb->lock);
}

size_t workbench_get_running_workers(struct workbench_t* wb)
{
	return wb->n_workers;
}
