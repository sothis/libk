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
