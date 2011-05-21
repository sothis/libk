/*
 * libk - sha1.c
 *
 * 1998, Steve Reid <sreid@sea-to-sky.net>
 * 1998, James H. Brown <jbrown@burgoyne.com>
 * 2001, Saul Kravitz <Saul.Kravitz@celera.com>
 * 2002, Ralph Giles <giles@ghostscript.com>
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../hash_desc.h"
#include "utils/unittest_desc.h"

enum sha1_characteristics_e {
	sha1_digest_bytes	= 20,
	sha1_state_size		= sha1_digest_bytes*8
};

struct sha1_t {
	uint32_t	state[5];
	uint32_t	count[2];
	uint32_t	buffer[16];
};

#define rol32(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#ifdef __BIG_ENDIAN__
#define blk0(i) block[i]
#else
#define blk0(i)	(block[i] = (rol32(block[i],24)&0xff00ff00)	\
		|(rol32(block[i],8)&0x00ff00ff))
#endif

#define blk(i)	(block[i&15] =				\
		rol32(block[(i+13)&15]^block[(i+8)&15]	\
		^block[(i+2)&15]^block[i&15],1))

#define R0(v,w,x,y,z,i)							\
	z+=((w&(x^y))^y)+blk0(i)+0x5a827999+rol32(v,5);w=rol32(w,30);
#define R1(v,w,x,y,z,i)							\
	z+=((w&(x^y))^y)+blk(i)+0x5a827999+rol32(v,5);w=rol32(w,30);
#define R2(v,w,x,y,z,i)							\
	z+=(w^x^y)+blk(i)+0x6ed9eba1+rol32(v,5);w=rol32(w,30);
#define R3(v,w,x,y,z,i)							\
	z+=(((w|x)&y)|(w&x))+blk(i)+0x8f1bbcdc+rol32(v,5);w=rol32(w,30);
#define R4(v,w,x,y,z,i)							\
	z+=(w^x^y)+blk(i)+0xca62c1d6+rol32(v,5);w=rol32(w,30);


void sha1_compress
(uint32_t* state, uint32_t* block)
{
	uint32_t a, b, c, d, e;

	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];

	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
}

static void sha1_init
(void* c, uint32_t output_bits)
{
	struct sha1_t* context = c;
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;
	context->state[4] = 0xc3d2e1f0;
	context->count[0] = context->count[1] = 0;
}

static void sha1_update
(void* c, const void* input, const size_t len, uint32_t output_bits)
{
	struct sha1_t* context = c;
	size_t i, j;

	j = (context->count[0] >> 3) & 63;
	if ((context->count[0] += len << 3) < (len << 3))
		context->count[1]++;
	context->count[1] += (len >> 29);
	if ((j + len) > 63) {
		memcpy(((void*)context->buffer)+j, input, (i = 64-j));
		sha1_compress(context->state, context->buffer);
		for (; i + 63 < len; i += 64) {
			memcpy(context->buffer, input + i, 64);
			sha1_compress(context->state, context->buffer);
		}
		j = 0;
	} else
		i = 0;
	memcpy(((void*)context->buffer)+j, input+i, len - i);
}

static void sha1_final
(void* c, void* digest, uint32_t output_bits)
{
	struct sha1_t* context = c;
	uint8_t finalcount[8];

	for (size_t i = 0; i < 8; i++) {
		finalcount[i] = (uint8_t)((context->count[(i>=4 ? 0 : 1)]
		>> ((3-(i & 3)) * 8) ) & 255);
	}
	sha1_update(context, "\x80", 1, 0);
	while ((context->count[0] & 504) != 448) {
		sha1_update(context, "\x00", 1, 0);
	}
	sha1_update(context, finalcount, 8, 0);
	for (size_t i = 0; i < sha1_digest_bytes; i++) {
		((uint8_t*)digest)[i] = (uint8_t)
		((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
	}
}


_unittest_data_ uint8_t _message[163] = {
	0xec, 0x29, 0x56, 0x12, 0x44, 0xed, 0xe7, 0x06,
	0xb6, 0xeb, 0x30, 0xa1, 0xc3, 0x71, 0xd7, 0x44,
	0x50, 0xa1, 0x05, 0xc3, 0xf9, 0x73, 0x5f, 0x7f,
	0xa9, 0xfe, 0x38, 0xcf, 0x67, 0xf3, 0x04, 0xa5,
	0x73, 0x6a, 0x10, 0x6e, 0x92, 0xe1, 0x71, 0x39,
	0xa6, 0x81, 0x3b, 0x1c, 0x81, 0xa4, 0xf3, 0xd3,
	0xfb, 0x95, 0x46, 0xab, 0x42, 0x96, 0xfa, 0x9f,
	0x72, 0x28, 0x26, 0xc0, 0x66, 0x86, 0x9e, 0xda,
	0xcd, 0x73, 0xb2, 0x54, 0x80, 0x35, 0x18, 0x58,
	0x13, 0xe2, 0x26, 0x34, 0xa9, 0xda, 0x44, 0x00,
	0x0d, 0x95, 0xa2, 0x81, 0xff, 0x9f, 0x26, 0x4e,
	0xcc, 0xe0, 0xa9, 0x31, 0x22, 0x21, 0x62, 0xd0,
	0x21, 0xcc, 0xa2, 0x8d, 0xb5, 0xf3, 0xc2, 0xaa,
	0x24, 0x94, 0x5a, 0xb1, 0xe3, 0x1c, 0xb4, 0x13,
	0xae, 0x29, 0x81, 0x0f, 0xd7, 0x94, 0xca, 0xd5,
	0xdf, 0xaf, 0x29, 0xec, 0x43, 0xcb, 0x38, 0xd1,
	0x98, 0xfe, 0x4a, 0xe1, 0xda, 0x23, 0x59, 0x78,
	0x02, 0x21, 0x40, 0x5b, 0xd6, 0x71, 0x2a, 0x53,
	0x05, 0xda, 0x4b, 0x1b, 0x73, 0x7f, 0xce, 0x7c,
	0xd2, 0x1c, 0x0e, 0xb7, 0x72, 0x8d, 0x08, 0x23,
	0x5a, 0x90, 0x11
};

_unittest_data_ uint8_t digest_sha1_00[sha1_digest_bytes] = {
	0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d,
	0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90,
	0xaf, 0xd8, 0x07, 0x09
};

_unittest_data_ uint8_t digest_sha1_163[sha1_digest_bytes] = {
	0x97, 0x01, 0x11, 0xc4, 0xe7, 0x7b, 0xcc, 0x88,
	0xcc, 0x20, 0x45, 0x9c, 0x02, 0xb6, 0x9b, 0x4a,
	0xa8, 0xf5, 0x82, 0x17
};

unittest(sha1, "SHA1")
{
	struct sha1_t c;
	uint8_t _out[sha1_digest_bytes];

	sha1_init(&c, sha1_state_size);
	sha1_update(&c, 0, 0, sha1_state_size);
	sha1_final(&c, _out, sha1_state_size);
	if (memcmp(_out, digest_sha1_00, sha1_digest_bytes)) {
		*details = "output does not match expected hashsum";
		return -1;
	}
	sha1_init(&c, sha1_state_size);
	sha1_update(&c, _message, sizeof(_message), sha1_state_size);
	sha1_final(&c, _out, sha1_state_size);
	if (memcmp(_out, digest_sha1_163, sha1_digest_bytes)) {
		*details = "output does not match expected hashsum";
		return -1;
	}
	return 0;
}


static const char* const authors[] = {
	"Steve Reid <sreid@sea-to-sky.net>",
	"James H. Brown <jbrown@burgoyne.com>",
	"Saul Kravitz <Saul.Kravitz@celera.com>",
	"Ralph Giles <giles@ghostscript.com>",
	"Janos Laube <janos.dev@gmail.com>",
	0
};

hashsum_start(SHA1, "SHA1")
	.authors		= authors,
	.insecure		= 1,
	.state_size		= sha1_state_size,
	.context_size		= sizeof(struct sha1_t),
	.init			= &sha1_init,
	.update			= &sha1_update,
	.final			= &sha1_final,
hashsum_end
