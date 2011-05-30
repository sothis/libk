/*
 * libk - skein_common.c
 *
 * 2008, Doug Whiting <doug.whiting@exar.com>
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

/* This implementation is the straight hash algorithm only, based on Doug
 * Whiting's Skein reference implementation. */

#include <string.h>
#include "skein.h"
#include "utils/endian-neutral.h"

#define SKEIN_VERSION		1ull
#define SKEIN_ID_STRING		0x33414853ull
#define SKEIN_FLAG_FIRST	(1ull << 62)
#define SKEIN_FLAG_FINAL	(1ull << 63)
#define SKEIN_CFG		(4ull << 56)
#define SKEIN_MSG		(48ull << 56)
#define SKEIN_OUT		(63ull << 56)


static void
skein_compression
(struct skein_t* ctx, const void* blockstart, size_t n, uint64_t bytes_add)
{
	do {
		k_bc_set_encrypt_key(ctx->cipher, ctx->state,
			ctx->state_bytes*8);
		_put_uint64_l(ctx->tweak,
			_get_uint64_l(ctx->tweak) + bytes_add);
		k_bc_set_tweak(ctx->cipher, ctx->tweak, 16);
		k_bc_encrypt(ctx->cipher, blockstart, ctx->state);
		feed_forward64(blockstart, ctx->state, ctx->state_bytes/8);
		_put_uint64_l(ctx->tweak+8,
			_get_uint64_l(ctx->tweak+8) & ~SKEIN_FLAG_FIRST);
		blockstart += ctx->state_bytes;
	} while (--n);
}

void skein_init(
	struct skein_t* ctx,
	size_t state_size,
	uint32_t hash_outputsize)
{
	uint64_t ver, flags, cfg_block[16] = {0};

	k_bc_t* oldcipher = ctx->cipher;
	memset(ctx, 0, sizeof(struct skein_t));
	ctx->cipher = oldcipher;

	if (!oldcipher) {
		switch (state_size) {
			case skein1024:
				ctx->cipher =
					k_bc_init(BLK_CIPHER_THREEFISH_1024);
				break;
			case skein512:
				ctx->cipher =
					k_bc_init(BLK_CIPHER_THREEFISH_512);
				break;
			case skein256:
			default:
				ctx->cipher =
					k_bc_init(BLK_CIPHER_THREEFISH_256);
		}
	}

	ctx->outputsize = hash_outputsize;
	ctx->state_bytes = state_size;

	ver = (SKEIN_VERSION << 32) | SKEIN_ID_STRING;
	_put_uint64_l(cfg_block, ver);
	_put_uint64_l(cfg_block+8, (uint64_t)hash_outputsize);

	flags = SKEIN_FLAG_FIRST | SKEIN_CFG | SKEIN_FLAG_FINAL;
	_put_uint64_l(ctx->tweak+8, flags);
	ctx->partial_count = 0;
	skein_compression(ctx, cfg_block, 1, 32);

	flags = SKEIN_FLAG_FIRST | SKEIN_MSG;
	ctx->tweak[0] = 0;
	_put_uint64_l(ctx->tweak+8, flags);
	ctx->partial_count = 0;
}

void skein_update(
	struct skein_t* ctx,
	const void* message,
	size_t message_size)
{
	size_t n;

	if (message_size + ctx->partial_count > ctx->state_bytes) {
		if (ctx->partial_count) {
			n = ctx->state_bytes - ctx->partial_count;
			if (n) {
				memcpy(ctx->partial.partial8+ctx->partial_count,
					message, n);
				message_size -= n;
				message += n;
				ctx->partial_count += n;
			}
			skein_compression(ctx, ctx->partial.partial8, 1,
				ctx->state_bytes);
			ctx->partial_count = 0;
		}
		if (message_size > ctx->state_bytes) {
			n = (message_size-1) / ctx->state_bytes;
			skein_compression(ctx, message, n, ctx->state_bytes);
			message_size -= n * ctx->state_bytes;
			message += n * ctx->state_bytes;
		}
	}
	if (message_size) {
		memcpy(ctx->partial.partial8 + ctx->partial_count,
			message, message_size);
		ctx->partial_count += message_size;
	}
}

void skein_final(
	struct skein_t* ctx,
	void* digest)
{
	uint64_t i, flags;
	uint32_t remaining, digestsize;
	uint8_t final_key[128];

	_put_uint64_l(ctx->tweak+8,
		_get_uint64_l(ctx->tweak+8) | SKEIN_FLAG_FINAL);
	if (ctx->partial_count < ctx->state_bytes) {
		memset(ctx->partial.partial8 + ctx->partial_count, 0,
			ctx->state_bytes - ctx->partial_count);
	}
	skein_compression(ctx, ctx->partial.partial8, 1, ctx->partial_count);
	digestsize = (ctx->outputsize + 7) / 8;
	memset(ctx->partial.partial8, 0, ctx->state_bytes);
	ctx->partial_count = 0;
	memcpy(final_key, ctx->state, ctx->state_bytes);
	flags = SKEIN_FLAG_FIRST | SKEIN_OUT | SKEIN_FLAG_FINAL;

	for (i = 0; i * ctx->state_bytes < digestsize; i++) {
		ctx->tweak[0] = 0;
		_put_uint64_l(ctx->tweak+8, flags);
		_put_uint64_l(ctx->partial.partial64, i);
		skein_compression(ctx, ctx->partial.partial8, 1,
			sizeof(uint64_t));
		remaining = digestsize - i * ctx->state_bytes;
		if (remaining >= ctx->state_bytes)
			remaining = ctx->state_bytes;
		memcpy(digest + i * ctx->state_bytes, ctx->state, remaining);
		memcpy(ctx->state, final_key, ctx->state_bytes);
	}
}

void skein_finish(
	struct skein_t* ctx
)
{
	k_bc_finish(ctx->cipher);
}
