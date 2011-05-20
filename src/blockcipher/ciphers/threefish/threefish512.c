#include "threefish.h"


enum tf_characteristics_e {
	tf512_block_size	= 64,
};

struct tf512_key_schedule_t {
	uint64_t	t[3];
	uint64_t	k[9];
};

#define r01 46
#define r02 36
#define r03 19
#define r04 37
#define r05 33
#define r06 27
#define r07 14
#define r08 42
#define r09 17
#define r10 49
#define r11 36
#define r12 39
#define r13 44
#define r14 9
#define r15 54
#define r16 56
#define r17 39
#define r18 30
#define r19 34
#define r20 24
#define r21 13
#define r22 50
#define r23 10
#define r24 17
#define r25 25
#define r26 29
#define r27 39
#define r28 43
#define r29 8
#define r30 35
#define r31 56
#define r32 22

#define m1(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)(\
	m(0,1,r01,k[p0],k[p1]),m(2,3,r02,k[p2],k[p3]),\
	m(4,5,r03,k[p4],k[p5]+t[p6]),m(6,7,r04,k[p7]+t[p8],k[p9]+p10),\
	m(2,1,r05,0,0),m(4,7,r06,0,0),m(6,5,r07,0,0),m(0,3,r08,0,0),\
	m(4,1,r09,0,0),m(6,3,r10,0,0),m(0,5,r11,0,0),m(2,7,r12,0,0),\
	m(6,1,r13,0,0),m(0,7,r14,0,0),m(2,5,r15,0,0),m(4,3,r16,0,0))

#define m2(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)(\
	m(0,1,r17,k[p0],k[p1]),m(2,3,r18,k[p2],k[p3]),\
	m(4,5,r19,k[p4],k[p5]+t[p6]),m(6,7,r20,k[p7]+t[p8],k[p9]+p10),\
	m(2,1,r21,0,0),m(4,7,r22,0,0),m(6,5,r23,0,0),m(0,3,r24,0,0),\
	m(4,1,r25,0,0),m(6,3,r26,0,0),m(0,5,r27,0,0),m(2,7,r28,0,0),\
	m(6,1,r29,0,0),m(0,7,r30,0,0),m(2,5,r31,0,0),m(4,3,r32,0,0))

#define u1(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)(\
	u(6,1,r13,0,0),u(0,7,r14,0,0),u(2,5,r15,0,0),u(4,3,r16,0,0),\
	u(4,1,r09,0,0),u(6,3,r10,0,0),u(0,5,r11,0,0),u(2,7,r12,0,0),\
	u(2,1,r05,0,0),u(4,7,r06,0,0),u(6,5,r07,0,0),u(0,3,r08,0,0),\
	u(0,1,r01,k[p0],k[p1]),u(2,3,r02,k[p2],k[p3]),\
	u(4,5,r03,k[p4],k[p5]+t[p6]),u(6,7,r04,k[p7]+t[p8],k[p9]+p10))

#define u2(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)(\
	u(6,1,r29,0,0),u(0,7,r30,0,0),u(2,5,r31,0,0),u(4,3,r32,0,0),\
	u(4,1,r25,0,0),u(6,3,r26,0,0),u(0,5,r27,0,0),u(2,7,r28,0,0),\
	u(2,1,r21,0,0),u(4,7,r22,0,0),u(6,5,r23,0,0),u(0,3,r24,0,0),\
	u(0,1,r17,k[p0],k[p1]),u(2,3,r18,k[p2],k[p3]),\
	u(4,5,r19,k[p4],k[p5]+t[p6]),u(6,7,r20,k[p7]+t[p8],k[p9]+p10))


static void
tf512_set_key(void* c, const void* k, const uint32_t kb)
{
	struct tf512_key_schedule_t* s = c;
	copy_uint64_t(s->k, k, tf512_block_size/sizeof(uint64_t));
	s->k[8] = tf_parity^s->k[0]^s->k[1]^s->k[2]^s->k[3]^s->k[4]^s->k[5];
	s->k[8] ^= s->k[6]^s->k[7];
}

static void
tf512_set_tweak(void* c, const void* t, const uint32_t tb)
{
	struct tf512_key_schedule_t* s = c;
	copy_uint64_t(s->t, t, 2);
	s->t[2] = s->t[0]^s->t[1];
}

