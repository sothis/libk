#ifndef _THREAD_H
#define _THREAD_H

#include <stdint.h>
#include <stddef.h>

typedef enum thread_error_e {
	THREAD_ESUCCESS		= 0x00000000,
	THREAD_EINTERNAL	= 0x80000001,
} thread_error;

typedef void*	thread_t;
typedef void (*thread_fn)(void* arg);

#define THREAD_SPINLOCK_INITIALIZER	0;

typedef volatile int32_t	spinlock_t;
typedef volatile int32_t	atomic_int32_t;
typedef volatile int64_t	atomic_int64_t;
typedef volatile size_t		atomic_size_t;
typedef volatile const void*	atomic_ptr_t;

#if defined(__cplusplus)
extern "C" {
#endif

/* to be removed */
uint64_t get_thread_invokations(void);

extern thread_error thread_create
(thread_t* thread, thread_fn proc, void* arg);
extern thread_error thread_join(thread_t* thread);
extern intptr_t thread_self(void);
extern intptr_t thread_id(thread_t* thread);

extern void thread_msleep(int32_t milliseconds);
extern void thread_yield(void);
extern void thread_fullbarrier(void);
/* atomically sets *a to val, returns old value of *a */
extern int32_t thread_set32(atomic_int32_t* a, int32_t val);
extern int64_t thread_set64(atomic_int64_t* a, int64_t val);
extern size_t thread_set_size(atomic_size_t* a, size_t val);
extern const void* thread_set_ptr(atomic_ptr_t* a, const void* val);
/* atomically adds val to *a, returns new value of *a */
extern int32_t thread_add32(atomic_int32_t* a, int32_t val);
extern int64_t thread_add64(atomic_int64_t* a, int64_t val);
extern size_t thread_add_size(atomic_size_t* a, size_t val);
/* atomically substracts val from *a, returns new value of *a */
extern int32_t thread_sub32(atomic_int32_t* a, int32_t val);
extern int64_t thread_sub64(atomic_int64_t* a, int64_t val);
extern size_t thread_sub_size(atomic_size_t* a, size_t val);

/* a spinlock must be initialized with:
 * 	spinlock_t spinlock = THREAD_SPINLOCK_INITIALIZER;
 */
/* note: only use a spinlock if the work in between the lock requires
 * only a few cpu cycles. */
extern void thread_spin_lock(spinlock_t* spinlock, int spincount_until_yield);
extern void thread_spin_unlock(spinlock_t* spinlock);

#if defined(__cplusplus)
}
#endif

#endif /* _THREAD_H */
