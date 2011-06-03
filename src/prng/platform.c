/*
 * libk - platform.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "prng_desc.h"
#include "utils/endian-neutral.h"
#include "utils/unittest_desc.h"

enum platform_characteristics_e {
	wordsize	= 32,
	cachesize	= 4096*((wordsize+7)/8),
};

struct platform_t {
	int		fd_random;
	int		fd_urandom;
	size_t		cache_fill;
	uint32_t	cache[4096];
};

static void platform_setfds(void* state, int fd_random, int fd_urandom)
{
	struct platform_t* c = state;
	c->fd_random = fd_random;
	c->fd_urandom = fd_urandom;
}

static void platform_update(void* state, void* output)
{
	uint32_t r = 0;
#ifdef __WINNT__
	/* TODO: cache here too */
	rand_s(&r);
#else
	struct platform_t* c = state;

	if (c->cache_fill) {
		c->cache_fill--;
		r = c->cache[c->cache_fill];
		_put_uint32_l(output, r);
		return;
	}

	size_t total = 0;
	ssize_t nread;
	void* cache = c->cache;
	while ((nread = read(c->fd_urandom, cache + total,
		cachesize - total)) > 0) {
		total += nread;
		if (total == cachesize)
			break;
	}
	c->cache_fill = 4095;
	r = c->cache[c->cache_fill];
#endif
	_put_uint32_l(output, r);
}


static const char* const authors[] = {
	"Janos Laube <janos.dev@gmail.com>",
	0
};

prng_start(PLATFORM, "Platform Specific Prng")
	.authors		= authors,
	.word_size		= wordsize,
	.context_size		= sizeof(struct platform_t),
	.setfds			= &platform_setfds,
	.update			= &platform_update,
prng_end
