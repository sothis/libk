/*
 * cfb.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <string.h>
#include "bcmode_desc.h"
#include "utils/xor.h"

static void cfb_encrypt
(struct k_bc_t* m, const void* i, void* o, size_t worker)
{
	size_t bs = m->blockcipher->block_size;
	uint8_t* iv = m->worker_ivs[0];
	uint8_t b[bs];

	memcpy(b, i, bs);

	m->blockcipher->encrypt(m->schedule, iv, o);
	xorb_64(o, o, b, bs);
	memcpy(iv, o, bs);
}

static void cfb_decrypt
(struct k_bc_t* m, const void* i, void* o, size_t worker)
{
	size_t bs = m->blockcipher->block_size;
	uint8_t* iv = m->worker_ivs[worker];
	uint8_t b[bs];

	memcpy(b, i, bs);

	m->blockcipher->encrypt(m->schedule, iv, o);
	xorb_64(o, o, b, bs);
	memcpy(iv, b, bs);
}

static void cfb_prepare_worker_ivs
(struct k_bc_t* m, struct mode_param_t pa[], size_t worker)
{
	size_t bs = m->blockcipher->block_size;
	size_t w = 0;

	while (++w < worker)
		memcpy(m->worker_ivs[w], pa[w].in - bs, bs);
}

static const char* const authors[] = {
	"Janos Laube <janos.dev@gmail.com>",
	0
};

blockciphermode_start(CFB, "Cipher Feedback")
	.authors			= authors,
	.decrypt_uses_encrypt_transform	= 1,
	.decrypt_parallelizable		= 1,
	.prepare_iv			= &cfb_prepare_worker_ivs,
	.encrypt			= &cfb_encrypt,
	.decrypt			= &cfb_decrypt,
blockciphermode_end