static void
tf512_encrypt(const void* c, const void* i, void* o)
{
	const struct tf512_key_schedule_t* s = c;
	uint64_t o0, o1, o2, o3, o4, o5, o6, o7;
	const uint64_t* t = s->t;
	const uint64_t* k = s->k;
	o0 = _get_uint64_l(i);
	o1 = _get_uint64_l(i+8);
	o2 = _get_uint64_l(i+16);
	o3 = _get_uint64_l(i+24);
	o4 = _get_uint64_l(i+32);
	o5 = _get_uint64_l(i+40);
	o6 = _get_uint64_l(i+48);
	o7 = _get_uint64_l(i+56);
	m1(0,1,2,3,4,5,0,6,1,7,0), m2(1,2,3,4,5,6,1,7,2,8,1);
	m1(2,3,4,5,6,7,2,8,0,0,2), m2(3,4,5,6,7,8,0,0,1,1,3);
	m1(4,5,6,7,8,0,1,1,2,2,4), m2(5,6,7,8,0,1,2,2,0,3,5);
	m1(6,7,8,0,1,2,0,3,1,4,6), m2(7,8,0,1,2,3,1,4,2,5,7);
	m1(8,0,1,2,3,4,2,5,0,6,8), m2(0,1,2,3,4,5,0,6,1,7,9);
	m1(1,2,3,4,5,6,1,7,2,8,10), m2(2,3,4,5,6,7,2,8,0,0,11);
	m1(3,4,5,6,7,8,0,0,1,1,12), m2(4,5,6,7,8,0,1,1,2,2,13);
	m1(5,6,7,8,0,1,2,2,0,3,14), m2(6,7,8,0,1,2,0,3,1,4,15);
	m1(7,8,0,1,2,3,1,4,2,5,16), m2(8,0,1,2,3,4,2,5,0,6,17);
	_put_uint64_l(o, o0+k[0]);
	_put_uint64_l(o+8, o1+k[1]);
	_put_uint64_l(o+16, o2+k[2]);
	_put_uint64_l(o+24, o3+k[3]);
	_put_uint64_l(o+32, o4+k[4]);
	_put_uint64_l(o+40, o5+k[5]+t[0]);
	_put_uint64_l(o+48, o6+k[6]+t[1]);
	_put_uint64_l(o+56, o7+k[7]+18);
}

static void
tf512_decrypt(const void* c, const void* i, void* o)
{
	const struct tf512_key_schedule_t* s = c;
	uint64_t o0, o1, o2, o3, o4, o5, o6, o7;
	const uint64_t* t = s->t;
	const uint64_t* k = s->k;
	o0 = _get_uint64_l(i)-k[0];
	o1 = _get_uint64_l(i+8)-k[1];
	o2 = _get_uint64_l(i+16)-k[2];
	o3 = _get_uint64_l(i+24)-k[3];
	o4 = _get_uint64_l(i+32)-k[4];
	o5 = _get_uint64_l(i+40)-k[5]-t[0];
	o6 = _get_uint64_l(i+48)-k[6]-t[1];
	o7 = _get_uint64_l(i+56)-k[7]-18;
	u2(8,0,1,2,3,4,2,5,0,6,17), u1(7,8,0,1,2,3,1,4,2,5,16);
	u2(6,7,8,0,1,2,0,3,1,4,15), u1(5,6,7,8,0,1,2,2,0,3,14);
	u2(4,5,6,7,8,0,1,1,2,2,13), u1(3,4,5,6,7,8,0,0,1,1,12);
	u2(2,3,4,5,6,7,2,8,0,0,11), u1(1,2,3,4,5,6,1,7,2,8,10);
	u2(0,1,2,3,4,5,0,6,1,7,9), u1(8,0,1,2,3,4,2,5,0,6,8);
	u2(7,8,0,1,2,3,1,4,2,5,7), u1(6,7,8,0,1,2,0,3,1,4,6);
	u2(5,6,7,8,0,1,2,2,0,3,5), u1(4,5,6,7,8,0,1,1,2,2,4);
	u2(3,4,5,6,7,8,0,0,1,1,3), u1(2,3,4,5,6,7,2,8,0,0,2);
	u2(1,2,3,4,5,6,1,7,2,8,1), u1(0,1,2,3,4,5,0,6,1,7,0);
	_put_uint64_l(o, o0);
	_put_uint64_l(o+8, o1);
	_put_uint64_l(o+16, o2);
	_put_uint64_l(o+24, o3);
	_put_uint64_l(o+32, o4);
	_put_uint64_l(o+40, o5);
	_put_uint64_l(o+48, o6);
	_put_uint64_l(o+56, o7);
}


