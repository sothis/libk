/*
 * ktool - main.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <stdio.h>
#include <string.h>
#include <libk/libk.h>

static void print_help(void)
{
	k_print_version();
	fprintf(stderr, " ktool test                          " \
		"- run unittests\n");
	fprintf(stderr, " ktool perf                          " \
		"- run benchmarks\n");
	fprintf(stderr, " ktool version                       " \
		"- print version information\n");
	fprintf(stderr, " ktool help                          " \
		"- print this\n");
}

int main(int argc, char* argv[], char* envp[])
{
	if (argc < 2) {
		print_help();
		return -1;
	}

	if (!strcmp(argv[1], "test")) {
		int failed = k_run_unittests(1);
		if (failed)
			printf("failed unittests: %u\n", failed);
		else
			printf("passed all unittests\n");
		return failed;
	}
	if (!strcmp(argv[1], "perf")) {
		k_run_benchmarks(1);
		return 0;
	}
	if (!strcmp(argv[1], "version")) {
		k_print_version();
		return 0;
	}
	if (!strcmp(argv[1], "help")) {
		print_help();
		return 0;
	}
	print_help();
	return -1;
}
