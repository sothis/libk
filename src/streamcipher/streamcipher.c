/*
 * libk - streamcipher.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <string.h>
#include "streamcipher_desc.h"
#include "utils/mem.h"
#include "utils/err.h"

section_prologue(__streamciphers, struct streamcipher_desc);

static inline const struct streamcipher_desc* sc_get_by_id
(enum streamcipher_e id)
{
	foreach_section_item(struct streamcipher_desc, streamciphers,
	__streamciphers) {
		if (id == streamciphers[i].id)
			return &streamciphers[i];
	}
	return NULL;
}

__export_function struct k_sc_t* k_sc_init
(enum streamcipher_e cipher)
{
	enum k_error_e err = K_ESUCCESS;
	struct k_sc_t* c = 0;

	c = k_calloc(1, sizeof(struct k_sc_t));
	if (!c) {
		err = K_ENOMEM;
		goto k_sc_init_err;
	}
	if (cipher == STREAM_CIPHER_NOOP)
		return c;
	c->cipher = sc_get_by_id(cipher);
	if (!c->cipher) {
		err = K_ENOCIPHER;
		goto k_sc_init_err;
	}
	if (!c->cipher->context_size) {
		err = K_ECTXZERO;
		goto k_sc_init_err;
	}
	c->ctx = k_locked_calloc(1, c->cipher->context_size,
		&c->alloced_ctxsize);
	if (!c->ctx) {
		err = K_ENOMEM;
		goto k_sc_init_err;
	}

	return c;

k_sc_init_err:
	k_sc_finish(c);
	k_error(err);
	return NULL;
}

__export_function struct k_sc_t* k_sc_init_with_blockcipher
(enum blockcipher_e cipher, enum bcstreammode_e mode, size_t max_workers)
{
	struct k_sc_t* c = 0;

	c = k_sc_init(STREAM_CIPHER_NOOP);
	if (!c)
		return NULL;
	c->blockcipher = k_bc_init(cipher);
	if (!c->blockcipher) {
		k_sc_finish(c);
		return NULL;
	}
	if (k_bcmode_set_mode(c->blockcipher, mode, max_workers)) {
		k_sc_finish(c);
		return NULL;
	}

	return c;
}

__export_function void k_sc_finish
(struct k_sc_t* c)
{
	if (c) {
		if (c->blockcipher)
			k_bc_finish(c->blockcipher);
		if (c->ctx)
			k_locked_free(c->ctx, c->alloced_ctxsize);
		k_free(c);
	}
}

__export_function int32_t k_sc_set_key
(struct k_sc_t* c, const void* key, uint32_t keybits)
{
	if (c->cipher) {
		if (c->cipher->insecure)
			k_warn(K_EINSECUREENC);
		c->cipher->init(c->ctx, key, keybits);
	}
	else return k_bcmode_set_key(c->blockcipher, key, keybits,
			BLK_CIPHER_KEY_ENCRYPT);
	return 0;
}

__export_function void k_sc_set_nonce
(struct k_sc_t* c, const void* nonce)
{
	if (c->blockcipher)
		k_bcmode_set_iv(c->blockcipher, nonce);
	// TODO: introduce nonce infrastructure for pure stream ciphers
	// in order to be able to construct custom cryptosystems
}

__export_function void k_sc_update
(struct k_sc_t* c, const void* input, void* output, size_t bytes)
{
	if (c->cipher)
		return c->cipher->update(c->ctx, input, output, bytes);
	else {
		size_t bs = k_bc_get_blocksize(c->blockcipher);
		size_t rem = bytes % bs;
		k_bcmode_update(c->blockcipher, input, output, bytes/bs);
		if (rem) {
			uint8_t last_block[bs];
			uint8_t last_block_out[bs];
			memset(last_block, 0, bs);
			memset(last_block_out, 0, bs);
			memcpy(last_block, input+bytes-rem, rem);
			k_bcmode_update(c->blockcipher, last_block,
				last_block_out, 1);
			memcpy(output+bytes-rem, last_block_out, rem);
		}
	}
}
