#ifndef _COND_H
#define _COND_H

#include "mutex.h"
#include <stdint.h>

typedef enum cond_error_e {
	COND_ESUCCESS		= 0x00000000,
	COND_ENOMEM		= 0x80000001,
	COND_ENOEV		= 0x80000002,
} cond_error;

typedef void*	cond_t;

#if defined(__cplusplus)
extern "C" {
#endif

extern cond_error cond_init(cond_t* cond);
extern void cond_destroy(cond_t* cond);

extern cond_error cond_wait(cond_t* cond, mutex_t* mutex);
extern cond_error cond_signal(cond_t* cond);
extern cond_error cond_broadcast(cond_t* cond);

#if defined(__cplusplus)
}
#endif

#endif /* _COND_H */
