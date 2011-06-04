/*
 * libk - bench_threefish.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include "utils/benchmark_desc.h"
#include <libk/libk.h>

bcmode_perftest(tf256_ctr_e, "Threefish 256 CTR", BLK_CIPHER_THREEFISH_256,
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, 256, 1);

bcmode_perftest(tf512_ctr_e, "Threefish 512 CTR", BLK_CIPHER_THREEFISH_512,
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, 512, 1);

bcmode_perftest(tf1024_ctr_e, "Threefish 1024 CTR", BLK_CIPHER_THREEFISH_1024,
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, 1024, 1);

bcmode_perftest(tf256_ctr_ek, "Threefish 256 CTR Keystream",
	BLK_CIPHER_THREEFISH_256, BLK_CIPHER_MODE_CTR,
	BLK_CIPHER_KEY_ENCRYPT, 256, 0);

bcmode_perftest(tf512_ctr_ek, "Threefish 512 CTR Keystream",
	BLK_CIPHER_THREEFISH_512, BLK_CIPHER_MODE_CTR,
	BLK_CIPHER_KEY_ENCRYPT, 512, 0);

bcmode_perftest(tf1024_ctr_ek, "Threefish 1024 CTR Keystream",
	BLK_CIPHER_THREEFISH_1024,BLK_CIPHER_MODE_CTR,
	BLK_CIPHER_KEY_ENCRYPT, 1024, 0);
