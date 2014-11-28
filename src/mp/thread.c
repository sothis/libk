#include "thread.h"
#include "cond.h"

#include <stdlib.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN	1
#define VC_EXTRALEAN		1
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#include <sys/select.h>
#endif
#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

typedef struct thread_i_s {
#if defined(WIN32)
	HANDLE			thread;
#else
	pthread_t		thread;
#endif
	intptr_t		thread_id;
	thread_fn		proc;
	void*			proc_arg;

	int32_t			ready;
	cond_t			may_procceed;
	mutex_t			lock;
} thread_i_t;

intptr_t thread_self(void)
{
	#if defined(WIN32)
	return GetCurrentThreadId();
	#else
	return (intptr_t)pthread_self();
	#endif
}

intptr_t thread_id(thread_t* thread)
{
	thread_i_t* t = (thread_i_t*)*thread;
	return t->thread_id;
}

#if defined(WIN32)
static DWORD WINAPI thread_invoke(void* arg)
#else
static void* thread_invoke(void* arg)
#endif
{
	thread_i_t* t = (thread_i_t*)arg;

	mutex_lock(&t->lock);
	t->thread_id = thread_self();
	t->ready = 1;
	cond_signal(&t->may_procceed);
	mutex_unlock(&t->lock);

	t->proc(t->proc_arg);
	return 0;
}

static atomic_int64_t invokations = 0;

uint64_t get_thread_invokations(void)
{
	return (uint64_t) invokations;
}

thread_error
thread_create(thread_t* thread, thread_fn proc, void* arg)
{
	thread_error e = THREAD_ESUCCESS;
	thread_i_t* t = 0;
	int have_cond = 0;
	int have_lock = 0;

	thread_add64(&invokations, 1);

	t = (thread_i_t*) calloc(1, sizeof(thread_i_t));
	if (!t) {
		e = THREAD_EINTERNAL;
		goto err;
	}

	t->proc = proc;
	t->proc_arg = arg;

	if (mutex_init(&t->lock, 0)) {
		e = THREAD_EINTERNAL;
		goto err;
	}
	have_lock = 1;

	if (cond_init(&t->may_procceed)) {
		e = THREAD_EINTERNAL;
		goto err;
	}
	have_cond = 1;

	#if defined(WIN32)
	t->thread = CreateThread(0, 0, &thread_invoke, t, 0, 0);
	if (!t->thread) {
		e = THREAD_EINTERNAL;
		goto err;
	}
	#else
	if (pthread_create(&t->thread, 0, &thread_invoke, t)) {
		e = THREAD_EINTERNAL;
		goto err;
	}
	#endif

	mutex_lock(&t->lock);
	while (!t->ready)
		cond_wait(&t->may_procceed, &t->lock);
	mutex_unlock(&t->lock);

	*thread = t;
	goto out;
err:
	if (have_lock)
		mutex_destroy(&t->lock);
	if (have_cond)
		cond_destroy(&t->may_procceed);
	if (t)
		free(t);
	*thread = 0;
out:
	return e;
}

thread_error thread_join(thread_t* thread)
{
	thread_error e = THREAD_ESUCCESS;

	thread_i_t* t = (thread_i_t*)*thread;
	#if defined(WIN32)
	if (WaitForSingleObject(t->thread, INFINITE) != WAIT_OBJECT_0) {
		e = THREAD_EINTERNAL;
	}
	CloseHandle(t->thread);
	#else
	if (pthread_join(t->thread, 0)) {
		e = THREAD_EINTERNAL;
	}
	#endif
	cond_destroy(&t->may_procceed);
	mutex_destroy(&t->lock);
	free(t);
	*thread = 0;
	return e;
}

void thread_msleep(int32_t milliseconds)
{
	#if !defined(WIN32)
	struct timeval to_wait;
	#endif

	if (milliseconds <= 0)
		return;
	#if defined(WIN32)
	Sleep(milliseconds);
	#else
	to_wait.tv_sec = milliseconds / 1000;
	to_wait.tv_usec = (milliseconds % 1000) * 1000;
	select(0, 0, 0, 0, &to_wait);
	#endif
}

