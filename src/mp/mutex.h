#ifndef _MUTEX_H
#define _MUTEX_H

#include <stdint.h>

typedef enum mutex_error_e {
	MUTEX_ESUCCESS		= 0x00000000,
	MUTEX_ENOMEM		= 0x80000001,
	MUTEX_ENOLOCK		= 0x80000002,
} mutex_error;

typedef void*	mutex_t;

#if defined(__cplusplus)
extern "C" {
#endif

extern mutex_error mutex_init
(mutex_t* mutex, int lock);

extern void mutex_destroy
(mutex_t* mutex);

extern intptr_t mutex_get_internal_handle
(mutex_t* mutex);

extern void mutex_lock
(mutex_t* mutex);

extern int mutex_try_lock
(mutex_t* mutex);

extern void mutex_unlock
(mutex_t* mutex);

#if defined(__cplusplus)
}
#endif

#endif /* _MUTEX_H */
