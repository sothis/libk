#include "queue.h"
#include "cond.h"
#include <stdlib.h>

typedef struct queue_s {
	void**		buf;
	void**		pbuf;
	void*		notify;
	int		notified;

	size_t		size;
	size_t		maxitems;
	size_t		items;
	size_t		read;
	size_t		write;

	size_t		psize;
	size_t		pmaxitems;
	size_t		pitems;
	size_t		pread;
	size_t		pwrite;

	mutex_t		lock;
	cond_t		may_read;
	cond_t		may_write;
	cond_t		may_pwrite;
	cond_t		may_notify;
} queue_i_t;

queue_error queue_init
(queue_t* queue, size_t maxitems, size_t pmaxitems)
{
	queue_i_t* q = 0;
	queue_error e = QUEUE_ESUCCESS;
	int have_buf = 0;
	int have_pbuf = 0;
	int have_lock = 0;
	int have_cond_read = 0;
	int have_cond_write = 0;
	int have_cond_pwrite = 0;

	if (!maxitems || !pmaxitems) {
		e = QUEUE_EINVAL;
		goto err;
	}
	q = (queue_i_t*)malloc(sizeof(queue_i_t));
	if (!q) {
		e = QUEUE_ENOMEM;
		goto err;
	}

	q->maxitems = maxitems;
	q->size = maxitems + 1;
	q->items = q->read = q->write = 0;

	q->pmaxitems = pmaxitems;
	q->psize = maxitems + 1;
	q->pitems = q->pread = q->pwrite = 0;

	q->notify = 0;
	q->notified = 0;

	q->buf = (void**)calloc(q->size, sizeof(void*));
	if (!q->buf) {
		e = QUEUE_ENOMEM;
		goto err;
	}
	have_buf = 1;
	q->pbuf = (void**)calloc(q->psize, sizeof(void*));
	if (!q->pbuf) {
		e = QUEUE_ENOMEM;
		goto err;
	}
	have_pbuf = 1;
	if (mutex_init(&q->lock, 0)) {
		e = QUEUE_ENOMEM;
		goto err;
	}
	have_lock = 1;
	if (cond_init(&q->may_write)) {
		e = QUEUE_ENOMEM;
		goto err;
	}
	have_cond_write = 1;
	if (cond_init(&q->may_pwrite)) {
		e = QUEUE_ENOMEM;
		goto err;
	}
	have_cond_pwrite = 1;
	if (cond_init(&q->may_read)) {
		e = QUEUE_ENOMEM;
		goto err;
	}
	have_cond_read = 1;
	if (cond_init(&q->may_notify)) {
		e = QUEUE_ENOMEM;
		goto err;
	}

	*queue = q;
	goto out;
err:
	if (have_cond_write)
		cond_destroy(&q->may_write);
	if (have_cond_pwrite)
		cond_destroy(&q->may_pwrite);
	if (have_cond_read)
		cond_destroy(&q->may_read);
	if (have_lock)
		mutex_destroy(&q->lock);
	if (have_pbuf)
		free(q->pbuf);
	if (have_buf)
		free(q->buf);
	if (q)
		free(q);
	*queue = 0;
out:
	return e;
}

void queue_destroy(queue_t* queue)
{
	queue_i_t* q = (queue_i_t*)*queue;

	cond_destroy(&q->may_write);
	cond_destroy(&q->may_pwrite);
	cond_destroy(&q->may_read);
	cond_destroy(&q->may_notify);
	mutex_destroy(&q->lock);
	free(q->pbuf);
	free(q->buf);
	free(q);
	*queue = 0;
}

int queue_full(queue_t* queue)
{
	queue_i_t* q = (queue_i_t*)*queue;
	return (q->items == q->maxitems);
}

int queue_pfull(queue_t* queue)
{
	queue_i_t* q = (queue_i_t*)*queue;
	return (q->pitems == q->pmaxitems);
}

int queue_empty(queue_t* queue)
{
	queue_i_t* q = (queue_i_t*)*queue;
	return (q->items == 0);
}

