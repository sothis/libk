/*
 * pres - main.c
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
#include "ntwrap.h"
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
		.hashsum 	= HASHSUM_SKEIN_512,
		.hashsize	= 512,
		.blockcipher	= BLK_CIPHER_THREEFISH_1024,
		.ciphermode	= BLK_CIPHER_STREAMMODE_CTR,
		.keysize	= 1024,
		.key		=	"00000000000000000000000000000000"
					"00000000000000000000000000000000"
					"00000000000000000000000000000000"
					"00000000000000000000000000000000"
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
		chdir(c);
		c = strtok(0, "/");
		n_done++;
	}
	free(p);
	chdir(cur);
	free(cur);
}

static int export_all(const char* filename, const char* dir)
{
	if (k_pres_open(&_cur_pres, filename)) {
		perror("pres_open");
		return -1;
	}

	mkdir(dir MKDIR_MODE);
	chdir(dir);

	uint64_t e = k_pres_res_count(&_cur_pres);

	for (uint64_t i = 1; i <= e; ++i) {
		const char* name = k_pres_res_name_by_id(&_cur_pres, i);
		printf("exporting '%s'\n", name);

		create_dirs(name);

		struct pres_res_t r;
		k_pres_res_by_id(&_cur_pres, &r, i);
		uint64_t s = k_pres_res_size(&r);
		void* m = k_pres_res_map(&r, 0, 0);

		int fd = open(name, O_RDWR|O_CREAT, 0400);
		if (fd == -1) {
			perror("open");
			exit(1);
		}

		size_t total = 0;
		ssize_t nwritten;
		while (total != s) {
			nwritten = write(fd, m + total, s - total);
			if (nwritten < 0) {
				close(fd);
				perror("write");
				exit(1);
			}
			total += nwritten;
		}
		fsync(fd);
		close(fd);
		k_pres_res_unmap(&r);
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

	if (!strcmp(argv[1], "import") && (argc > 3))
		return import_directory(argv[2], argv[3]);
	if (!strcmp(argv[1], "export-all") && (argc > 3))
		return export_all(argv[2], argv[3]);

	return -1;
}
