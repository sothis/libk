#ifndef _WORKBENCH_H
#define _WORKBENCH_H

#include <stddef.h>

#define QUEUED_JOBS	32

typedef struct workbench_t* workbench_t;
typedef void (*worker_fn)(void* arguments);

workbench_t workbench_create(size_t n_workers);
void workbench_delete(workbench_t wb);
void workbench_commit(workbench_t wb, void* arg);
void workbench_join(workbench_t wb);
void workbench_set_worker(struct workbench_t* wb, worker_fn fn);
size_t workbench_get_running_workers(struct workbench_t* wb);

#endif /* _WORKBENCH_H */
