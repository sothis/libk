/*
 * libk - ioutils.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _IOUTILS_H
#define _IOUTILS_H

#include <stddef.h>

extern int k_tcreate_dirs(const char* path);

extern int k_ftw
(const char* path, int(ftw_fn)(const char* path, size_t baseoff));

extern char* k_get_pass(const char* prompt);

#endif /* _IOUTILS_H */
