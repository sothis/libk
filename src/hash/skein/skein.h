/*
 * libk - skein.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _SKEIN_H
#define _SKEIN_H

#include <stdint.h>
#include <stddef.h>
#include <libk/libk.h>
#include "../hash_desc.h"
#include "utils/unittest_desc.h"


enum skein_statesize_e {
	skein256 = 32,
	skein512 = 64,
	skein1024 = 128,
};

struct skein_t
{
	k_bc_t*	 			cipher;
	uint64_t			tweak[2];
	uint64_t			state[16];
	union {
		uint8_t			partial8[128];
		uint64_t		partial64[16];
	} partial;
	size_t				state_bytes;
	size_t				partial_count;
	uint32_t			outputsize; /* in bits */
};

void skein_init(
	struct skein_t* ctx,
	size_t state_size,
	uint32_t hash_outputsize
);

void skein_update(
	struct skein_t*	ctx,
	const void*	message,
	size_t		message_size
);

void skein_final(
	struct skein_t*	ctx,
	void*		digest
);

void skein_finish(
	struct skein_t* ctx
);


#endif /* _SKEIN_H */
