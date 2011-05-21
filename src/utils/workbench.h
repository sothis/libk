/*
 * libk - workbench.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

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
