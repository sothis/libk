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

struct platform_t {
	int	fd_random;
	int	fd_urandom;
};

static void platform_setfds(void* state, int fd_random, int fd_urandom)
{
	struct platform_t* c = state;
	c->fd_random = fd_random;
	c->fd_urandom = fd_urandom;
}

static void platform_update(void* state, void* output)
{
	struct platform_t* c = state;
	uint32_t r = 0;
#ifdef __WINNT__
	rand_s(&r);
#else
	size_t total = 0;
	ssize_t nread;
	while ((nread = read(c->fd_urandom, &r, sizeof(uint32_t))) > 0) {
		total += nread;
		if (total == sizeof(uint32_t))
			break;
	}
#endif
	_put_uint32_l(output, r);
}


static const char* const authors[] = {
	"Janos Laube <janos.dev@gmail.com>",
	0
};

prng_start(PLATFORM, "Platform Specific Prng")
	.authors		= authors,
	.word_size		= 32,
	.context_size		= sizeof(struct platform_t),
	.setfds			= &platform_setfds,
	.update			= &platform_update,
prng_end
