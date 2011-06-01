/*
 * libk - version.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils/sections.h"

static char __libk_extra_ver[256];
static char __libk_git_ver[256];
static const char* const __libk_version_string = VERSION;
static const char* const __libk_builddate_string = __DATE__;
static const char* const __libk_buildtime_string = __TIME__;
#ifdef NDEBUG
static const int _debug_build = 0;
#else
static const int _debug_build = 1;
#endif

__export_function const char* k_version_string(void)
{
	return __libk_version_string;
}

__export_function uint32_t k_version(const char** extra, const char** git)
{
	uint32_t maj = 0, min = 0, plev = 0;
	const char* s = __libk_version_string;

	const char* e = 0, *g = 0;
	while (*s) {
		if (*s == '-')
			e = s+1;
		if (*s == '+')
			g = s+1;
		s++;
	}

	if (git) {
		memset(__libk_git_ver, 0, 256);
		if (g)
			strncpy(__libk_git_ver, g, 255);
		*git = __libk_git_ver;
	}
	if (extra) {
		memset(__libk_extra_ver, 0, 256);
		if (e)
			strncpy(__libk_extra_ver, e, 255);
		char* ms = __libk_extra_ver;
		while (*ms) {
			if (*ms == '+') {
				*ms = 0;
				break;
			}
			ms++;
		}
		*extra = __libk_extra_ver;
	}

	sscanf(__libk_version_string, "%u.%u.%u", &maj, &min, &plev);
	return (maj*10000) + (min*100) + plev;
}

__export_function uint32_t k_version_major(void)
{
	uint32_t v = k_version(0, 0);
	return v/10000;
}

__export_function uint32_t k_version_minor(void)
{
	uint32_t v = k_version(0, 0);
	return (v % 10000) / 100;
}

__export_function uint32_t k_version_patchlevel(void)
{
	uint32_t v = k_version(0, 0);
	return v % 100;
}

__export_function void k_print_version(void)
{
	fprintf(stderr, "libk %s, %s %s%s\n",
		__libk_version_string,
		__libk_builddate_string,
		__libk_buildtime_string,
		_debug_build ? " (debug build)" : "");
}