int queue_pempty(queue_t* queue)
{
	queue_i_t* q = (queue_i_t*)*queue;
	return (q->pitems == 0);
}

static void advance_read_pointer(queue_i_t* q)
{
	q->read = (q->read + 1) % q->size;
}

static void advance_write_pointer(queue_i_t* q)
{
	q->write = (q->write + 1) % q->size;
}

static void advance_pread_pointer(queue_i_t* q)
{
	q->pread = (q->pread + 1) % q->psize;
}

static void advance_pwrite_pointer(queue_i_t* q)
{
	q->pwrite = (q->pwrite + 1) % q->psize;
}


void queue_write(queue_t* queue, void* item)
{
	queue_i_t* q = (queue_i_t*)*queue;

	mutex_lock(&q->lock);
	while (queue_full(queue)) {
		cond_wait(&q->may_write, &q->lock);
	}
	if (queue_empty(queue))
		cond_broadcast(&q->may_read);
	q->buf[q->write] = item;
	advance_write_pointer(q);
	q->items++;
	mutex_unlock(&q->lock);
}

void queue_pwrite(queue_t* queue, void* item)
{
	queue_i_t* q = (queue_i_t*)*queue;

	mutex_lock(&q->lock);
	while (queue_pfull(queue)) {
		cond_wait(&q->may_pwrite, &q->lock);
	}
	if (queue_pempty(queue))
		cond_broadcast(&q->may_read);
	q->pbuf[q->pwrite] = item;
	advance_pwrite_pointer(q);
	q->pitems++;
	mutex_unlock(&q->lock);
}

void queue_notify(queue_t* queue, void* item)
{
	queue_i_t* q = (queue_i_t*)*queue;

	mutex_lock(&q->lock);
	while (q->notified) {
		cond_wait(&q->may_notify, &q->lock);
	}
	q->notify = item;
	q->notified = 1;
	cond_broadcast(&q->may_read);
	mutex_unlock(&q->lock);
}

void queue_read(queue_t* queue, void** item)
{
	queue_i_t* q = (queue_i_t*)*queue;

	mutex_lock(&q->lock);
	while (queue_empty(queue) && queue_pempty(queue)
	&& !q->notified) {
		cond_wait(&q->may_read, &q->lock);
	}

	if (q->notified) {
		*item = q->notify;
		q->notified = 0;
		cond_broadcast(&q->may_notify);
		mutex_unlock(&q->lock);
		return;
	}
	if (!queue_pempty(queue)) {
		if (queue_pfull(queue))
			cond_broadcast(&q->may_pwrite);
		*item = q->pbuf[q->pread];
		advance_pread_pointer(q);
		q->pitems--;
		mutex_unlock(&q->lock);
		return;
	}
	if (!queue_empty(queue)) {
		if (queue_full(queue))
			cond_broadcast(&q->may_write);
		*item = q->buf[q->read];
		advance_read_pointer(q);
		q->items--;
		mutex_unlock(&q->lock);
		return;
	}
}

int queue_try_read(queue_t* queue, void** item)
{
	queue_i_t* q = (queue_i_t*)*queue;

	mutex_lock(&q->lock);
	if (queue_empty(queue) && queue_pempty(queue)
	&& !q->notify) {
		mutex_unlock(&q->lock);
		return -1;
	}

	if (q->notify) {
		*item = q->notify;
		q->notify = 0;
		cond_broadcast(&q->may_notify);
		mutex_unlock(&q->lock);
		return 0;
	}
	if (!queue_pempty(queue)) {
		if (queue_pfull(queue))
			cond_broadcast(&q->may_pwrite);
		*item = q->pbuf[q->pread];
		advance_pread_pointer(q);
		q->pitems--;
		mutex_unlock(&q->lock);
		return 0;
	}
	if (!queue_empty(queue)) {
		if (queue_full(queue))
			cond_broadcast(&q->may_write);
		*item = q->buf[q->read];
		advance_read_pointer(q);
		q->items--;
		mutex_unlock(&q->lock);
		return 0;
	}
	return 0;
}
