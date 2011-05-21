/*
 * libk - threefish256.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include "threefish.h"


enum tf_characteristics_e {
	tf256_block_size	= 32,
};

struct tf256_key_schedule_t {
	uint64_t	t[3];
	uint64_t	k[5];
};

#define r01 14
#define r02 16
#define r03 52
#define r04 57
#define r05 23
#define r06 40
#define r07 5
#define r08 37
#define r09 25
#define r10 33
#define r11 46
#define r12 12
#define r13 58
#define r14 22
#define r15 32
#define r16 32

#define m1(p0,p1,p2,p3,p4,p5,p6)(\
	m(0,1,r01,k[p0],k[p1]+t[p2]),m(2,3,r02,k[p3]+t[p4],k[p5]+p6),\
	m(0,3,r03,0,0),m(2,1,r04,0,0),m(0,1,r05,0,0),m(2,3,r06,0,0),\
	m(0,3,r07,0,0),m(2,1,r08,0,0))

#define m2(p0,p1,p2,p3,p4,p5,p6)(\
	m(0,1,r09,k[p0],k[p1]+t[p2]),m(2,3,r10,k[p3]+t[p4],k[p5]+p6),\
	m(0,3,r11,0,0),m(2,1,r12,0,0),m(0,1,r13,0,0),m(2,3,r14,0,0),\
	m(0,3,r15,0,0),m(2,1,r16,0,0))

#define u1(p0,p1,p2,p3,p4,p5,p6)(\
	u(0,3,r07,0,0),u(2,1,r08,0,0),u(0,1,r05,0,0),u(2,3,r06,0,0),\
	u(0,3,r03,0,0),u(2,1,r04,0,0),u(0,1,r01,k[p0],k[p1]+t[p2]),\
	u(2,3,r02,k[p3]+t[p4],k[p5]+p6))

#define u2(p0,p1,p2,p3,p4,p5,p6)(\
	u(0,3,r15,0,0),u(2,1,r16,0,0),u(0,1,r13,0,0),u(2,3,r14,0,0),\
	u(0,3,r11,0,0),u(2,1,r12,0,0),u(0,1,r09,k[p0],k[p1]+t[p2]),\
	u(2,3,r10,k[p3]+t[p4],k[p5]+p6))


static void
tf256_set_key(void* c, const void* k, const uint32_t kb)
{
	struct tf256_key_schedule_t* s = c;
	copy_uint64_t(s->k, k, tf256_block_size/sizeof(uint64_t));
	s->k[4] = tf_parity^s->k[0]^s->k[1]^s->k[2]^s->k[3];
}

static void
tf256_set_tweak(void* c, const void* t, const uint32_t tb)
{
	struct tf256_key_schedule_t* s = c;
	copy_uint64_t(s->t, t, 2);
	s->t[2] = s->t[0]^s->t[1];
}

static void
tf256_encrypt(const void* c, const void* i, void* o)
{
	const struct tf256_key_schedule_t* s = c;
	uint64_t o0, o1, o2, o3;
	const uint64_t* t = s->t;
	const uint64_t* k = s->k;
	o0 = _get_uint64_l(i);
	o1 = _get_uint64_l(i+8);
	o2 = _get_uint64_l(i+16);
	o3 = _get_uint64_l(i+24);
	m1(0,1,0,2,1,3,0), m2(1,2,1,3,2,4,1);
	m1(2,3,2,4,0,0,2), m2(3,4,0,0,1,1,3);
	m1(4,0,1,1,2,2,4), m2(0,1,2,2,0,3,5);
	m1(1,2,0,3,1,4,6), m2(2,3,1,4,2,0,7);
	m1(3,4,2,0,0,1,8), m2(4,0,0,1,1,2,9);
	m1(0,1,1,2,2,3,10), m2(1,2,2,3,0,4,11);
	m1(2,3,0,4,1,0,12), m2(3,4,1,0,2,1,13);
	m1(4,0,2,1,0,2,14), m2(0,1,0,2,1,3,15);
	m1(1,2,1,3,2,4,16), m2(2,3,2,4,0,0,17);
	_put_uint64_l(o, o0+k[3]);
	_put_uint64_l(o+8, o1+k[4]+t[0]);
	_put_uint64_l(o+16, o2+k[0]+t[1]);
	_put_uint64_l(o+24, o3+k[1]+18);
}

static void
tf256_decrypt(const void* c, const void* i, void* o)
{
	const struct tf256_key_schedule_t* s = c;
	uint64_t o0, o1, o2, o3;
	const uint64_t* t = s->t;
	const uint64_t* k = s->k;
	o0 = _get_uint64_l(i)-k[3];
	o1 = _get_uint64_l(i+8)-k[4]-t[0];
	o2 = _get_uint64_l(i+16)-k[0]-t[1];
	o3 = _get_uint64_l(i+24)-k[1]-18;
	u2(2,3,2,4,0,0,17), u1(1,2,1,3,2,4,16);
	u2(0,1,0,2,1,3,15), u1(4,0,2,1,0,2,14);
	u2(3,4,1,0,2,1,13), u1(2,3,0,4,1,0,12);
	u2(1,2,2,3,0,4,11), u1(0,1,1,2,2,3,10);
	u2(4,0,0,1,1,2,9), u1(3,4,2,0,0,1,8);
	u2(2,3,1,4,2,0,7), u1(1,2,0,3,1,4,6);
	u2(0,1,2,2,0,3,5), u1(4,0,1,1,2,2,4);
	u2(3,4,0,0,1,1,3), u1(2,3,2,4,0,0,2);
	u2(1,2,1,3,2,4,1), u1(0,1,0,2,1,3,0);
	_put_uint64_l(o, o0);
	_put_uint64_l(o+8, o1);
	_put_uint64_l(o+16, o2);
	_put_uint64_l(o+24, o3);
}


_unittest_data_ uint8_t _tweak[] = {
	0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
	0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0
};

_unittest_data_ uint8_t _key_256[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

_unittest_data_ uint8_t _plain_256[] = {
	0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
	0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0,
	0xef, 0xee, 0xed, 0xec, 0xeb, 0xea, 0xe9, 0xe8,
	0xe7, 0xe6, 0xe5, 0xe4, 0xe3, 0xe2, 0xe1, 0xe0
};

_unittest_data_ uint8_t _cipher_256[] = {
	0x48, 0x07, 0x07, 0xb5, 0xe4, 0x5e, 0x71, 0x29,
	0xd9, 0x59, 0x06, 0x43, 0xb6, 0xd0, 0x24, 0xe0,
	0xb0, 0xf4, 0xfd, 0xbb, 0x7b, 0x11, 0xc2, 0xe7,
	0x9d, 0xc6, 0xfd, 0xb4, 0xcd, 0xc8, 0xaf, 0x36
};

unittest(tf256_encrypt, "Threefish 256 encryption")
{
	struct tf256_key_schedule_t schedule;
	uint8_t _out[sizeof(_plain_256)];

	tf256_set_key(&schedule, _key_256, sizeof(_key_256)*8);
	tf256_set_tweak(&schedule, _tweak, sizeof(_tweak)*8);
	tf256_encrypt(&schedule, _plain_256, _out);
	if (memcmp(_out, _cipher_256, sizeof(_cipher_256))) {
		*details = "output does not match expected ciphertext";
		return -1;
	}
	return 0;
}

unittest(tf256_decrypt, "Threefish 256 decryption")
{
	struct tf256_key_schedule_t schedule;
	uint8_t _out[sizeof(_cipher_256)];

	tf256_set_key(&schedule, _key_256, sizeof(_key_256)*8);
	tf256_set_tweak(&schedule, _tweak, sizeof(_tweak)*8);
	tf256_decrypt(&schedule, _cipher_256, _out);
	if (memcmp(_out, _plain_256, sizeof(_plain_256))) {
		*details = "output does not match expected plaintext";
		return -1;
	}
	return 0;
}


static const char* const authors[] = {
	"Janos Laube <janos.dev@gmail.com>",
	0
};

static const struct range32_t key_range[] = {
	{ .from = 256, .to = 256 },
	{ 0 }
};

static const struct range32_t tweak_range[] = {
	{ .from = 128, .to = 128 },
	{ 0 }
};

blockcipher_start(THREEFISH_256, "Threefish 256")
	.authors		= authors,
	.insecure		= 0,
	.block_size		= tf256_block_size,
	.key_range		= key_range,
	.tweak_range		= tweak_range,
	.schedule_size		= sizeof(struct tf256_key_schedule_t),
	.set_encrypt_key	= &tf256_set_key,
	.set_decrypt_key	= &tf256_set_key,
	.set_tweak		= &tf256_set_tweak,
	.encrypt		= &tf256_encrypt,
	.decrypt		= &tf256_decrypt,
blockcipher_end
