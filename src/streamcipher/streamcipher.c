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
#include "utils/xor.h"

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
(enum streamcipher_e cipher, uint32_t noncebits)
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

	if (!noncebits) {
		// set err
		goto k_sc_init_err;
	}
	// check valid noncesize for cipher here
	c->noncesize = noncebits;

	return c;

k_sc_init_err:
	k_sc_finish(c);
	k_error(err);
	return NULL;
}

__export_function struct k_sc_t* k_sc_init_with_blockcipher
(enum blockcipher_e cipher, enum bcmode_e mode, size_t max_workers)
{
	enum k_error_e err = K_ESUCCESS;
	struct k_sc_t* c = 0;

	int32_t r = k_bcmode_produces_keystream(mode);
	if (r < 0)
		goto k_sc_init_err;
	if (!r) {
		err = K_EINVMODE;
		goto k_sc_init_err;
	}

	c = k_sc_init(STREAM_CIPHER_NOOP, 0);
	if (!c)
		return NULL;
	c->blockcipher = k_bc_init(cipher);
	if (!c->blockcipher)
		goto k_sc_init_err;

	size_t bs = k_bc_get_blocksize(c->blockcipher);
	c->noncesize = bs*8;

	c->partial_block = k_calloc(1, bs);
	if (!c->partial_block) {
		err = K_ENOMEM;
		goto k_sc_init_err;
	}

	if (k_bcmode_set_mode(c->blockcipher, (enum bcmode_e)mode,
	max_workers))
		goto k_sc_init_err;

	return c;

k_sc_init_err:
	k_sc_finish(c);
	if (err != K_ESUCCESS)
		k_error(err);
	return NULL;
}

__export_function void k_sc_finish
(struct k_sc_t* c)
{
	if (c) {
		if (c->blockcipher)
			k_bc_finish(c->blockcipher);
		if (c->partial_block)
			k_free(c->partial_block);
		if (c->ctx)
			k_locked_free(c->ctx, c->alloced_ctxsize);
		k_free(c);
	}
}

__export_function int32_t k_sc_set_key
(struct k_sc_t* c, const void* nonce, const void* key, uint32_t keybits)
{
	if (c->cipher) {
		if (c->cipher->insecure)
			k_warn(K_EINSECUREENC);
		if (key && (keybits > c->noncesize)) {
			// err here
			return -1;
		}
		size_t noncebytes = (c->noncesize+7)/8;
		void* k = k_calloc(noncebytes, 1);
		memcpy(k, nonce, noncebytes);
		if (key)
			xorb_64(k, k, key, (keybits+7)/8);

		c->cipher->init(c->ctx, k, c->noncesize);
		k_free(k);
	}
	else {
		if (key && k_bcmode_set_key(c->blockcipher, key, keybits,
			BLK_CIPHER_KEY_ENCRYPT))
			return -1;
		k_bcmode_set_iv(c->blockcipher, nonce);
		c->have_partial_block = 0;
	}
	return 0;
}

__export_function size_t k_sc_get_nonce_bytes
(struct k_sc_t* c)
{
	return (c->noncesize+7) / 8;
}

__export_function void k_sc_update
(struct k_sc_t* c, const void* input, void* output, size_t bytes)
{
	if (c->cipher) {
		c->cipher->update(c->ctx, input, output, bytes);
		return;
	} else {
		size_t bs = k_bc_get_blocksize(c->blockcipher);

		if (c->have_partial_block) {
			size_t b =
				(bytes > c->partial_remaining) ?
				c->partial_remaining :
				bytes;

			uint8_t first_block_out[bs];
			memset(first_block_out, 0, bs);
			if (input)
				memcpy(c->partial_block+c->partial_bytes,
					input, b);

			c->partial_remaining -= b;

			if (c->partial_remaining)
				k_bcmode_backup_iv(c->blockcipher);

			k_bcmode_update(c->blockcipher, input ?
				c->partial_block : 0, first_block_out, 1);

			if (c->partial_remaining)
				k_bcmode_restore_iv(c->blockcipher);

			memcpy(output, first_block_out+c->partial_bytes, b);
			c->partial_bytes += b;

			bytes -= b;
			if (input)
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

			k_bcmode_backup_iv(c->blockcipher);

			memset(c->partial_block, 0, bs);
			memset(last_block_out, 0, bs);
			if (input)
				memcpy(c->partial_block, input+bytes-rem, rem);
			k_bcmode_update(c->blockcipher, input ?
				c->partial_block : 0, last_block_out, 1);
			memcpy(output+bytes-rem, last_block_out, rem);
			c->partial_bytes = rem;
			c->partial_remaining = bs - rem;
			c->have_partial_block = 1;

			k_bcmode_restore_iv(c->blockcipher);
		}
	}
}

__export_function void k_sc_keystream
(struct k_sc_t* c, void* output, size_t bytes)
{
	k_sc_update(c, 0, output, bytes);
}
