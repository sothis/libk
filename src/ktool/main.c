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

struct pres_file_t _cur_pres;
static int
ft_walk(const char* path, size_t baseoff)
{
	int r;

	if ((r = k_pres_add_file(&_cur_pres, path, baseoff)) != 0) {
		if (r == 1)
			printf("skipped: '%s'\n", path);
		if (r == -1) {
			perror("pres_add_file");
			return -1;
		}
	}
	return 0;
}

static int import_directory
(const char* directory, const char* filename, const char* pass)
{
	struct pres_options_t o = {
		.name		= filename,
		.hashsum 	= HASHSUM_SKEIN_1024,
		.hashsize	= 1024,
	};

	if (pass) {
		o.blockcipher = BLK_CIPHER_THREEFISH_1024;
		o.ciphermode = BLK_CIPHER_STREAMMODE_CTR;
		o.keysize = 1024;
		o.pass = pass;
	}

	if (k_pres_create(&_cur_pres, &o)) {
		perror("pres_create");
		return -1;
	}

	char* cwd = getcwd(0, 0);
	if (!cwd) {
		perror("getcwd");
		return -1;
	}
	if (chdir(directory)) {
		free(cwd);
		perror("chdir");
		return -1;
	}

	printf("importing '%s' ...\n", directory);
	if (k_ftw(".", ft_walk)) {
		free(cwd);
		perror("k_winftw");
		return -1;
	}

	if (k_pres_close(&_cur_pres)) {
		free(cwd);
		perror("pres_close");
		return -1;
	}
	if (chdir(cwd)) {
		free(cwd);
		perror("chdir");
		return -1;
	}
	free(cwd);
	return 0;
}

static int export_all(const char* filename, const char* dir)
{
	char* pass = 0;
	int r = k_pres_needs_pass(filename);
	if (r < 0) {
		perror("k_pres_needs_pass");
		return -1;
	}

	if (r) {
		pass = k_get_pass("enter password  : ");
		if (!pass) {
			perror("get_pass");
			return -1;
		}
	}

	if (k_pres_open(&_cur_pres, filename, pass)) {
		perror("pres_open");
		return -1;
	}
	if (pass)
		free(pass);

	mkdir(dir MKDIR_MODE);
	if (chdir(dir)) {
		perror("chdir");
		exit(1);
	}

	uint64_t e = k_pres_res_count(&_cur_pres);
	printf("exporting %lu items from '%s' ...\n", (long)e, filename);

	for (uint64_t i = 1; i <= e; ++i) {
		const char* name = k_pres_res_name_by_id(&_cur_pres, i, 0);

		if (k_tcreate_dirs(name)) {
			printf("resource %lu: '%s'\n", (long)i, name);
			perror("k_tcreate_dirs");
			continue;
		}

		struct pres_res_t r;
		k_pres_res_by_id(&_cur_pres, &r, i);

		int fd = k_tcreat(name, 0400);
		if (fd == -1) {
			printf("resource %lu: '%s'\n", (long)i, name);
			perror("k_tcreat");
			continue;
		}

		uint64_t s = k_pres_res_size(&r);
		uint64_t mmap_window = 16*1024*1024;
		size_t niter = s / mmap_window;
		size_t nlast = s % mmap_window;


		for (uint64_t i = 0; i < niter; ++i) {
			void* m = k_pres_res_map(&r, mmap_window,
				i*mmap_window);
			size_t total = 0;
			ssize_t nwritten;
			while (total != mmap_window) {
				nwritten = write(fd, m + total,
					mmap_window - total);
				if (nwritten < 0) {
					perror("write");
					k_pres_res_unmap(&r);
					k_trollback_and_close(fd);
					continue;
				}
				total += nwritten;
			}
			k_pres_res_unmap(&r);
		}
		if (nlast) {
			void* m = k_pres_res_map(&r, nlast,
				niter*mmap_window);
			size_t total = 0;
			ssize_t nwritten;
			while (total != nlast) {
				nwritten = write(fd, m + total, nlast - total);
				if (nwritten < 0) {
					perror("write");
					k_pres_res_unmap(&r);
					k_trollback_and_close(fd);
					continue;
				}
				total += nwritten;
			}
			k_pres_res_unmap(&r);
		}
		if (k_tcommit_and_close(fd)) {
			printf("resource %lu: '%s'\n", (long)i, name);
			perror("k_tcommit_and_close");
			continue;
		}

	}

	if (k_pres_close(&_cur_pres)) {
		perror("pres_close");
		return -1;
	}
	return 0;
}

