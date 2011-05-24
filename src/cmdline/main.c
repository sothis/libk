/*
 * nktool - main.c
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

#ifndef __WINNT__
#include <ftw.h>
#define MKDIR_MODE ,0700
#else
#include "utils/ntwrap.h"
#define MKDIR_MODE
#endif

#include <libk/libk.h>

#ifndef __WINNT__
static void __term_handler(int sig, siginfo_t* info, void* unused)
{
	exit(0);
}
#endif

#ifdef __DARWIN__
#define FTW_CONTINUE		0
#define FTW_STOP		~0
#define FTW_ACTIONRETVAL	0
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
#endif
}

struct pres_file_t _cur_pres;

#ifndef __WINNT__
static int
ft_walk(const char* path, const struct stat* sb, int type, struct FTW* ftw)
{
	int r;
	/* only match regular files (hardlinks currently remain as copies) */
	if (type != FTW_F)
		return FTW_CONTINUE;
	if (!S_ISREG(sb->st_mode))
		return FTW_CONTINUE;

	if ((r = k_pres_add_file(&_cur_pres, path, ftw->base)) != 0) {
		if (r == 1)
			printf("skipped: %s\n", path);
		if (r == -1) {
			perror("pres_add_file");
			return FTW_STOP;
		}
	} else
		printf("imported: %s\n", path);

	return FTW_CONTINUE;
}
#endif

static int import_directory(const char* directory, const char* filename)
{
	struct pres_options_t o = {
		.name		= filename,
		.hashsum 	= HASHSUM_SKEIN_1024,
		.hashsize	= 1024,
#if 1
		.blockcipher	= BLK_CIPHER_THREEFISH_1024,
		.ciphermode	= BLK_CIPHER_STREAMMODE_CTR,
		.keysize	= 1024,
		.key		=	"00000000000000000000000000000000"
					"00000000000000000000000000000000"
					"00000000000000000000000000000000"
					"00000000000000000000000000000000"
#endif
	};

	if (k_pres_create(&_cur_pres, &o)) {
		perror("pres_create");
		return -1;
	}
#ifndef __WINNT__
	/* stay within the same filesystem, do not follow symlinks */
	if (nftw(directory, ft_walk, 128, FTW_ACTIONRETVAL|FTW_MOUNT|FTW_PHYS))
		return -1;
#endif
	if (k_pres_commit_and_close(&_cur_pres)) {
		perror("pres_commit_and_close");
		return -1;
	}
	return 0;
}

static void create_dirs(const char* path)
{
	char* cur = getcwd(0, 0);
	char* p = calloc(strlen(path)+1, sizeof(char));
	strcpy(p, path);

	char* t = p;
	size_t n_dirs = 0;
	while (*t) {
		if (*t == '/')
			n_dirs++;
		t++;
	}

	char* c = strtok(p, "/");
	size_t n_done = 0;
	while (c && n_done < n_dirs) {
		mkdir(c MKDIR_MODE);
		if (chdir(c)) {
			perror("chdir");
			exit(1);
		}
		c = strtok(0, "/");
		n_done++;
	}
	free(p);
	if (chdir(cur)) {
		perror("chdir");
		exit(1);
	}
	free(cur);
}

static int export_all(const char* filename, const char* dir)
{
	const char* key =	"00000000000000000000000000000000"
				"00000000000000000000000000000000"
				"00000000000000000000000000000000"
				"00000000000000000000000000000000";

	if (k_pres_open(&_cur_pres, filename, key)) {
		perror("pres_open");
		return -1;
	}

	mkdir(dir MKDIR_MODE);
	if (chdir(dir)) {
		perror("chdir");
		exit(1);
	}

	uint64_t e = k_pres_res_count(&_cur_pres);

next:
	for (uint64_t i = 1; i <= e; ++i) {
		const char* name = k_pres_res_name_by_id(&_cur_pres, i);
		printf("exporting '%s'\n", name);

		create_dirs(name);

		struct pres_res_t r;
		k_pres_res_by_id(&_cur_pres, &r, i);

		int fd = tcreat(name, 0400);
		if (fd == -1) {
			perror("tcreat");
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
					trollback_and_close(fd);
					goto next;
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
					trollback_and_close(fd);
					goto next;
				}
				total += nwritten;
			}
			k_pres_res_unmap(&r);
		}

		tcommit_and_close(fd);
	}

	if (k_pres_close(&_cur_pres)) {
		perror("pres_close");
		return -1;
	}
	return 0;
}

int main(int argc, char* argv[], char* envp[])
{
	__init();

	if (argc < 2)
		return -1;

	if (!strcmp(argv[1], "import") && (argc > 3)) {
		size_t s = strlen(argv[2]);
		/* TODO: introduce failsafe path resolution */
		/* remove trailing slashes */
		if (s > 0 && argv[2][s-1] == '/') {
			char* last = argv[2] + s - 1;
			while (*last == '/') {
				*last = 0;
				last--;
			}
		}
		return import_directory(argv[2], argv[3]);
	}
	if (!strcmp(argv[1], "export-all") && (argc > 3))
		return export_all(argv[2], argv[3]);
	if (!strcmp(argv[1], "--test")) {
		int failed = k_run_unittests(1);
		if (failed)
			printf("failed unittests: %u\n", failed);
		else
			printf("passed all unittests\n");
		return failed;
	}
	return -1;
}
