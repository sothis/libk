/*
 * libk - tfile.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

/* NOTE: in order for this to work properly, the process must not change
 * it's current working directory via chdir() and friends while still having
 * tfiles open */

#include <libk/libk.h>
#include "utils/sections.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef __WINNT__
#include "ntwrap.h"
#endif

#if defined(__DARWIN__) || defined(__WINNT__)
#define O_NOATIME		0
#endif

static volatile sig_atomic_t tfile_init = 0;
static struct mempool_t mappool;
static const char* template = ".tfile_XXXXXX";

struct tfile_t {
	int fd;
	char* filename;
	char* tmpfilename;
	mode_t mode;
};

static void trollback_one(struct tfile_t* tf, size_t off)
{
	unlink(tf->tmpfilename);
	close(tf->fd);
	free(tf->filename);
	free(tf->tmpfilename);
	pool_cut(&mappool, off, sizeof(struct tfile_t));
}

static int tcommit_one(struct tfile_t* tf, size_t off)
{
	int old_errno;

	if (fchmod(tf->fd, tf->mode))
		goto err;
	if (fsync(tf->fd))
		goto err;
	if (close(tf->fd))
		goto err;
	/* TODO: rename is broken on windows. it doesn't allow to replace
	 * exising files atomically. see google for existing implementations
	 * and replace rename() for windows targets with a suitable one. */
	if (rename(tf->tmpfilename, tf->filename))
		goto err;

	free(tf->filename);
	free(tf->tmpfilename);
	pool_cut(&mappool, off, sizeof(struct tfile_t));
	return 0;
err:
	old_errno = errno;
	trollback_one(tf, off);
	errno = old_errno;
	return -1;
}

static void __cleanup(void)
{
	struct tfile_t* openfiles = pool_getmem(&mappool, 0);
	size_t nfiles = pool_getsize(&mappool) / sizeof(struct tfile_t);

	for (size_t i = 0; i < nfiles; ++i) {
		trollback_one(&openfiles[i], i*sizeof(struct tfile_t));
	}
	pool_free(&mappool);
}

static int __init(void)
{
	if (atexit(__cleanup))
		return -1;
	pool_alloc(&mappool, 0);
	tfile_init = 1;
	return 0;
}

__export_function int tcreat(const char* name, mode_t mode)
{
	int res = -1;
	mode_t old_umask;
	size_t namelen;
	struct tfile_t tf;

	if (!tfile_init && __init())
		return -1;

	memset(&tf, 0, sizeof(struct tfile_t));
	tf.fd = -1;
	tf.mode = mode;
	old_umask = umask(077);

	/* check if name is valid and if we have read-write permissions in
	 * the case the file exists. ENOENT is not fatal, since we're about
	 * to create the file anyway. */
	int fd = open(name, O_RDWR|O_NOATIME);
	if (fd == -1 && errno != ENOENT)
		goto err;
	if (fd != -1)
		if (close(fd))
			goto err;

	namelen = strlen(name);
	if (!namelen) {
		errno = EINVAL;
		goto err;
	}

	tf.filename = calloc(namelen+1, sizeof(char));
	if (!tf.filename)
		goto err;
	memcpy(tf.filename, name, namelen);

	tf.tmpfilename = calloc(strlen(template)+1, sizeof(char));
	if (!tf.tmpfilename)
		goto err;
	sprintf(tf.tmpfilename, "%s", template);

	if ((tf.fd = mkstemp(tf.tmpfilename)) == -1)
		goto err;

	/* must be the last action, since if pool_append fails, the state
	 * of the pool remains untouched, so that we can return safely
	 * without modifying the pool in the case of an error */
	if (pool_append(&mappool, &tf, sizeof(struct tfile_t)))
		goto err;

	res = tf.fd;
	goto out;
err:
	if (tf.fd != -1) {
		unlink(tf.tmpfilename);
		close(tf.fd);
	}
	if (tf.filename)
		free(tf.filename);
	if (tf.tmpfilename)
		free(tf.tmpfilename);
out:
	umask(old_umask);
	return res;
}

__export_function int tcommit_and_close(int fd)
{
	if (!tfile_init)
		return 0;

	struct tfile_t* openfiles = pool_getmem(&mappool, 0);
	size_t nfiles = pool_getsize(&mappool) / sizeof(struct tfile_t);

	for (size_t i = 0; i < nfiles; ++i) {
		if (fd == openfiles[i].fd)
			return tcommit_one(&openfiles[i],
				i*sizeof(struct tfile_t));
	}
	return 0;
}

__export_function void trollback_and_close(int fd)
{
	if (!tfile_init)
		return;

	struct tfile_t* openfiles = pool_getmem(&mappool, 0);
	size_t nfiles = pool_getsize(&mappool) / sizeof(struct tfile_t);

	for (size_t i = 0; i < nfiles; ++i) {
		if (fd == openfiles[i].fd)
			return trollback_one(&openfiles[i],
				i*sizeof(struct tfile_t));
	}
}
