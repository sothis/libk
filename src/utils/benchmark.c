/*
 * libk - benchmark.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include "benchmark_desc.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __WINNT__
#include <windows.h>
#endif

section_prologue(__benchmarks, struct benchmark_desc);

__export_function void k_run_benchmarks
(int verbose)
{
	const struct benchmark_desc* tests = section_start(__benchmarks);
	size_t n = section_items(__benchmarks, struct benchmark_desc);
	size_t i = 0;
#ifdef __WINNT__
	timeBeginPeriod(1);
#endif
	for (i = 0; i < n; ++i) {
		const char* details = 0;
		if (verbose) {
			fprintf(stderr, "%s\n", tests[i].description);
		}
		double res = tests[i].run(&details);
		if (verbose) {
			fprintf(stderr, "\t%s: %.2f\n",
				details ? details: "result", res);
		}
	}
}
