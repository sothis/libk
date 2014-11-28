#ifndef _WORKERPOOL_H
#define _WORKERPOOL_H

#include <stdint.h>
#include <stddef.h>

typedef enum wp_error_e {
	WP_ESUCCESS	= 0x00000000,
	WP_ENOMEM	= 0x80000001,
	WP_ENOWORKER	= 0x80000002,
	WP_ESHUTDOWN	= 0x80000003,
} wp_error;

typedef int (*worker_activate_fn)
(void** worker_specific, const void* key, void* user_arg);

typedef void (*worker_deactivate_fn)
(void* worker_specific);

typedef int (*worker_invoke_fn)
(void* worker_specific, void* job);

typedef void (*worker_done_fn)
(void* job);

typedef void (*worker_fail_fn)
(void* job);

typedef struct wp_opts_s {
	size_t			max_workers;
	size_t			max_queued_jobs;
	size_t			queue_try_count;
	int			retry_gracetime;

	worker_activate_fn	on_activate;
	worker_deactivate_fn	on_deactivate;
	worker_invoke_fn	on_invoke;
	worker_done_fn		on_job_done;
	worker_fail_fn		on_job_fail;
} wp_opts_t;

typedef void*	wp_t;


#if defined(__cplusplus)
extern "C" {
#endif


extern wp_error wp_init
(wp_t* pool, wp_opts_t* opts);

extern void wp_shutdown
(wp_t* pool);

extern void wp_destroy
(wp_t* pool);

extern wp_error wp_activate_worker
(wp_t* pool, const void* key, void* user_arg);

extern void wp_deactivate_worker
(wp_t* pool, const void* key);

extern void wp_deactivate_all
(wp_t* pool);

extern wp_error wp_post_work
(wp_t* pool, void* job);

#if defined(__cplusplus)
}
#endif

#endif /* _WORKERPOOL_H */
