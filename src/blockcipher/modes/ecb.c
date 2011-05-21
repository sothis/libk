/*
 * libk - ecb.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include "bcmode_desc.h"


static void ecb_encrypt
(struct k_bc_t* m, const void* i, void* o, size_t worker)
{
	m->blockcipher->encrypt(m->schedule, i, o);
}

static void ecb_decrypt
(struct k_bc_t* m, const void* i, void* o, size_t worker)
{
	m->blockcipher->decrypt(m->schedule, i, o);
}


static const char* const authors[] = {
	"Janos Laube <janos.dev@gmail.com>",
	0
};

blockciphermode_start(ECB, "Electronic Code Book")
	.authors		= authors,
	.encrypt_parallelizable	= 1,
	.decrypt_parallelizable	= 1,
	.encrypt		= &ecb_encrypt,
	.decrypt		= &ecb_decrypt,
blockciphermode_end
