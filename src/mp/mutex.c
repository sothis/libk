#include "mutex.h"

#include <stdlib.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN	1
#define VC_EXTRALEAN		1
#include <windows.h>
#else
#include <pthread.h>
#endif
#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

typedef struct mutex_i_s {
#if defined(WIN32)
	CRITICAL_SECTION	lock;
#else
	pthread_mutex_t		lock;
#endif
} mutex_i_t;


mutex_error mutex_init(mutex_t* mutex, int lock)
{
	mutex_i_t* mi = 0;
	mutex_error e = MUTEX_ESUCCESS;

	mi = (mutex_i_t*)calloc(1, sizeof(mutex_i_t));
	if (!mi) {
		e = MUTEX_ENOMEM;
		goto err;
	}
	#if defined(WIN32)
	InitializeCriticalSection(&mi->lock);
	if (lock)
		EnterCriticalSection(&mi->lock);
	#else
	if (pthread_mutex_init(&mi->lock, 0)) {
		e = MUTEX_ENOLOCK;
		goto err;
	}
	if (lock)
		pthread_mutex_lock(&mi->lock);
	#endif

	*mutex = mi;
	goto out;
err:
	if (mi) {
		free(mi);
	}
	*mutex = 0;
out:
	return e;
}

void mutex_destroy(mutex_t* mutex)
{
	mutex_i_t* mi = (mutex_i_t*)*mutex;
	#if defined(WIN32)
	DeleteCriticalSection(&mi->lock);
	#else
	pthread_mutex_destroy(&mi->lock);
	#endif
	free(mi);
	*mutex = 0;
}

intptr_t mutex_get_internal_handle(mutex_t* mutex)
{
	mutex_i_t* mi = (mutex_i_t*)*mutex;
	return (intptr_t)&mi->lock;
}

void mutex_lock(mutex_t* mutex)
{
	mutex_i_t* mi = (mutex_i_t*)*mutex;
	#if defined(WIN32)
	EnterCriticalSection(&mi->lock);
	#else
	pthread_mutex_lock(&mi->lock);
	#endif
}

int mutex_try_lock(mutex_t* mutex)
{
	mutex_i_t* mi = (mutex_i_t*)*mutex;

	#if defined(WIN32)
	if (!TryEnterCriticalSection(&mi->lock))
		return -1;
	return 0;
	#else
	if (!pthread_mutex_trylock(&mi->lock))
		return 0;
	return -1;
	#endif
}

void mutex_unlock(mutex_t* mutex)
{
	mutex_i_t* mi = (mutex_i_t*)*mutex;
	#if defined(WIN32)
	LeaveCriticalSection(&mi->lock);
	#else
	pthread_mutex_unlock(&mi->lock);
	#endif
}
