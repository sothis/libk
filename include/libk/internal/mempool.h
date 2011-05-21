/*
 * mempool.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _MEMPOOL_H
#define _MEMPOOL_H

#include <stddef.h>
#include <pthread.h>

struct mempool_t {
	void*		base;
	size_t		data_size;
	size_t		page_size;
	size_t		pages;
	pthread_mutex_t	lock;
};

int pool_alloc(struct mempool_t* p, size_t page_size);
int pool_free(struct mempool_t* p);
void* pool_getmem(struct mempool_t* p, size_t off);
size_t pool_getsize(struct mempool_t* p);

int pool_insert(struct mempool_t* p, size_t off, const void* src, size_t s);
int pool_append(struct mempool_t* p, const void* src, size_t s);
int pool_prepend(struct mempool_t* p, const void* src, size_t s);

void pool_cut(struct mempool_t* p, size_t off, size_t s);
void pool_detach(struct mempool_t* p, size_t s);

#endif /* _MEMPOOL_H */
