#ifndef _QUEUE_H
#define _QUEUE_H

#include <stddef.h>

typedef enum queue_error_e {
	QUEUE_ESUCCESS		= 0x00000000,
	QUEUE_ENOMEM		= 0x80000001,
	QUEUE_EINVAL		= 0x80000002,
} queue_error;

typedef void*	queue_t;

#if defined(__cplusplus)
extern "C" {
#endif

extern queue_error queue_init
(queue_t* queue, size_t maxitems, size_t pmaxitems);

extern void queue_destroy
(queue_t* queue);

extern void queue_read
(queue_t* queue, void** item);

extern int queue_try_read
(queue_t* queue, void** item);

extern void queue_write
(queue_t* queue, void* item);

extern void queue_pwrite
(queue_t* queue, void* item);

extern void queue_notify
(queue_t* queue, void* item);

#if defined(__cplusplus)
}
#endif

#endif /* _QUEUE_H */
