/*
 * libk - noop.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <string.h>
#include "bcmode_desc.h"

static void noop_encrypt
(struct k_bc_t* m, const void* i, void* o, size_t worker)
{
	memmove(o, i, m->blockcipher->block_size);
}


static const char* const authors[] = {
	"Janos Laube <janos.dev@gmail.com>",
	0
};

blockciphermode_start(NOOP, "No Operation (copy data)")
	.authors		= authors,
	.encrypt_parallelizable	= 1,
	.decrypt_parallelizable	= 1,
	.encrypt		= &noop_encrypt,
	.decrypt		= &noop_encrypt,
blockciphermode_end
