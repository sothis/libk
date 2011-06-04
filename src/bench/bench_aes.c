/*
 * libk - bench_aes.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include "utils/benchmark_desc.h"
#include <libk/libk.h>

bcmode_perftest(aes128_ctr_e, "AES 128 CTR", BLK_CIPHER_AES,
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, 128, 1);

bcmode_perftest(aes192_ctr_e, "AES 192 CTR", BLK_CIPHER_AES,
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, 192, 1);

bcmode_perftest(aes128_ctr_ek, "AES 128 CTR Keystream", BLK_CIPHER_AES,
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, 128, 0);

bcmode_perftest(aes192_ctr_ek, "AES 192 CTR Keystream", BLK_CIPHER_AES,
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, 192, 0);