static int exportid(const char* filename, const char* dir, uint64_t id)
{
	char* pass = 0;
	int p = k_pres_needs_pass(filename);
	if (p < 0) {
		perror("k_pres_needs_pass");
		return -1;
	}

	if (p) {
		pass = k_get_pass("enter password  : ");
		if (!pass) {
			perror("get_pass");
			return -1;
		}
	}

	if (k_pres_open(&_cur_pres, filename, pass)) {
		perror("pres_open");
		return -1;
	}
	if (pass)
		free(pass);

	mkdir(dir MKDIR_MODE);
	if (chdir(dir)) {
		perror("chdir");
		exit(1);
	}

	const char* basename;
	k_pres_res_name_by_id(&_cur_pres, id, &basename);

	printf("exporting resource %lu: '%s'\n", (long)id, basename);

	struct pres_res_t r;
	k_pres_res_by_id(&_cur_pres, &r, id);

	int fd = k_tcreat(basename, 0400);
	if (fd == -1) {
		printf("resource %lu: '%s'\n", (long)id, basename);
		perror("k_tcreat");
		exit(1);
	}

	uint64_t s = k_pres_res_size(&r);
	uint64_t mmap_window = 16*1024*1024;
	size_t niter = s / mmap_window;
	size_t nlast = s % mmap_window;

	for (uint64_t i = 0; i < niter; ++i) {
		void* m = k_pres_res_map(&r, mmap_window,
			i*mmap_window);
		size_t total = 0;
		ssize_t nwritten;
		while (total != mmap_window) {
			nwritten = write(fd, m + total,
				mmap_window - total);
			if (nwritten < 0) {
				perror("write");
				k_pres_res_unmap(&r);
				k_trollback_and_close(fd);
				return -1;
			}
			total += nwritten;
		}
		k_pres_res_unmap(&r);
	}
	if (nlast) {
		void* m = k_pres_res_map(&r, nlast,
			niter*mmap_window);
		size_t total = 0;
		ssize_t nwritten;
		while (total != nlast) {
			nwritten = write(fd, m + total, nlast - total);
			if (nwritten < 0) {
				perror("write");
				k_pres_res_unmap(&r);
				k_trollback_and_close(fd);
				return -1;
			}
			total += nwritten;
		}
		k_pres_res_unmap(&r);
	}

	if (k_tcommit_and_close(fd)) {
		printf("resource %lu: '%s'\n", (long)id, basename);
		perror("k_tcommit_and_close");
		return -1;
	}


	if (k_pres_close(&_cur_pres)) {
		perror("pres_close");
		return -1;
	}
	return 0;
}

static int list_all(const char* filename)
{
	char* pass = 0;
	int r = k_pres_needs_pass(filename);
	if (r < 0) {
		perror("k_pres_needs_pass");
		return -1;
	}

	if (r) {
		pass = k_get_pass("enter password  : ");
		if (!pass) {
			perror("get_pass");
			return -1;
		}
	}

	if (k_pres_open(&_cur_pres, filename, pass)) {
		perror("pres_open");
		return -1;
	}
	if (pass)
		free(pass);

	uint64_t e = k_pres_res_count(&_cur_pres);

	for (long i = 1; i <= e; ++i) {
		const char* name = k_pres_res_name_by_id(&_cur_pres, i, 0);
		printf("%lu:\t'%s'\n", i, name);
	}

	if (k_pres_close(&_cur_pres)) {
		perror("pres_close");
		return -1;
	}
	return 0;
}

static void print_help(void)
{
	k_version_print();
	fprintf(stderr, " ktool test                          " \
		"- run unittests\n");
	fprintf(stderr, " ktool ls   <infile>                 " \
		"- list contents of pres container\n");
	fprintf(stderr, " ktool imp  <indir> <outfile>        " \
		"- import directory into pres container\n");
	fprintf(stderr, " ktool imps <indir> <outfile>        " \
		"- import directory into encrypted pres\n");
	fprintf(stderr, " ktool exp  <infile> <outdir>        " \
		"- export everything from pres container\n");
	fprintf(stderr, " ktool expid  <infile> <outdir> <id> " \
		"- export specific id from pres container\n");
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

	if (!strcmp(argv[1], "imp") && (argc > 3)) {
		return import_directory(argv[2], argv[3], 0);
	}
	if (!strcmp(argv[1], "imps") && (argc > 3)) {
		char* p = k_get_pass("enter password  : ");
		if (!p)
			return -1;
		char* p2 = k_get_pass("retype password : ");
		if (!p2) {
			free(p);
			return -1;
		}
		if (strcmp(p, p2)) {
			printf("passwords do not match\n");
			free(p);
			free(p2);
			return -1;
		}
		free(p2);
		int res = import_directory(argv[2], argv[3], p);
		free(p);
		return res;
	}
	if (!strcmp(argv[1], "exp") && (argc > 3))
		return export_all(argv[2], argv[3]);
	if (!strcmp(argv[1], "expid") && (argc > 4))
		return exportid(argv[2], argv[3], strtoull(argv[4], 0, 10));
	if (!strcmp(argv[1], "ls") && (argc > 2))
		return list_all(argv[2]);
	if (!strcmp(argv[1], "test")) {
		int failed = k_run_unittests(1);
		if (failed)
			printf("failed unittests: %u\n", failed);
		else
			printf("passed all unittests\n");
		return failed;
	}
	if (!strcmp(argv[1], "version")) {
		k_version_print();
		return 0;
	}
	if (!strcmp(argv[1], "help")) {
		print_help();
		return 0;
	}
	print_help();
	return -1;
}
