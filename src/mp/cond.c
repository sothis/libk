#include "cond.h"
#include "thread.h"

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

#if defined(WIN32)

typedef struct waiter_s {
	intptr_t		thread_id;
	HANDLE			wait;
	int32_t			waiting;
	struct waiter_s*	next;
} waiter_t;

typedef struct cond_i_s {
	waiter_t*		first_waiter;
} cond_i_t;

cond_error cond_init(cond_t* cond)
{
	cond_i_t* c = 0;
	cond_error e = COND_ESUCCESS;

	c = (cond_i_t*) calloc(1, sizeof(cond_i_t));
	if (!c) {
		e = COND_ENOMEM;
		goto err;
	}

	*cond = c;
	goto out;
err:
	if (c)
		free(c);
	*cond = 0;
out:
	return e;
}

void cond_destroy(cond_t* cond)
{
	cond_i_t* c = (cond_i_t*)*cond;
	waiter_t* w, *n;

	w = c->first_waiter;
	while (w) {
		n = w->next;
		CloseHandle(w->wait);
		free(w);
		w = n;
	}
	free(c);
	*cond = 0;
}

static waiter_t* _get_myself(cond_t* cond)
{
	cond_i_t* c = (cond_i_t*)*cond;
	intptr_t tid = thread_self();
	waiter_t* w;

	w = c->first_waiter;
	while (w) {
		if (w->thread_id == tid) {
			w->waiting = 1;
			return w;
		}
		w = w->next;
	}

	w = (waiter_t*)malloc(sizeof(waiter_t));
	if (!w)
		return 0;

	w->thread_id = tid;
	w->wait = CreateEvent(0, 0, 0, 0);
	if (!w->wait) {
		free(w);
		return 0;
	}
	w->next = c->first_waiter;
	c->first_waiter = w;
	w->waiting = 1;
	return w;
}

cond_error cond_wait(cond_t* cond, mutex_t* mutex)
{
	cond_error e = COND_ESUCCESS;
	waiter_t* w = _get_myself(cond);

	if (!w)
		return COND_ENOMEM;

	mutex_unlock(mutex);
	WaitForSingleObject(w->wait, INFINITE);
	mutex_lock(mutex);
	return e;
}

cond_error cond_signal(cond_t* cond)
{
	cond_error e = COND_ESUCCESS;
	cond_i_t* c = (cond_i_t*)*cond;
	waiter_t* w;

	w = c->first_waiter;
	while (w) {
		if (w->waiting) {
			SetEvent(w->wait);
			w->waiting = 0;
			return e;
		}
		w = w->next;
	}
	return e;
}

cond_error cond_broadcast(cond_t* cond)
{
	cond_error e = COND_ESUCCESS;
	cond_i_t* c = (cond_i_t*)*cond;
	waiter_t* w;

	w = c->first_waiter;
	while (w) {
		if (w->waiting) {
			SetEvent(w->wait);
			w->waiting = 0;
		}
		w = w->next;
	}
	return e;
}

#endif /* WIN32 */

#if !defined(WIN32)

typedef struct cond_i_s {
	pthread_cond_t		cond;
	pthread_condattr_t	condattr;
} cond_i_t;

cond_error cond_init(cond_t* cond)
{
	cond_i_t* c = 0;
	cond_error e = COND_ESUCCESS;
	int have_condattr = 0;

	c = (cond_i_t*)calloc(1, sizeof(cond_i_t));
	if (!c) {
		e = COND_ENOMEM;
		goto err;
	}

	if (pthread_condattr_init(&c->condattr)) {
		e = COND_ENOEV;
		goto err;
	}
	have_condattr = 1;
	if (pthread_cond_init(&c->cond, &c->condattr)) {
		e = COND_ENOEV;
		goto err;
	}

	*cond = c;
	goto out;
err:
	if (have_condattr)
		pthread_condattr_destroy(&c->condattr);
	if (c) {
		free(c);
	}
	*cond = 0;
out:
	return e;
}

void cond_destroy(cond_t* cond)
{
	cond_i_t* c = (cond_i_t*)*cond;

	pthread_cond_destroy(&c->cond);
	pthread_condattr_destroy(&c->condattr);

	free(c);
	*cond = 0;
}

cond_error cond_wait(cond_t* cond, mutex_t* mutex)
{
	cond_error e = COND_ESUCCESS;
	cond_i_t* c = (cond_i_t*)*cond;
	pthread_mutex_t* m = (pthread_mutex_t*)mutex_get_internal_handle(mutex);

	pthread_cond_wait(&c->cond, m);
	return e;
}

cond_error cond_signal(cond_t* cond)
{
	cond_error e = COND_ESUCCESS;
	cond_i_t* c = (cond_i_t*)*cond;

	pthread_cond_signal(&c->cond);
	return e;
}

cond_error cond_broadcast(cond_t* cond)
{
	cond_error e = COND_ESUCCESS;
	cond_i_t* c = (cond_i_t*)*cond;

	pthread_cond_broadcast(&c->cond);
	return e;
}

#endif /* !WIN32 */
