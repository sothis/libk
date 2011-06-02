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
	size_t bs = k_bc_get_blocksize(c->blockcipher);
	/* TODO: lock this memory */
	c->partial_block = k_calloc(1, bs);
	if (!c->partial_block) {
		k_sc_finish(c);
		k_error(K_ENOMEM);
		return NULL;
	}
	/* TODO: lock this memory */
	c->old_iv = k_calloc(1, bs);
	if (!c->old_iv) {
		k_sc_finish(c);
		k_error(K_ENOMEM);
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
		if (c->partial_block)
			k_free(c->partial_block);
		if (c->old_iv)
			k_free(c->old_iv);
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
		/* TODO: test for valid keysize here */
		c->cipher->init(c->ctx, key, keybits);
	}
	else return k_bcmode_set_key(c->blockcipher, key, keybits,
			BLK_CIPHER_KEY_ENCRYPT);
	return 0;
}

__export_function void k_sc_set_nonce
(struct k_sc_t* c, const void* nonce)
{
	if (c->blockcipher) {
		c->have_partial_block = 0;
		k_bcmode_set_iv(c->blockcipher, nonce);
	}
	// TODO: introduce nonce infrastructure for pure stream ciphers
	// in order to be able to construct custom cryptosystems
}

__export_function size_t k_sc_get_nonce_bytes
(struct k_sc_t* c)
{
	if (c->blockcipher)
		return k_bc_get_blocksize(c->blockcipher);
	// TODO: introduce nonce infrastructure for pure stream ciphers
	// in order to be able to construct custom cryptosystems
	return 0;
}

__export_function void k_sc_update
(struct k_sc_t* c, const void* input, void* output, size_t bytes)
{
	if (c->cipher)
		return c->cipher->update(c->ctx, input, output, bytes);
	else {
		size_t bs = k_bc_get_blocksize(c->blockcipher);

		if (c->have_partial_block) {
			size_t b =
				(bytes > c->partial_remaining) ?
				c->partial_remaining :
				bytes;

			uint8_t first_block_out[bs];
			memset(first_block_out, 0, bs);
			memcpy(c->partial_block+c->partial_bytes, input, b);

			c->partial_remaining -= b;

			if (c->partial_remaining) {
				const void* oiv = k_bcmode_get_iv(c->blockcipher);
				memcpy(c->old_iv, oiv, bs);
			}

			k_bcmode_update(c->blockcipher, c->partial_block,
				first_block_out, 1);

			if (c->partial_remaining) {
				k_bcmode_set_iv(c->blockcipher, c->old_iv);
			}

			memcpy(output, first_block_out+c->partial_bytes, b);
			c->partial_bytes += b;

			bytes -= b;
			input += b;
			output += b;

			if (!c->partial_remaining) {
				c->partial_bytes = 0;
				c->have_partial_block = 0;
			}
		}

		size_t rem = bytes % bs;
		k_bcmode_update(c->blockcipher, input, output, bytes/bs);
		if (rem) {
			uint8_t last_block_out[bs];

			/* backup old iv */
			const void* oiv = k_bcmode_get_iv(c->blockcipher);
			memcpy(c->old_iv, oiv, bs);

			memset(c->partial_block, 0, bs);
			memset(last_block_out, 0, bs);
			memcpy(c->partial_block, input+bytes-rem, rem);
			k_bcmode_update(c->blockcipher, c->partial_block,
				last_block_out, 1);
			memcpy(output+bytes-rem, last_block_out, rem);
			c->partial_bytes = rem;
			c->partial_remaining = bs - rem;
			c->have_partial_block = 1;

			/* restore old iv */
			k_bcmode_set_iv(c->blockcipher, c->old_iv);
		}
	}
}
