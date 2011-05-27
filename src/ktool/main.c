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

#ifndef __WINNT__
#include <ftw.h>
#include <termios.h>
#define MKDIR_MODE ,0700

extern int getln(char** lineptr, size_t* n, FILE* stream);
#else
#include <windows.h>
#include <conio.h>
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

	printf("importing: %s\n", path);
	if ((r = k_pres_add_file(&_cur_pres, path, ftw->base)) != 0) {
		if (r == 1)
			printf("\tskipped\n");
		if (r == -1) {
			perror("pres_add_file");
			return FTW_STOP;
		}
	} else
		printf("\tsuccess\n");

	return FTW_CONTINUE;
}

char* get_pass(const char* prompt)
{
	size_t n = 1024;
	/* TODO: when moving this into libk, use locked memory */
	char* pass = calloc(n+1, 1);
	struct termios old, new;
	int nread;


	/* TODO: this is error-prone when sending a signal while being
	 * in getline(). */
	if (tcgetattr(fileno(stdin), &old) != 0)
		return 0;
	new = old;
	new.c_lflag &= ~ECHO;
	if (tcsetattr(fileno(stdin), TCSAFLUSH, &new) != 0)
		return 0;

	printf("%s", prompt);
	fflush(stdout);
	fflush(stdin);
	nread = getln(&pass, &n, stdin);

	tcsetattr(fileno(stdin), TCSAFLUSH, &old);
	printf("\n");
	fflush(stdout);
	fflush(stdin);

	if (nread == -1) {
		printf("can't create safe password prompt\n");
		free(pass);
		return 0;
	}
	pass[nread-1] = 0;
	return pass;
}
#else

static int
winftw(const char* path)
{
	int r;
	char d[65536];
	char mask[65536];
	HANDLE h;
	WIN32_FIND_DATA fdat;

	snprintf(mask, 65535, "%s/*.*", path);

	h = FindFirstFile(mask, &fdat);

	while (h != INVALID_HANDLE_VALUE) {
		if(fdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if(strcmp(fdat.cFileName, ".") &&
			strcmp(fdat.cFileName, "..")) {
				snprintf(d, 65535, "%s/%s", path,
					fdat.cFileName);
				winftw(d);
			}
		}
		else {
			snprintf(d, 65535, "%s/%s", path, fdat.cFileName);
			printf("importing: %s\n", d);
			if ((r = k_pres_add_file(&_cur_pres, d, strlen(path)+1))
			!= 0) {
				if (r == 1)
					printf("\tskipped\n");
				if (r == -1) {
					perror("pres_add_file");
					exit(1);
				}
			} else
				printf("\tsuccess\n");
		}
		if (!FindNextFile(h, &fdat))
			break;
	}
	FindClose(h);
	return 0;
}

char* get_pass(const char* prompt)
{
	/* TODO: this is rubbish. implement something better */
	/* TODO: when moving this into libk, use locked memory */
	char* pass = calloc(1, 4097);
	int i = 0;
	char a;

	if (!pass)
		return 0;

	printf("%s", prompt);
	fflush(stdout);
	while(i <= 4096) {
		fflush(stdin);
		a = getch();
		if (a > 47 && a < 123) {
			pass[i]=a;
			i++;
		} else if(a == 8) {
			if(i > 0)
				i--;
		} else if(a == 13) {
			pass[i] = 0;
			break;
		}
	}
	printf("\n");
	fflush(stdout);
	fflush(stdin);
	return pass;
}
#endif

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
#ifndef __WINNT__
	/* stay within the same filesystem, do not follow symlinks */
	if (nftw(".", ft_walk, 128, FTW_ACTIONRETVAL|FTW_MOUNT|FTW_PHYS)) {
		free(cwd);
		perror("nftw");
		return -1;
	}
#else
	if (winftw(".")) {
		free(cwd);
		perror("winftw");
		return -1;
	}
#endif
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
		pass = get_pass("enter password  : ");
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

next:
	for (uint64_t i = 1; i <= e; ++i) {
		const char* name = k_pres_res_name_by_id(&_cur_pres, i);
		printf("exporting '%s'\n", name);

		if (k_tcreate_dirs(name)) {
			perror("k_tcreate_dirs");
			exit(1);
		}

		struct pres_res_t r;
		k_pres_res_by_id(&_cur_pres, &r, i);

		int fd = k_tcreat(name, 0400);
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
					k_trollback_and_close(fd);
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
					k_trollback_and_close(fd);
					goto next;
				}
				total += nwritten;
			}
			k_pres_res_unmap(&r);
		}

		k_tcommit_and_close(fd);
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
	fprintf(stderr, " ktool test                    " \
		"- run unittests\n");
	fprintf(stderr, " ktool imp  <indir> <outfile>  " \
		"- import directory into pres container\n");
	fprintf(stderr, " ktool imps <indir> <outfile>  " \
		"- import directory into encrypted pres container\n");
	fprintf(stderr, " ktool exp  <infile> <outdir>  " \
		"- export everything from pres container\n");
	fprintf(stderr, " ktool version                 " \
		"- print version information\n");
	fprintf(stderr, " ktool help                    " \
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
		char* p = get_pass("enter password  : ");
		if (!p)
			return -1;
		char* p2 = get_pass("retype password : ");
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