void thread_yield(void)
{
	#if defined(WIN32)
	SwitchToThread();
	#else
	sched_yield();
	#endif
}

void thread_fullbarrier(void)
{
	#if !defined(_MSC_VER)
	__sync_synchronize();
	#else
	_ReadWriteBarrier();
	#endif
}

int32_t thread_set32(atomic_int32_t* a, int32_t val)
{
	#if !defined(_MSC_VER)
	return __sync_val_compare_and_swap (a, *a, val);
	#else
	return InterlockedExchange(a, val);
	#endif
}

int32_t thread_add32(atomic_int32_t* a, int32_t val)
{
	#if !defined(_MSC_VER)
	return __sync_add_and_fetch(a, val);
	#else
	int32_t r;
	r = InterlockedExchangeAdd(a, val);
	return r + val;
	#endif
}

int32_t thread_sub32(atomic_int32_t* a, int32_t val)
{
	#if !defined(_MSC_VER)
	return __sync_sub_and_fetch(a, val);
	#else
	int32_t r;
	r = InterlockedExchangeAdd(a, -val);
	return r - val;
	#endif
}

int64_t thread_set64(atomic_int64_t* a, int64_t val)
{
	#if !defined(_MSC_VER)
	return __sync_val_compare_and_swap (a, *a, val);
	#else
	return InterlockedExchange64(a, val);
	#endif
}

int64_t thread_add64(atomic_int64_t* a, int64_t val)
{
	#if !defined(_MSC_VER)
	return __sync_add_and_fetch(a, val);
	#else
	int64_t r;
	r = InterlockedExchangeAdd64(a, val);
	return r + val;
	#endif
}

int64_t thread_sub64(atomic_int64_t* a, int64_t val)
{
	#if !defined(_MSC_VER)
	return __sync_sub_and_fetch(a, val);
	#else
	int64_t r;
	r = InterlockedExchangeAdd64(a, -val);
	return r - val;
	#endif
}

size_t thread_set_size(atomic_size_t* a, size_t val)
{
	if (sizeof(atomic_size_t) == 4) {
		return (size_t)thread_set32((atomic_int32_t*)a, (int32_t)val);
	}
	if (sizeof(atomic_size_t) == 8) {
		return (size_t)thread_set64((atomic_int64_t*)a, (int64_t)val);
	}
}

const void* thread_set_ptr(atomic_ptr_t* a, const void* val)
{
	intptr_t r, v = (intptr_t)val;
	if (sizeof(atomic_ptr_t) == 4) {
		r = (intptr_t)thread_set32((atomic_int32_t*)a, (int32_t)v);
		return (const void*)r;
	}
	if (sizeof(atomic_ptr_t) == 8) {
		r = (intptr_t)thread_set64((atomic_int64_t*)a, (int64_t)v);
		return (const void*)r;
	}
}

size_t thread_add_size(atomic_size_t* a, size_t val)
{
	if (sizeof(atomic_size_t) == 4) {
		return (size_t)thread_add32((atomic_int32_t*)a, (int32_t)val);
	}
	if (sizeof(atomic_size_t) == 8) {
		return (size_t)thread_add64((atomic_int64_t*)a, (int64_t)val);
	}
}

size_t thread_sub_size(atomic_size_t* a, size_t val)
{
	if (sizeof(atomic_size_t) == 4) {
		return (size_t)thread_sub32((atomic_int32_t*)a, (int32_t)val);
	}
	if (sizeof(atomic_size_t) == 8) {
		return (size_t)thread_sub64((atomic_int64_t*)a, (int64_t)val);
	}
}


void thread_spin_lock(spinlock_t* spinlock, int spincount_until_yield)
{
	int count = 0;
	#if !defined(_MSC_VER)
	while(!__sync_bool_compare_and_swap(spinlock, 0, 1)) {
	#else
	while(InterlockedCompareExchange(spinlock, 1, 0)) {
	#endif
		if (!(++count % spincount_until_yield)) {
			thread_yield();
		}
	}
}

void thread_spin_unlock(spinlock_t* spinlock)
{
	thread_fullbarrier();
	*spinlock = 0;
}
