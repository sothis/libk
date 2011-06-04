/*
 * libk - bench_skein.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <libk/libk.h>
#include "utils/benchmark_desc.h"

#if 1

benchmark(skein256, "Skein256-256")
{
	k_hash_t* c = k_hash_init(HASHSUM_SKEIN_256, 256);

	uint8_t _out[256/8];
	double res;
	uint8_t* in_buf;
	uint64_t usec = 0;
	struct timeval b, e;

	in_buf = k_calloc(PROCESS_MB*1048576ul, sizeof(uint8_t));
	gettimeofday(&b, 0);
	k_hash_update(c, in_buf, PROCESS_MB*1048576ul);
	k_hash_final(c, _out);
	gettimeofday(&e, 0);
	usec = ((e.tv_sec*1000000ull + e.tv_usec) -
		(b.tv_sec*1000000ull + b.tv_usec));
	res = ((double)(PROCESS_MB))/((double)usec/1000000.0);
	k_free(in_buf);
	k_hash_finish(c);

	*details = "Throughput MiB/sec";
	return res;
}

#endif

#if 1

benchmark(skein512, "Skein512-512")
{
	k_hash_t* c = k_hash_init(HASHSUM_SKEIN_512, 512);

	uint8_t _out[512/8];
	double res;
	uint8_t* in_buf;
	uint64_t usec = 0;
	struct timeval b, e;

	in_buf = k_calloc(PROCESS_MB*1048576ul, sizeof(uint8_t));
	gettimeofday(&b, 0);
	k_hash_update(c, in_buf, PROCESS_MB*1048576ul);
	k_hash_final(c, _out);
	gettimeofday(&e, 0);
	usec = ((e.tv_sec*1000000ull + e.tv_usec) -
		(b.tv_sec*1000000ull + b.tv_usec));
	res = ((double)(PROCESS_MB))/((double)usec/1000000.0);
	k_free(in_buf);
	k_hash_finish(c);

	*details = "Throughput MiB/sec";
	return res;
}

#endif

#if 1

benchmark(skein1024, "Skein1024-1024")
{
	k_hash_t* c = k_hash_init(HASHSUM_SKEIN_1024, 1024);

	uint8_t _out[1024/8];
	double res;
	uint8_t* in_buf;
	uint64_t usec = 0;
	struct timeval b, e;

	in_buf = k_calloc(PROCESS_MB*1048576ul, sizeof(uint8_t));
	gettimeofday(&b, 0);
	k_hash_update(c, in_buf, PROCESS_MB*1048576ul);
	k_hash_final(c, _out);
	gettimeofday(&e, 0);
	usec = ((e.tv_sec*1000000ull + e.tv_usec) -
		(b.tv_sec*1000000ull + b.tv_usec));
	res = ((double)(PROCESS_MB))/((double)usec/1000000.0);
	k_free(in_buf);
	k_hash_finish(c);

	*details = "Throughput MiB/sec";
	return res;
}

#endif

