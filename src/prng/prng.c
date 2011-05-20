#include <string.h>
#include "prng_desc.h"
#include "utils/endian-neutral.h"
#include "utils/mem.h"
#include "utils/err.h"

section_prologue(__prngs, struct prng_desc);

static inline const struct prng_desc* prng_get_by_id
(enum prng_e id)
{
	foreach_section_item(struct prng_desc, prngs,
	__prngs) {
		if (id == prngs[i].id)
			return &prngs[i];
	}
	return NULL;
}

__export_function struct k_prng_t* k_prng_init
(enum prng_e prng)
{
	enum k_error_e err = K_ESUCCESS;
	struct k_prng_t* c = 0;

	c = k_calloc(1, sizeof(struct k_prng_t));
	if (!c) {
		err = K_ENOMEM;
		goto k_prng_init_err;
	}
	c->prng = prng_get_by_id(prng);
	if (!c->prng) {
		err = K_ENOCIPHER;
		goto k_prng_init_err;
	}
	if (!c->prng->context_size) {
		err = K_ECTXZERO;
		goto k_prng_init_err;
	}
	c->ctx = k_locked_calloc(1, c->prng->context_size,
		&c->alloced_ctxsize);
	if (!c->ctx) {
		err = K_ENOMEM;
		goto k_prng_init_err;
	}

	return c;

k_prng_init_err:
	k_prng_finish(c);
	k_error(err);
	return NULL;
}

__export_function void k_prng_finish
(struct k_prng_t* c)
{
	if (c) {
		if (c->ctx)
			k_locked_free(c->ctx, c->alloced_ctxsize);
		k_free(c);
	}
}

__export_function void k_prng_set_seed
(struct k_prng_t* c, const void* seed, size_t seed_bytes)
{
	c->prng->init(c->ctx, seed, seed_bytes);
}

__export_function void k_prng_update
(struct k_prng_t* c, void* output, size_t bytes)
{
	size_t word_bytes = c->prng->word_size / 8;
	size_t i = bytes / word_bytes;
	size_t rem = bytes % word_bytes;
	for (size_t l = 0; l < i; ++l)
		c->prng->update(c->ctx, output + (l*word_bytes));
	if (rem) {
		uint8_t last_block[word_bytes];
		memset(last_block, 0, word_bytes);
		c->prng->update(c->ctx, last_block);
		memcpy(output+bytes-rem, last_block, rem);
	}
}

__export_function uint8_t k_prng_get_uint8
(struct k_prng_t* c)
{
	uint8_t res;
	k_prng_update(c, &res, sizeof(res));
	return res;
}

__export_function uint16_t k_prng_get_uint16
(struct k_prng_t* c)
{
	uint16_t res;
	k_prng_update(c, &res, sizeof(res));
	return _get_uint16_l(&res);
}

__export_function uint32_t k_prng_get_uint32
(struct k_prng_t* c)
{
	uint32_t res;
	k_prng_update(c, &res, sizeof(res));
	return _get_uint32_l(&res);
}

__export_function uint64_t k_prng_get_uint64
(struct k_prng_t* c)
{
	uint64_t res;
	k_prng_update(c, &res, sizeof(res));
	return _get_uint64_l(&res);
}
