#include <libk/libk.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern int run_unittests(int verbose);

int32_t main(int32_t argc, char* argv[], char* envp[])
{
	if (argc < 2)
		return -1;

	if (!strcmp(argv[1], "--libktest")) {
		int failed = run_unittests(1);
		if (failed)
			printf("failed unittests: %u\n", failed);
		else
			printf("passed all unittests\n");
		return failed;
	}

#if 0
	uint8_t mt_seed[8] = { 0x01, 0x23, 0x02, 0x34, 0x03, 0x45, 0x04, 0x56 };
	k_prng_t* mt = k_prng_init(PRNG_MT19937_32);

	k_prng_set_seed(mt, mt_seed, sizeof(mt_seed));
	for (size_t i = 0; i < 64; i++) {
		printf("%.10u ", k_prng_get_uint32(mt));
		if (i % 2 == 1 || i == 63)
			printf("\n");
	}
#if 0
	k_prng_set_seed(mt, mt_seed, sizeof(mt_seed));
	for (size_t i = 0; i < 64; i++) {
		printf("%.20llu ", (unsigned long long)k_prng_get_uint64(mt));
		if (i % 2 == 1 || i == 63)
			printf("\n");
	}
#endif
	k_prng_set_seed(mt, mt_seed, sizeof(mt_seed));
	for (size_t i = 0; i < 64; i++) {
		printf("%.3u ", k_prng_get_uint8(mt));
		if (i % 2 == 1 || i == 63)
			printf("\n");
	}
	k_prng_finish(mt);
#endif
	return 0;
}