_unittest_data_ uint8_t _tweak[] = {
	0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
	0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0
};

_unittest_data_ uint8_t _key_512[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

_unittest_data_ uint8_t _plain_512[] = {
	0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
	0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0,
	0xef, 0xee, 0xed, 0xec, 0xeb, 0xea, 0xe9, 0xe8,
	0xe7, 0xe6, 0xe5, 0xe4, 0xe3, 0xe2, 0xe1, 0xe0,
	0xdf, 0xde, 0xdd, 0xdc, 0xdb, 0xda, 0xd9, 0xd8,
	0xd7, 0xd6, 0xd5, 0xd4, 0xd3, 0xd2, 0xd1, 0xd0,
	0xcf, 0xce, 0xcd, 0xcc, 0xcb, 0xca, 0xc9, 0xc8,
	0xc7, 0xc6, 0xc5, 0xc4, 0xc3, 0xc2, 0xc1, 0xc0
};

_unittest_data_ uint8_t _cipher_512[] = {
	0xfc, 0x5b, 0xa9, 0xdd, 0x9f, 0x6a, 0xd7, 0x09,
	0xc3, 0x30, 0x6c, 0x83, 0x88, 0x45, 0x51, 0xd4,
	0xcd, 0xf6, 0x50, 0x55, 0x88, 0xdc, 0xd5, 0x4e,
	0xc8, 0xfd, 0xd7, 0x6b, 0x2b, 0xa6, 0x4d, 0x16,
	0xf0, 0xc8, 0x55, 0x31, 0x35, 0xcb, 0x98, 0xbf,
	0xe8, 0x7d, 0x85, 0xb5, 0x82, 0x34, 0x73, 0x51,
	0x6d, 0x37, 0x8c, 0xfc, 0xd0, 0xe2, 0x4e, 0x64,
	0xb1, 0xd2, 0x85, 0x71, 0xbb, 0x3a, 0x8d, 0x6e
};

unittest(tf512_encrypt, "Threefish 512 encryption")
{
	struct tf512_key_schedule_t schedule;
	uint8_t _out[sizeof(_plain_512)];

	tf512_set_key(&schedule, _key_512, sizeof(_key_512)*8);
	tf512_set_tweak(&schedule, _tweak, sizeof(_tweak)*8);
	tf512_encrypt(&schedule, _plain_512, _out);
	if (memcmp(_out, _cipher_512, sizeof(_cipher_512))) {
		*details = "output does not match expected ciphertext";
		return -1;
	}
	return 0;
}

unittest(tf512_decrypt, "Threefish 512 decryption")
{
	struct tf512_key_schedule_t schedule;
	uint8_t _out[sizeof(_cipher_512)];

	tf512_set_key(&schedule, _key_512, sizeof(_key_512)*8);
	tf512_set_tweak(&schedule, _tweak, sizeof(_tweak)*8);
	tf512_decrypt(&schedule, _cipher_512, _out);
	if (memcmp(_out, _plain_512, sizeof(_plain_512))) {
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
	{ .from = 512, .to = 512 },
	{ 0 }
};

static const struct range32_t tweak_range[] = {
	{ .from = 128, .to = 128 },
	{ 0 }
};

blockcipher_start(THREEFISH_512, "Threefish 512")
	.authors		= authors,
	.insecure		= 0,
	.block_size		= tf512_block_size,
	.key_range		= key_range,
	.tweak_range		= tweak_range,
	.schedule_size		= sizeof(struct tf512_key_schedule_t),
	.set_encrypt_key	= &tf512_set_key,
	.set_decrypt_key	= &tf512_set_key,
	.set_tweak		= &tf512_set_tweak,
	.encrypt		= &tf512_encrypt,
	.decrypt		= &tf512_decrypt,
blockcipher_end
