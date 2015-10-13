/*
 * libk - ioutils.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <libk/libk.h>
#include "utils/sections.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef __WINNT__
#include <windows.h>
#include <conio.h>
#include "ntwrap.h"
#else
#include <ftw.h>
#include <termios.h>
#endif

#if defined(__DARWIN__)
#define O_NOATIME		0
#define FTW_CONTINUE		0
#define FTW_STOP		~0
#define FTW_ACTIONRETVAL	0
#elif defined(__WINNT__)
#define O_NOATIME	_O_BINARY
#endif

__attribute__((unused))
static int getln(char** lineptr, size_t* n, FILE* stream)
{
	int nchars_avail;
	char* read_pos;
	int ret;

	if (!lineptr || !n || !stream) {
		errno = EINVAL;
		return -1;
	}

	if (!*lineptr) {
		*n = 64;
		*lineptr = malloc(*n);
		if (!*lineptr) {
			errno = ENOMEM;
			return -1;
		}
		*lineptr[0] = '\0';
	}

	nchars_avail = *n;
	read_pos = *lineptr;

	while(1) {
		int save_errno;
		int c;
		c = getc(stream);
		save_errno = errno;
		if (nchars_avail < 2) {
			if (*n > 64)
				*n *= 2;
			else
				*n += 64;

			nchars_avail = *n + *lineptr - read_pos;
			*lineptr = realloc(*lineptr, *n);
			if (!*lineptr) {
				errno = ENOMEM;
				return -1;
			}
			read_pos = *n - nchars_avail + *lineptr;
		}

		if (ferror(stream)) {
			errno = save_errno;
			return -1;
		}
		if (c == EOF) {
			if (read_pos == *lineptr)
				return -1;
			else
				break;
		}
		*read_pos++ = c;
		nchars_avail--;
		if (c == '\n')
			break;
	}
	*read_pos = '\0';
	ret = read_pos - (*lineptr);
	return ret;
}

__export_function int k_tcreate_dirs(const char* path)
{
	#ifdef __WINNT__
	wchar_t* wc = 0;
	#endif
	int res = 0;
	char* cur = 0;
	char* p = 0;

	cur = getcwd(0, 0);
	if (!cur)
		goto err;

	p = calloc(strlen(path)+1, sizeof(char));
	if(!p)
		goto err;
	strcpy(p, path);

	char* t = p;
	if (!t)
		goto err;

	size_t n_dirs = 0;
	while (*t) {
		if (*t == '/')
			n_dirs++;
		t++;
	}

	char* c = strtok(p, "/");
	size_t n_done = 0;
	while (c && n_done < n_dirs) {
		#ifndef __WINNT__
		mkdir(c, 0700);
		if (chdir(c))
			goto err;
		#else
		wc = utf8_to_ucs2(c);
		if (!wc)
			goto err;
		_wmkdir(wc);
		if (_wchdir(wc))
			goto err;
		free(wc);
		wc = 0;
		#endif
		c = strtok(0, "/");
		n_done++;
	}

	if (chdir(cur))
		goto err;

	res = 0;
	goto out;
err:
	res = -1;
out:
	#ifdef __WINNT__
	if (wc)
		free(wc);
	#endif
	if (cur)
		free(cur);
	if (p)
		free(p);
	return res;
}

#ifdef __WINNT__
/* TODO: allocate memory dynamically; use a stack for storing subdirs in
 * order to process this iterative rather then recursive */
__export_function int
k_ftw(const char* path, int(ftw_fn)(const char* path, size_t baseoff))
{
	char d[32768];
	char mask[32768];
	HANDLE h;
	WIN32_FIND_DATAW fdat;

	snprintf(mask, 32767, "%s/*.*", path);
	wchar_t* wcmask = utf8_to_ucs2(mask);

	h = FindFirstFileW(wcmask, &fdat);
	free(wcmask);

	while (h != INVALID_HANDLE_VALUE) {
		if(fdat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			char* mbfile = ucs2_to_utf8(fdat.cFileName);
			if(strcmp(mbfile, ".") && strcmp(mbfile, "..")) {
				snprintf(d, 32767, "%s/%s", path, mbfile);
				k_ftw(d, ftw_fn);
			}
			free(mbfile);
		}
		else {
			char* mbfile = ucs2_to_utf8(fdat.cFileName);
			snprintf(d, 32767, "%s/%s", path, mbfile);
			ftw_fn(d, strlen(path)+1);
		}
		if (!FindNextFileW(h, &fdat))
			break;
	}
	FindClose(h);
	return 0;
}
#else

/* TODO: make this threadlocal */
static int(*_ftw)(const char*, size_t) = 0;

static int
ft_walk(const char* path, const struct stat* sb, int type, struct FTW* ftw)
{
#if defined(__muslgcc__)
	return -1;
#else
	/* only match regular files (hardlinks currently remain as copies) */
	if (type != FTW_F)
		return FTW_CONTINUE;
	if (!S_ISREG(sb->st_mode))
		return FTW_CONTINUE;

	return _ftw(path, ftw->base);
#endif
}

__export_function int
k_ftw(const char* path, int(ftw_fn)(const char* path, size_t baseoff))
{
#if defined(__muslgcc__)
	return -1;
#else
	int res = 0;
	_ftw = ftw_fn;
	/* stay within the same filesystem, do not follow symlinks */
	res = nftw(path, ft_walk, 128, FTW_ACTIONRETVAL|FTW_MOUNT|FTW_PHYS);
	return res;
#endif
}
#endif


/* TODO: return a handle to password memory and adjust apis to make use of it,
 * the password memory shall be locked and on randomized heap,
 * maybe encrypted with a session key. use this directly in the key derivation
 * abstraction. check if the linux key retention api is helpful here. */
#ifdef __WINNT__
__export_function char* k_get_pass(const char* prompt)
{
	/* TODO: this is rubbish. implement something better */
	char* pass = calloc(1, 4097);
	int i = 0;
	char a;

	if (!pass)
		return 0;

	fprintf(stdout, "%s", prompt);
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
	fprintf(stdout, "\n");
	fflush(stdout);
	fflush(stdin);
	return pass;
}
#else
__export_function char* k_get_pass(const char* prompt)
{
	size_t n = 1024;
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

	fprintf(stdout, "%s", prompt);
	fflush(stdout);
	fflush(stdin);
	nread = getln(&pass, &n, stdin);

	tcsetattr(fileno(stdin), TCSAFLUSH, &old);
	fprintf(stdout, "\n");
	fflush(stdout);
	fflush(stdin);

	if (nread == -1) {
		free(pass);
		return 0;
	}
	pass[nread-1] = 0;
	return pass;
}
#endif
