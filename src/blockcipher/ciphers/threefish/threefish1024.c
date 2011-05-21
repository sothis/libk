/*
 * libk - threefish1024.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include "threefish.h"


enum tf_characteristics_e {
	tf1024_block_size	= 128,
};

struct tf1024_key_schedule_t {
	uint64_t	t[3];
	uint64_t	k[17];
};

#define r01 24
#define r02 13
#define r03 8
#define r04 47
#define r05 8
#define r06 17
#define r07 22
#define r08 37
#define r09 38
#define r10 19
#define r11 10
#define r12 55
#define r13 49
#define r14 18
#define r15 23
#define r16 52
#define r17 33
#define r18 4
#define r19 51
#define r20 13
#define r21 34
#define r22 41
#define r23 59
#define r24 17
#define r25 5
#define r26 20
#define r27 48
#define r28 41
#define r29 47
#define r30 28
#define r31 16
#define r32 25
#define r33 41
#define r34 9
#define r35 37
#define r36 31
#define r37 12
#define r38 47
#define r39 44
#define r40 30
#define r41 16
#define r42 34
#define r43 56
#define r44 51
#define r45 4
#define r46 53
#define r47 42
#define r48 41
#define r49 31
#define r50 44
#define r51 47
#define r52 46
#define r53 19
#define r54 42
#define r55 44
#define r56 25
#define r57 9
#define r58 48
#define r59 35
#define r60 52
#define r61 23
#define r62 31
#define r63 37
#define r64 20

#define m1(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18)(\
	m(0,1,r01,k[p0],k[p1]),m(2,3,r02,k[p2],k[p3]),m(4,5,r03,k[p4],k[p5]),\
	m(6,7,r04,k[p6],k[p7]),m(8,9,r05,k[p8],k[p9]),\
	m(10,11,r06,k[p10],k[p11]),m(12,13,r07,k[p12],k[p13]+t[p14]),\
	m(14,15,r08,k[p15]+t[p16],k[p17]+p18),m(0,9,r09,0,0),m(2,13,r10,0,0),\
	m(6,11,r11,0,0),m(4,15,r12,0,0),m(10,7,r13,0,0),m(12,3,r14,0,0),\
	m(14,5,r15,0,0),m(8,1,r16,0,0),m(0,7,r17,0,0),m(2,5,r18,0,0),\
	m(4,3,r19,0,0),m(6,1,r20,0,0),m(12,15,r21,0,0),m(14,13,r22,0,0),\
	m(8,11,r23,0,0),m(10,9,r24,0,0),m(0,15,r25,0,0),m(2,11,r26,0,0),\
	m(6,13,r27,0,0),m(4,9,r28,0,0),m(14,1,r29,0,0),m(8,5,r30,0,0),\
	m(10,3,r31,0,0),m(12,7,r32,0,0))

#define m2(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18)(\
	m(0,1,r33,k[p0],k[p1]),m(2,3,r34,k[p2],k[p3]),m(4,5,r35,k[p4],k[p5]),\
	m(6,7,r36,k[p6],k[p7]),m(8,9,r37,k[p8],k[p9]),\
	m(10,11,r38,k[p10],k[p11]),m(12,13,r39,k[p12],k[p13]+t[p14]),\
	m(14,15,r40,k[p15]+t[p16],k[p17]+p18),m(0,9,r41,0,0),m(2,13,r42,0,0),\
	m(6,11,r43,0,0),m(4,15,r44,0,0),m(10,7,r45,0,0),m(12,3,r46,0,0),\
	m(14,5,r47,0,0),m(8,1,r48,0,0),m(0,7,r49,0,0),m(2,5,r50,0,0),\
	m(4,3,r51,0,0),m(6,1,r52,0,0),m(12,15,r53,0,0),m(14,13,r54,0,0),\
	m(8,11,r55,0,0),m(10,9,r56,0,0),m(0,15,r57,0,0),m(2,11,r58,0,0),\
	m(6,13,r59,0,0),m(4,9,r60,0,0),m(14,1,r61,0,0),m(8,5,r62,0,0),\
	m(10,3,r63,0,0),m(12,7,r64,0,0))

#define u1(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18)(\
	u(0,15,r25,0,0),u(2,11,r26,0,0),u(6,13,r27,0,0),u(4,9,r28,0,0),\
	u(14,1,r29,0,0),u(8,5,r30,0,0),u(10,3,r31,0,0),u(12,7,r32,0,0),\
	u(0,7,r17,0,0),u(2,5,r18,0,0),u(4,3,r19,0,0),u(6,1,r20,0,0),\
	u(12,15,r21,0,0),u(14,13,r22,0,0),u(8,11,r23,0,0),u(10,9,r24,0,0),\
	u(0,9,r09,0,0),u(2,13,r10,0,0),u(6,11,r11,0,0),u(4,15,r12,0,0),\
	u(10,7,r13,0,0),u(12,3,r14,0,0),u(14,5,r15,0,0),u(8,1,r16,0,0),\
	u(0,1,r01,k[p0],k[p1]),u(2,3,r02,k[p2],k[p3]),u(4,5,r03,k[p4],k[p5]),\
	u(6,7,r04,k[p6],k[p7]),u(8,9,r05,k[p8],k[p9]),\
	u(10,11,r06,k[p10],k[p11]),u(12,13,r07,k[p12],k[p13]+t[p14]),\
	u(14,15,r08,k[p15]+t[p16],k[p17]+p18))

#define u2(p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18)(\
	u(0,15,r57,0,0),u(2,11,r58,0,0),u(6,13,r59,0,0),u(4,9,r60,0,0),\
	u(14,1,r61,0,0),u(8,5,r62,0,0),u(10,3,r63,0,0),u(12,7,r64,0,0),\
	u(0,7,r49,0,0),u(2,5,r50,0,0),u(4,3,r51,0,0),u(6,1,r52,0,0),\
	u(12,15,r53,0,0),u(14,13,r54,0,0),u(8,11,r55,0,0),u(10,9,r56,0,0),\
	u(0,9,r41,0,0),u(2,13,r42,0,0),u(6,11,r43,0,0),u(4,15,r44,0,0),\
	u(10,7,r45,0,0),u(12,3,r46,0,0),u(14,5,r47,0,0),u(8,1,r48,0,0),\
	u(0,1,r33,k[p0],k[p1]),u(2,3,r34,k[p2],k[p3]),u(4,5,r35,k[p4],k[p5]),\
	u(6,7,r36,k[p6],k[p7]),u(8,9,r37,k[p8],k[p9]),\
	u(10,11,r38,k[p10],k[p11]),u(12,13,r39,k[p12],k[p13]+t[p14]),\
	u(14,15,r40,k[p15]+t[p16],k[p17]+p18))


static void
tf1024_set_key(void* c, const void* k, const uint32_t kb)
{
	struct tf1024_key_schedule_t* s = c;
	copy_uint64_t(s->k, k, tf1024_block_size/sizeof(uint64_t));
	s->k[16] = tf_parity^s->k[0]^s->k[1]^s->k[2]^s->k[3]^s->k[4]^s->k[5];
	s->k[16] ^= s->k[6]^s->k[7]^s->k[8]^s->k[9]^s->k[10]^s->k[11]^s->k[12];
	s->k[16] ^= s->k[13]^s->k[14]^s->k[15];
}

static void
tf1024_set_tweak(void* c, const void* t, const uint32_t tb)
{
	struct tf1024_key_schedule_t* s = c;
	copy_uint64_t(s->t, t, 2);
	s->t[2] = s->t[0]^s->t[1];
}

static void
tf1024_encrypt(const void* c, const void* i, void* o)
{
	const struct tf1024_key_schedule_t* s = c;
	uint64_t o0, o1, o2, o3, o4, o5, o6, o7;
	uint64_t o8, o9, o10, o11, o12, o13, o14, o15;
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
	o8 = _get_uint64_l(i+64);
	o9 = _get_uint64_l(i+72);
	o10 = _get_uint64_l(i+80);
	o11 = _get_uint64_l(i+88);
	o12 = _get_uint64_l(i+96);
	o13 = _get_uint64_l(i+104);
	o14 = _get_uint64_l(i+112);
	o15 = _get_uint64_l(i+120);
	m1(0,1,2,3,4,5,6,7,8,9,10,11,12,13,0,14,1,15,0);
	m2(1,2,3,4,5,6,7,8,9,10,11,12,13,14,1,15,2,16,1);
	m1(2,3,4,5,6,7,8,9,10,11,12,13,14,15,2,16,0,0,2);
	m2(3,4,5,6,7,8,9,10,11,12,13,14,15,16,0,0,1,1,3);
	m1(4,5,6,7,8,9,10,11,12,13,14,15,16,0,1,1,2,2,4);
	m2(5,6,7,8,9,10,11,12,13,14,15,16,0,1,2,2,0,3,5);
	m1(6,7,8,9,10,11,12,13,14,15,16,0,1,2,0,3,1,4,6);
	m2(7,8,9,10,11,12,13,14,15,16,0,1,2,3,1,4,2,5,7);
	m1(8,9,10,11,12,13,14,15,16,0,1,2,3,4,2,5,0,6,8);
	m2(9,10,11,12,13,14,15,16,0,1,2,3,4,5,0,6,1,7,9);
	m1(10,11,12,13,14,15,16,0,1,2,3,4,5,6,1,7,2,8,10);
	m2(11,12,13,14,15,16,0,1,2,3,4,5,6,7,2,8,0,9,11);
	m1(12,13,14,15,16,0,1,2,3,4,5,6,7,8,0,9,1,10,12);
	m2(13,14,15,16,0,1,2,3,4,5,6,7,8,9,1,10,2,11,13);
	m1(14,15,16,0,1,2,3,4,5,6,7,8,9,10,2,11,0,12,14);
	m2(15,16,0,1,2,3,4,5,6,7,8,9,10,11,0,12,1,13,15);
	m1(16,0,1,2,3,4,5,6,7,8,9,10,11,12,1,13,2,14,16);
	m2(0,1,2,3,4,5,6,7,8,9,10,11,12,13,2,14,0,15,17);
	m1(1,2,3,4,5,6,7,8,9,10,11,12,13,14,0,15,1,16,18);
	m2(2,3,4,5,6,7,8,9,10,11,12,13,14,15,1,16,2,0,19);
	_put_uint64_l(o, o0+k[3]);
	_put_uint64_l(o+8, o1+k[4]);
	_put_uint64_l(o+16, o2+k[5]);
	_put_uint64_l(o+24, o3+k[6]);
	_put_uint64_l(o+32, o4+k[7]);
	_put_uint64_l(o+40, o5+k[8]);
	_put_uint64_l(o+48, o6+k[9]);
	_put_uint64_l(o+56, o7+k[10]);
	_put_uint64_l(o+64, o8+k[11]);
	_put_uint64_l(o+72, o9+k[12]);
	_put_uint64_l(o+80, o10+k[13]);
	_put_uint64_l(o+88, o11+k[14]);
	_put_uint64_l(o+96, o12+k[15]);
	_put_uint64_l(o+104, o13+k[16]+t[2]);
	_put_uint64_l(o+112, o14+k[0]+t[0]);
	_put_uint64_l(o+120, o15+k[1]+20);
}

static void
tf1024_decrypt(const void* c, const void* i, void* o)
{
	const struct tf1024_key_schedule_t* s = c;
	uint64_t o0, o1, o2, o3, o4, o5, o6, o7;
	uint64_t o8, o9, o10, o11, o12, o13, o14, o15;
	const uint64_t* t = s->t;
	const uint64_t* k = s->k;
	o0 = _get_uint64_l(i)-k[3];
	o1 = _get_uint64_l(i+8)-k[4];
	o2 = _get_uint64_l(i+16)-k[5];
	o3 = _get_uint64_l(i+24)-k[6];
	o4 = _get_uint64_l(i+32)-k[7];
	o5 = _get_uint64_l(i+40)-k[8];
	o6 = _get_uint64_l(i+48)-k[9];
	o7 = _get_uint64_l(i+56)-k[10];
	o8 = _get_uint64_l(i+64)-k[11];
	o9 = _get_uint64_l(i+72)-k[12];
	o10 = _get_uint64_l(i+80)-k[13];
	o11 = _get_uint64_l(i+88)-k[14];
	o12 = _get_uint64_l(i+96)-k[15];
	o13 = _get_uint64_l(i+104)-k[16]-t[2];
	o14 = _get_uint64_l(i+112)-k[0]-t[0];
	o15 = _get_uint64_l(i+120)-k[1]-20;
	u2(2,3,4,5,6,7,8,9,10,11,12,13,14,15,1,16,2,0,19);
	u1(1,2,3,4,5,6,7,8,9,10,11,12,13,14,0,15,1,16,18);
	u2(0,1,2,3,4,5,6,7,8,9,10,11,12,13,2,14,0,15,17);
	u1(16,0,1,2,3,4,5,6,7,8,9,10,11,12,1,13,2,14,16);
	u2(15,16,0,1,2,3,4,5,6,7,8,9,10,11,0,12,1,13,15);
	u1(14,15,16,0,1,2,3,4,5,6,7,8,9,10,2,11,0,12,14);
	u2(13,14,15,16,0,1,2,3,4,5,6,7,8,9,1,10,2,11,13);
	u1(12,13,14,15,16,0,1,2,3,4,5,6,7,8,0,9,1,10,12);
	u2(11,12,13,14,15,16,0,1,2,3,4,5,6,7,2,8,0,9,11);
	u1(10,11,12,13,14,15,16,0,1,2,3,4,5,6,1,7,2,8,10);
	u2(9,10,11,12,13,14,15,16,0,1,2,3,4,5,0,6,1,7,9);
	u1(8,9,10,11,12,13,14,15,16,0,1,2,3,4,2,5,0,6,8);
	u2(7,8,9,10,11,12,13,14,15,16,0,1,2,3,1,4,2,5,7);
	u1(6,7,8,9,10,11,12,13,14,15,16,0,1,2,0,3,1,4,6);
	u2(5,6,7,8,9,10,11,12,13,14,15,16,0,1,2,2,0,3,5);
	u1(4,5,6,7,8,9,10,11,12,13,14,15,16,0,1,1,2,2,4);
	u2(3,4,5,6,7,8,9,10,11,12,13,14,15,16,0,0,1,1,3);
	u1(2,3,4,5,6,7,8,9,10,11,12,13,14,15,2,16,0,0,2);
	u2(1,2,3,4,5,6,7,8,9,10,11,12,13,14,1,15,2,16,1);
	u1(0,1,2,3,4,5,6,7,8,9,10,11,12,13,0,14,1,15,0);
	_put_uint64_l(o, o0);
	_put_uint64_l(o+8, o1);
	_put_uint64_l(o+16, o2);
	_put_uint64_l(o+24, o3);
	_put_uint64_l(o+32, o4);
	_put_uint64_l(o+40, o5);
	_put_uint64_l(o+48, o6);
	_put_uint64_l(o+56, o7);
	_put_uint64_l(o+64, o8);
	_put_uint64_l(o+72, o9);
	_put_uint64_l(o+80, o10);
	_put_uint64_l(o+88, o11);
	_put_uint64_l(o+96, o12);
	_put_uint64_l(o+104, o13);
	_put_uint64_l(o+112, o14);
	_put_uint64_l(o+120, o15);
}


_unittest_data_ uint8_t _tweak[] = {
	0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
	0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0
};

_unittest_data_ uint8_t _key_1024[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
};

_unittest_data_ uint8_t _plain_1024[] = {
	0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
	0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0,
	0xef, 0xee, 0xed, 0xec, 0xeb, 0xea, 0xe9, 0xe8,
	0xe7, 0xe6, 0xe5, 0xe4, 0xe3, 0xe2, 0xe1, 0xe0,
	0xdf, 0xde, 0xdd, 0xdc, 0xdb, 0xda, 0xd9, 0xd8,
	0xd7, 0xd6, 0xd5, 0xd4, 0xd3, 0xd2, 0xd1, 0xd0,
	0xcf, 0xce, 0xcd, 0xcc, 0xcb, 0xca, 0xc9, 0xc8,
	0xc7, 0xc6, 0xc5, 0xc4, 0xc3, 0xc2, 0xc1, 0xc0,
	0xbf, 0xbe, 0xbd, 0xbc, 0xbb, 0xba, 0xb9, 0xb8,
	0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0xb2, 0xb1, 0xb0,
	0xaf, 0xae, 0xad, 0xac, 0xab, 0xaa, 0xa9, 0xa8,
	0xa7, 0xa6, 0xa5, 0xa4, 0xa3, 0xa2, 0xa1, 0xa0,
	0x9f, 0x9e, 0x9d, 0x9c, 0x9b, 0x9a, 0x99, 0x98,
	0x97, 0x96, 0x95, 0x94, 0x93, 0x92, 0x91, 0x90,
	0x8f, 0x8e, 0x8d, 0x8c, 0x8b, 0x8a, 0x89, 0x88,
	0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x80
};

_unittest_data_ uint8_t _cipher_1024[] = {
	0xc2, 0x69, 0x99, 0xcc, 0x6e, 0x4f, 0xad, 0xe4,
	0xda, 0x88, 0x20, 0x02, 0xb7, 0x10, 0x59, 0xc2,
	0x16, 0xe1, 0x03, 0x2b, 0x76, 0x59, 0xfe, 0x37,
	0x80, 0x44, 0x4f, 0x83, 0xcf, 0xc8, 0x2f, 0xac,
	0x86, 0x56, 0x5f, 0x3f, 0x03, 0x7b, 0xdd, 0x62,
	0x9d, 0x27, 0x61, 0x81, 0x32, 0x44, 0xef, 0x31,
	0x9d, 0x85, 0x68, 0x32, 0xe7, 0x15, 0xa3, 0x4d,
	0x74, 0x06, 0x7c, 0xd8, 0x7e, 0x3e, 0xc2, 0xb4,
	0x92, 0xb3, 0xbd, 0x00, 0xca, 0xb0, 0x0c, 0xc2,
	0x1d, 0x9b, 0xba, 0xd9, 0x13, 0xe3, 0x4f, 0xe2,
	0xaf, 0xce, 0x93, 0xd7, 0xcb, 0x25, 0x48, 0x44,
	0x65, 0xc1, 0xe6, 0x5f, 0x86, 0x67, 0xcc, 0xd8,
	0x6f, 0x93, 0xcd, 0xf5, 0x9b, 0xf5, 0x62, 0x3b,
	0x30, 0xd1, 0x3a, 0x03, 0xde, 0x81, 0x56, 0x6c,
	0xfd, 0x50, 0xd9, 0x26, 0x6f, 0x82, 0x4b, 0xff,
	0x5a, 0x8e, 0xad, 0x98, 0x10, 0x66, 0x53, 0x95
};

unittest(tf1024_encrypt, "Threefish 1024 encryption")
{
	struct tf1024_key_schedule_t schedule;
	uint8_t _out[sizeof(_plain_1024)];

	tf1024_set_key(&schedule, _key_1024, sizeof(_key_1024)*8);
	tf1024_set_tweak(&schedule, _tweak, sizeof(_tweak)*8);
	tf1024_encrypt(&schedule, _plain_1024, _out);
	if (memcmp(_out, _cipher_1024, sizeof(_cipher_1024))) {
		*details = "output does not match expected ciphertext";
		return -1;
	}
	return 0;
}

unittest(tf1024_decrypt, "Threefish 1024 decryption")
{
	struct tf1024_key_schedule_t schedule;
	uint8_t _out[sizeof(_cipher_1024)];

	tf1024_set_key(&schedule, _key_1024, sizeof(_key_1024)*8);
	tf1024_set_tweak(&schedule, _tweak, sizeof(_tweak)*8);
	tf1024_decrypt(&schedule, _cipher_1024, _out);
	if (memcmp(_out, _plain_1024, sizeof(_plain_1024))) {
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
	{ .from = 1024, .to = 1024 },
	{ 0 }
};

static const struct range32_t tweak_range[] = {
	{ .from = 128, .to = 128 },
	{ 0 }
};

blockcipher_start(THREEFISH_1024, "Threefish 1024")
	.authors		= authors,
	.insecure		= 0,
	.block_size		= tf1024_block_size,
	.key_range		= key_range,
	.tweak_range		= tweak_range,
	.schedule_size		= sizeof(struct tf1024_key_schedule_t),
	.set_encrypt_key	= &tf1024_set_key,
	.set_decrypt_key	= &tf1024_set_key,
	.set_tweak		= &tf1024_set_tweak,
	.encrypt		= &tf1024_encrypt,
	.decrypt		= &tf1024_decrypt,
blockcipher_end
