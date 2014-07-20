/*
 * ktool - main.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>

#ifndef __WINNT__
#define MKDIR_MODE ,0700
#else
#include <windows.h>
#define MKDIR_MODE
#endif

#include <libk/libk.h>

#ifndef __WINNT__
static void __term_handler(int sig, siginfo_t* info, void* unused)
{
	printf("\n");
	exit(0);
}
#endif


#ifdef __WINNT__
static uint32_t oldicp, oldocp;
static void __cleanup(void)
{
	SetConsoleOutputCP(oldocp);
	SetConsoleCP(oldicp);
}
#endif

static void __init(void)
{
#ifndef __WINNT__
	struct sigaction sa;

	if (sigemptyset(&sa.sa_mask))
		exit(1);

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = __term_handler;

	/* SIGPIPE, SIGABRT and friends are considered as bugs. no cleanup
	 * in these cases, fix the program or replace defective
	 * hardware :) */
	if (sigaction(SIGHUP, &sa, 0))
		exit(1);
	if (sigaction(SIGINT, &sa, 0))
		exit(1);
	if (sigaction(SIGQUIT, &sa, 0))
		exit(1);
	if (sigaction(SIGTERM, &sa, 0))
		exit(1);
#else
	atexit(__cleanup);
	oldicp = GetConsoleCP();
	oldocp = GetConsoleOutputCP();
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif
}


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
	__init();

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
