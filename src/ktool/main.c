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

static struct pres_file_t* _open_pres_container
(const char* filename, int writable)
{
	struct pres_file_t* p = malloc(sizeof(struct pres_file_t));
	if (!p)
		return 0;

	char* pass = 0;
	int r = k_pres_needs_pass(filename);
	if (r < 0)
		goto err_out;

	if (r) {
		pass = k_get_pass("enter password  : ");
		if (!pass)
			goto err_out;
	}

	if (k_pres_open(p, filename, pass, writable))
		goto err_out;

	goto out;
err_out:
	if (p) {
		free(p);
		p = 0;
	}
out:
	if (pass)
		free(pass);
	return p;
}

static int _close_pres_container(struct pres_file_t* p)
{
	int res = 0;
	if (k_pres_close(p))
		res = -1;
	free(p);
	return res;
}


static struct pres_file_t _cur_pres;
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
	};

	if (pass) {
		o.blockcipher = BLK_CIPHER_THREEFISH_1024;
		o.ciphermode = BLK_CIPHER_MODE_CTR;
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

static int add_file
(const char* container, const char* filename, const char* pass)
{
	int res = 0;
	struct pres_file_t* p = _open_pres_container(container, 1);
	if (!p)
		goto err_out;


	goto out;
err_out:
	perror("exportid");
	res = -1;
out:
	if (p)
		_close_pres_container(p);
	return res;
}

static int export_all(const char* filename, const char* dir)
{
	int res = 0;
	struct pres_file_t* p = _open_pres_container(filename, 0);

	if (!p)
		goto err_out;


	mkdir(dir MKDIR_MODE);
	if (chdir(dir))
		goto err_out;

	uint64_t e = k_pres_res_count(p);
	printf("exporting %lu items from '%s' ...\n", (long)e, filename);

	for (long i = 1; i <= e; ++i) {
		k_pres_export_id(p, i, 1);
	}

	goto out;

err_out:
	perror("export_all");
	res = -1;
out:
	if (p)
		_close_pres_container(p);
	return res;

}

static int exportid(const char* filename, const char* dir, uint64_t id)
{
	const char* basename;
	int res = 0;
	struct pres_file_t* p = _open_pres_container(filename, 0);

	if (!p)
		goto err_out;

	mkdir(dir MKDIR_MODE);
	if (chdir(dir))
		goto err_out;

	k_pres_res_name_by_id(p, id, &basename);
	printf("exporting %lu:\t'%s'\n", (long)id, basename);

	if (k_pres_export_id(p, id, 0))
		goto err_out;

	goto out;
err_out:
	perror("exportid");
	res = -1;
out:
	if (p)
		_close_pres_container(p);
	return res;
}

static int list_all(const char* filename)
{
	int res = 0;
	struct pres_file_t* p = _open_pres_container(filename, 0);

	if (!p)
		goto err_out;

	uint64_t e = k_pres_res_count(p);
	for (long i = 1; i <= e; ++i) {
		const char* name = k_pres_res_name_by_id(p, i, 0);
		printf("%lu:\t'%s'\n", i, name);
	}

	goto out;
err_out:
	perror("list_all");
	res = -1;
out:
	if (p)
		_close_pres_container(p);
	return res;
}

static void print_help(void)
{
	k_print_version();
	fprintf(stderr, " ktool test                          " \
		"- run unittests\n");
	fprintf(stderr, " ktool perf                          " \
		"- run benchmarks\n");
	fprintf(stderr, " ktool ls    <inpres>                " \
		"- list contents of pres container\n");
	fprintf(stderr, " ktool imp   <indir> <outpres>       " \
		"- import directory to new pres container\n");
	fprintf(stderr, " ktool add   <inpres> <infile>       " \
		"- add single file to pres container\n");
	fprintf(stderr, " ktool imps  <indir> <outpres>       " \
		"- import directory to new encrypted pres\n");
	fprintf(stderr, " ktool exp   <inpres> <outdir>       " \
		"- export everything from pres container\n");
	fprintf(stderr, " ktool expid <inpres> <outdir> <id>  " \
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
	if (!strcmp(argv[1], "add") && (argc > 3)) {
		return add_file(argv[2], argv[3], 0);
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
