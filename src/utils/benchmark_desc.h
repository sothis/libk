#ifndef _BENCHMARK_DESC_H
#define _BENCHMARK_DESC_H

#include "sections.h"
#include "mem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/time.h>

typedef double (*benchmark_fn)(const char**);
#define _benchmark_code_						\
	__attribute__((section(_CODE_SEGMENT "__benchmarkcode")))	\
	static
#define _benchmark_data_						\
	__attribute__((section(_DATA_SEGMENT "__benchmarkdata"),	\
	_section_alignment)) static const
#define _benchmark_entry_						\
	__attribute__((section(_DATA_SEGMENT "__benchmarks"),		\
	used, _section_alignment, externally_visible)) const

struct benchmark_desc {
	const char*		description;
	const benchmark_fn	run;
} __attribute__((_section_alignment));

#define benchmark(_name, _description)					\
	static double benchmark_##_name(const char** details);		\
	_benchmark_entry_ struct benchmark_desc				\
	__benchmark_##_name = {						\
		.description	= _description,				\
		.run		= &benchmark_##_name			\
	};								\
	_benchmark_code_ double benchmark_##_name(const char** details)


/* benchmark stubs */

#define PROCESS_MB 32
#define bcmode_perftest(_name, _desc, _cipher, _mode, _type, _keysize)	\
benchmark(_name, _desc)							\
{									\
	k_bc_t* c;							\
	double res;							\
	uint8_t* iv_buf;						\
	uint8_t* key_buf;						\
	uint8_t* in_buf;						\
	uint32_t blocksize, keysize = _keysize/8;			\
	uint64_t usec = 0;						\
	struct timeval b, e;						\
	c = k_bc_init(_cipher);						\
	k_bcmode_set_mode(c, _mode, 0);					\
	blocksize = k_bc_get_blocksize(c);				\
	iv_buf = k_calloc(blocksize, sizeof(uint8_t));			\
	key_buf = k_calloc(keysize, sizeof(uint8_t));			\
	in_buf = k_calloc(PROCESS_MB*1048576ul, sizeof(uint8_t));	\
	k_bcmode_set_key(c, key_buf, keysize*8, _type);			\
	k_bcmode_set_iv(c, iv_buf);					\
	k_free(iv_buf);							\
	k_free(key_buf);						\
	k_bcmode_update(c, in_buf, in_buf,				\
		PROCESS_MB*1048576ul / blocksize);			\
	gettimeofday(&b, 0);						\
	k_bcmode_update(c, in_buf, in_buf,				\
		PROCESS_MB*1048576ul / blocksize);			\
	gettimeofday(&e, 0);						\
	usec = ((e.tv_sec*1000000ull + e.tv_usec) -			\
		(b.tv_sec*1000000ull + b.tv_usec));			\
	res = ((double)(PROCESS_MB))/((double)usec/1000000.0);		\
	k_bc_finish(c);							\
	k_free(in_buf);							\
	*details = "Throughput MiB/sec";				\
	return res;							\
}

#endif /* _BENCHMARK_DESC_H */
