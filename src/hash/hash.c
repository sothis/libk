/*
 * libk - hash.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <string.h>
#include "hash_desc.h"
#include "utils/mem.h"
#include "utils/err.h"

section_prologue(__hashsums, struct hash_desc);

static inline const struct hash_desc* hash_get_by_id
(enum hashsum_e id)
{
	foreach_section_item(struct hash_desc, hashsums, __hashsums) {
		if (id == hashsums[i].id)
			return &hashsums[i];
	}
	return NULL;
}

__export_function struct k_hash_t* k_hash_init
(enum hashsum_e hashsum, uint32_t output_bits)
{
	enum k_error_e err = K_ESUCCESS;
	struct k_hash_t* c = 0;

	c = k_calloc(1, sizeof(struct k_hash_t));
	if (!c) {
		err = K_ENOMEM;
		goto k_hash_init_err;
	}
	c->hash = hash_get_by_id(hashsum);
	if (!c->hash) {
		err = K_ENOHASH;
		goto k_hash_init_err;
	}
	if (!c->hash->context_size) {
		err = K_ECTXZERO;
		goto k_hash_init_err;
	}
	c->context = k_locked_calloc(1, c->hash->context_size,
		&c->alloced_ctxsize);
	if (!c->context) {
		err = K_ENOMEM;
		goto k_hash_init_err;
	}
	if (output_bits)
		c->output_bits = output_bits;
	else
		c->output_bits = c->hash->state_size;

	k_hash_reset(c);

	if (c->hash->insecure) {
		k_warn(K_EINSECUREHASH);
	}

	return c;

k_hash_init_err:
	k_hash_finish(c);
	k_error(err);
	return NULL;
}

__export_function void k_hash_finish
(struct k_hash_t* c)
{
	if (c) {
		if (c->hash->finish && c->context)
			c->hash->finish(c->context);
		if (c->context)
			k_locked_free(c->context, c->alloced_ctxsize);
		k_free(c);
	}
}

__export_function void k_hash_reset
(struct k_hash_t* c)
{
	c->hash->init(c->context, c->output_bits);
}

__export_function void k_hash_update
(struct k_hash_t* c, const void* input, size_t bytes)
{
	c->hash->update(c->context, input, bytes, c->output_bits);
}

__export_function void k_hash_final
(struct k_hash_t* c, void* output)
{
	memset(output, 0, (c->output_bits + 7) / 8);
	c->hash->final(c->context, output, c->output_bits);
}

__export_function uint32_t k_hash_digest_size
(struct k_hash_t* c)
{
	return c->output_bits;
}
