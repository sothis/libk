/*
 * libk - arc4.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "streamcipher_desc.h"
#include "utils/unittest_desc.h"
#include "utils/xor.h"

struct arc4_t {
	uint8_t		m[256];
	uint8_t		x;
	uint8_t		y;
};

static inline void swap8(uint8_t *a, uint8_t *b)
{
	uint8_t swap;

	swap = *a;
	*a = *b;
	*b = swap;
}

static void arc4_init
(void* state, const void* key, uint32_t keybits)
{
	struct arc4_t* c = state;
	uint8_t a = 0, b = 0;
	const uint8_t* k = key;

	c->x = 0;
	c->y = 0;

	for (size_t i = 0; i < 256; ++i)
		c->m[i] = i;

	for (size_t i = 0; i < 256; ++i) {
		b = (k[a] + c->m[i] + b) & 255;
		swap8(&c->m[i], &c->m[b]);
		a = (a + 1) % ((keybits + 7) / 8);
	}
}

static void arc4_update
(void* state, void* output, size_t length)
{
	struct arc4_t* c = state;
	uint8_t x, y, j;
	uint8_t* out = output;

	x = c->x;
	y = c->y;

	for (size_t i = 0; i < length; ++i) {
		x = (x + 1) & 255;
		y = (c->m[x] + y) & 255;
		swap8(c->m+x, c->m+y);
		j = (c->m[x] + c->m[y]) & 255;
		out[i] = c->m[j];
	}

	c->x = x;
	c->y = y;
}


_unittest_data_ uint8_t _key[8] =
{
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
};

_unittest_data_ uint8_t _plain[8] =
{
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
};

_unittest_data_ uint8_t _cipher[8] =
{
	0x75, 0xb7, 0x87, 0x80, 0x99, 0xe0, 0xc5, 0x96
};

unittest(arc4, "ARC4")
{
	struct arc4_t c;
	uint8_t _out[sizeof(_plain)];

	arc4_init(&c, _key, sizeof(_key)*8);
	arc4_update(&c, _out, sizeof(_plain));
	xorb_64(_out, _plain, _out, sizeof(_plain));

	if (memcmp(_out, _cipher, sizeof(_cipher))) {
		*details = "output does not match expected ciphertext";
		return -1;
	}
	return 0;
}


static const char* const authors[] = {
	"Janos Laube <janos.dev@gmail.com>",
	0
};

static const struct range32_t key_range[] = {
	{ .from = 40, .to = 2048 },
	{ 0 }
};

streamcipher_start(ARC4, "ARC4")
	.authors		= authors,
	.state_size		= 2064,
	.context_size		= sizeof(struct arc4_t),
	.key_range		= key_range,
	.insecure		= 1,
	.init			= &arc4_init,
	.update			= &arc4_update,
streamcipher_end
