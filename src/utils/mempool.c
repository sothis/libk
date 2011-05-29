/*
 * libk - mempool.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <libk/libk.h>
#include <stdlib.h>
#include <string.h>

/* TODO: overflow checking */

static int pool_set_pages(struct mempool_t* p, ssize_t pagediff)
{
	size_t old = p->pages;
	if (((ssize_t)p->pages + pagediff) < 0)
		p->pages = 0;
	else
		p->pages += pagediff;

	void* temp = realloc(p->base, p->pages * p->page_size);
	if (p->pages && !temp) {
		p->pages = old;
		return -1;
	}
	if (pagediff > 0)
		memset(temp + (old*p->page_size), 0, pagediff*p->page_size);
	p->base = temp;
	return 0;
}

static int pool_resize(struct mempool_t* p, size_t s)
{
	size_t old = p->data_size;
	size_t new_pagecount = (s + (p->page_size-1)) / p->page_size;
	ssize_t diff = new_pagecount - p->pages;

	p->data_size = s;
	if (!diff)
		return 0;
	if (pool_set_pages(p, diff)) {
		p->data_size = old;
		return -1;
	}
	return 0;
}

int pool_alloc(struct mempool_t* p, size_t page_size)
{
	memset(p, 0, sizeof(struct mempool_t));
	p->page_size = page_size ? page_size : 2*1024*1024;
	int res = pthread_mutex_init(&p->lock, 0);
	if (!res)
		p->alloced = 1;
	return res;
}

int pool_free(struct mempool_t* p)
{
	int r;
	pthread_mutex_lock(&p->lock);
	if (p->base)
		free(p->base);
	pthread_mutex_unlock(&p->lock);
	r = pthread_mutex_destroy(&p->lock);
	if (!r)
		memset(p, 0, sizeof(struct mempool_t));
	return r;
}

void* pool_getmem(struct mempool_t* p, size_t off)
{
	void* r;
	pthread_mutex_lock(&p->lock);
	r = p->base + off;
	pthread_mutex_unlock(&p->lock);
	return r;
}

size_t pool_getsize(struct mempool_t* p)
{
	size_t r;
	pthread_mutex_lock(&p->lock);
	r = p->data_size;
	pthread_mutex_unlock(&p->lock);
	return r;
}

int pool_insert(struct mempool_t* p, size_t off, const void* src, size_t s)
{
	int r = 0;
	pthread_mutex_lock(&p->lock);
	if (!s || (off > p->data_size))
		goto out;

	size_t old_size = p->data_size;

	if (pool_resize(p, old_size + s))
		goto out_err;

	memmove(p->base + off + s, p->base + off, old_size - off);
	memmove(p->base + off, src, s);
	goto out;
out_err:
	r = -1;
out:
	pthread_mutex_unlock(&p->lock);
	return r;
}

int pool_append(struct mempool_t* p, const void* src, size_t s)
{
	return pool_insert(p, p->data_size, src, s);
}

int pool_prepend(struct mempool_t* p, const void* src, size_t s)
{
	return pool_insert(p, 0, src, s);
}

void pool_cut(struct mempool_t* p, size_t off, size_t s)
{
	pthread_mutex_lock(&p->lock);
	if (!s || (off + s > p->data_size))
		goto out;

	memmove(p->base + off, p->base + off + s, p->data_size - off - s);
	pool_resize(p, p->data_size - s);
out:
	pthread_mutex_unlock(&p->lock);
}

void pool_detach(struct mempool_t* p, size_t s)
{
	pool_cut(p, p->data_size - s, s);
}
