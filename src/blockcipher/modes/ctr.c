/*
 * libk - ctr.c
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
#include "utils/endian-neutral.h"

static void ctr_crypt
(struct k_bc_t* m, const void* i, void* o, size_t worker)
{
	size_t bs = m->blockcipher->block_size;
	uint8_t b[bs];
	uint8_t* iv = m->worker_ivs[worker];
	uint8_t* iv_ctr = iv + bs - sizeof(uint64_t);
	uint64_t ctr = _get_uint64_l(iv_ctr);

	memcpy(b, i, bs);

	m->blockcipher->encrypt(m->schedule, iv, o);
	xorb_64(o, o, b, bs);
	ctr++;
	_put_uint64_l(iv_ctr, ctr);
}

static void ctr_prepare_worker_ivs
(struct k_bc_t* m, struct mode_param_t pa[], size_t worker)
{
	size_t bs = m->blockcipher->block_size;
	size_t w = 0;
	uint64_t ctr = 0;
	uint8_t* iv = m->worker_ivs[0];
	uint8_t* iv_ctr = iv + bs - sizeof(uint64_t);

	ctr = _get_uint64_l(iv_ctr);

	while (++w < worker) {
		uint8_t* wiv = m->worker_ivs[w] + bs - sizeof(uint64_t);
		memcpy(m->worker_ivs[w], iv, bs);
		ctr += pa[w-1].nblocks;
		_put_uint64_l(wiv, ctr);
	}
}

static const char* const authors[] = {
	"Janos Laube <janos.dev@gmail.com>",
	0
};

blockciphermode_start(CTR, "Counter")
	.authors			= authors,
	.decrypt_uses_encrypt_transform	= 1,
	.encrypt_parallelizable		= 1,
	.decrypt_parallelizable		= 1,
	.prepare_iv			= &ctr_prepare_worker_ivs,
	.encrypt			= &ctr_crypt,
	.decrypt			= &ctr_crypt,
blockciphermode_end
