#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#ifndef __WINNT__
#include <ftw.h>
#else
#include "ntwrap.h"
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
ft_walk(const char *path, const struct stat *sb, int type, struct FTW *ftw)
{
	int r;
	/* only match regular files (hardlinks remain as copies) */
	if (type != FTW_F)
		return FTW_CONTINUE;
	if (!S_ISREG(sb->st_mode))
		return FTW_CONTINUE;
	/* filter hidden items */
	if (!memcmp(path+ftw->base, ".", 1)) {
		printf("warning: not adding hidden file %s\n", path);
		return FTW_CONTINUE;
	}

	printf("importing: %s\n", path);
	if ((r = k_pres_add_file(&_cur_pres, path)) != 0) {
		perror("pres_add_file");
		if (r == -1)
			return FTW_STOP;
	}

	return FTW_CONTINUE;
}
#endif

static int import_directory(const char* directory, const char* filename)
{
	if (k_pres_create(&_cur_pres, filename)) {
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

static int list_resources(const char* filename)
{
	if (k_pres_open(&_cur_pres, filename)) {
		perror("pres_open");
		return -1;
	}

	if (k_pres_list(&_cur_pres)) {
		perror("pres_list");
		return -1;
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
	if (!strcmp(argv[1], "list") && (argc > 2))
		return list_resources(argv[2]);

	return -1;
}
