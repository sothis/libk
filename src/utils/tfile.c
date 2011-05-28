/*
 * libk - tfile.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

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

#if defined(__DARWIN__)
#define O_NOATIME	0
#elif defined(__WINNT__)
#define O_NOATIME	_O_BINARY
#endif

#include <libgen.h>

static volatile sig_atomic_t tfile_init = 0;
static struct mempool_t mappool;
static const char* template = ".tfile_XXXXXX";

/* just prepend the cwd */
static char* absfilename(const char* name)
{
	size_t wds, ns;
	char* p = 0;
	char* cwd = 0;

	ns = strlen(name);
	if (!ns)
		goto err_out;

	cwd = getcwd(0, 0);
	if (!cwd)
		goto err_out;
	wds = strlen(cwd);
	if (!wds)
		goto err_out;

	p = calloc(wds+ns+3, sizeof(char*));
	if (!p)
		goto err_out;

	if (name[0] == '/') {
		memcpy(p, name, ns);
	}
#ifdef __WINNT__
	else if((strlen(name) > 2) && name[1] == ':' && name[2] == '\\') {
		memcpy(p, name, ns);
	}
#endif
	else {
		memcpy(p, cwd, wds);
		p[wds] = '/';
		memcpy(p+wds+1, name, ns);
	}

	free(cwd);
	return p;
err_out:
	if (p)
		free(p);
	if (cwd)
		free(cwd);
	return 0;

}


struct tfile_t {
	int fd;
	char* filename;
	char* tmpfilename;
	mode_t mode;
};

static void trollback_one(struct tfile_t* tf, size_t off)
{
	close(tf->fd);
	#ifdef __WINNT__
	wchar_t* wtf = utf8_to_ucs2(tf->tmpfilename);
	if (wtf) {
		_wunlink(wtf);
		free(wtf);
	}
	#else
	unlink(tf->tmpfilename);
	#endif
	free(tf->filename);
	free(tf->tmpfilename);
	pool_cut(&mappool, off, sizeof(struct tfile_t));
}

static int tcommit_one(struct tfile_t* tf, size_t off)
{
	#ifdef __WINNT__
	wchar_t* wtf = 0;
	wchar_t* wf = 0;
	#endif
	int old_errno;

	if (fchmod(tf->fd, tf->mode))
		goto err;

	/* TODO: make this optional */
	if (fsync(tf->fd))
		goto err;

	if (close(tf->fd))
		goto err;
	/* TODO: rename is broken on windows. it doesn't allow to replace
	 * exising files atomically. see google for existing implementations
	 * and replace rename() for windows targets with a suitable one. */
#ifdef __WINNT__
	wtf = utf8_to_ucs2(tf->tmpfilename);
	wf = utf8_to_ucs2(tf->filename);
	if (!wtf || !wf)
		goto err;
	if (_wrename(wtf, wf))
		goto err;
	free(wtf);
	free(wf);
#else
	if (rename(tf->tmpfilename, tf->filename))
		goto err;
#endif
	free(tf->filename);
	free(tf->tmpfilename);
	pool_cut(&mappool, off, sizeof(struct tfile_t));
	return 0;
err:
	#ifdef __WINNT__
	if (wtf)
		free(wtf);
	if (wf)
		free(wf);
	#endif
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

__export_function int k_tcreat(const char* name, mode_t mode)
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

	namelen = strlen(name);
	if (!namelen) {
		errno = EINVAL;
		goto err;
	}

	tf.filename = absfilename(name);
	if (!tf.filename)
		goto err;

	/* check if name is valid and if we have read-write permissions in
	 * the case the file exists. ENOENT is not fatal, since we're about
	 * to create the file anyway. */
	#ifdef __WINNT__
	int fd;
	wchar_t* wc = utf8_to_ucs2(tf.filename);
	if (!wc)
		fd = -1;
	else {
		fd = _wopen(wc, O_RDWR);
		free(wc);
	}
	#else
	int fd = open(tf.filename, O_RDWR|O_NOATIME);
	#endif
	if (fd == -1 && errno != ENOENT)
		goto err;
	if (fd != -1)
		if (close(fd))
			goto err;

	tf.tmpfilename = calloc(strlen(tf.filename)+strlen(template)+2, 1);
	if (!tf.tmpfilename)
		goto err;

	strcpy(tf.tmpfilename, tf.filename);
	char* dir = dirname(tf.tmpfilename);

	sprintf(tf.tmpfilename, "%s/%s", dir, template);

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
		close(tf.fd);
		#ifdef __WINNT__
		wchar_t* wtf = utf8_to_ucs2(tf.tmpfilename);
		if (wtf) {
			_wunlink(wtf);
			free(wtf);
		}
		#else
		unlink(tf.tmpfilename);
		#endif
	}
	if (tf.filename)
		free(tf.filename);
	if (tf.tmpfilename)
		free(tf.tmpfilename);
out:
	umask(old_umask);
	return res;
}

__export_function int k_tcommit_and_close(int fd)
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

__export_function void k_trollback_and_close(int fd)
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
