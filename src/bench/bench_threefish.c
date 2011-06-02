#include "utils/benchmark_desc.h"
#include <libk/libk.h>

bcmode_perftest(tf256_ctr_e, "Threefish 256 CTR", BLK_CIPHER_THREEFISH_256,
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, 256);

bcmode_perftest(tf512_ctr_e, "Threefish 512 CTR", BLK_CIPHER_THREEFISH_512,
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, 512);

bcmode_perftest(tf1024_ctr_e, "Threefish 1024 CTR", BLK_CIPHER_THREEFISH_1024,
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, 1024);
