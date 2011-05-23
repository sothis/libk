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
#include "prng_desc.h"
#include "utils/endian-neutral.h"
#include "utils/unittest_desc.h"

struct platform_t {
	uint32_t	dummy;
};

static void platform_update(void* state, void* output)
{
	uint32_t r = 0;
#ifdef __WINNT__
	rand_s(&r);
#else
	int fd = open("/dev/urandom", O_RDONLY);
	read(fd, &r, sizeof(uint32_t));
	close(fd);
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
	.update			= &platform_update,
prng_end
