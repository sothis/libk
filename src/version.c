/*
 * libk - version.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <stdio.h>
#include "utils/sections.h"

static const char* const __libk_version_string = VERSION;
static const char* const __libk_builddate_string = __DATE__;
static const char* const __libk_buildtime_string = __TIME__;

__export_function void k_version_print(void)
{
	fprintf(stderr, "libk %s, %s %s\n",
		__libk_version_string,
		__libk_builddate_string,
		__libk_buildtime_string);
}

__export_function const char* k_version_get(void)
{
	return __libk_version_string;
}
