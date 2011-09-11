#include <signal.h>
#include <stdlib.h>

#include <libk/libk.h>
#include "utils/sections.h"
#include "utils/err.h"
#include "utils/mem.h"

static volatile sig_atomic_t filedesc_init = 0;
static struct mempool_t descriptors;

static void __cleanup(void)
{
	// close open files here (in order to enable automatic rollback)
	pool_free(&descriptors);
}

static void __init(void)
{
	atexit(__cleanup);
	/* realloc every 1024 files */
	pool_alloc(&descriptors, 1024*(sizeof(struct pres_file_t)));
	filedesc_init = 1;
}

struct pres_file_t* get_descriptor(int fd)
{
	struct pres_file_t* files = pool_getmem(&descriptors, 0);
	size_t n = pool_getsize(&descriptors) / sizeof(struct pres_file_t);

	for (size_t i = 0; i < n; ++i) {
		if (fd == files[i].fd)
			return (&files[i]);
	}
	return 0;
}

void remove_descriptor(int fd)
{
	struct pres_file_t* files = pool_getmem(&descriptors, 0);
	size_t n = pool_getsize(&descriptors) / sizeof(struct pres_file_t);

	for (size_t i = 0; i < n; ++i) {
		if (fd == files[i].fd) {
			pool_cut(&descriptors, i*sizeof(struct pres_file_t),
				sizeof(struct pres_file_t));
			return;
		}
	}
	return;
}

int add_descriptor(struct pres_file_t* f)
{
	kerrno = K_ESUCCESS;

	if (!filedesc_init)
		__init();

	if (pool_append(&descriptors, f, sizeof(*f))) {
		kerrno = errno;
		goto out;
	}

out:
	return -kerrno;
}


