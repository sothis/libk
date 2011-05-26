/*
 * libk - tfile.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _TFILE_H
#define _TFILE_H

#include <sys/types.h>

extern int k_tcreat(const char* name, mode_t mode);
extern int k_tcommit_and_close(int fd);
extern void k_trollback_and_close(int fd);

extern int k_tcreate_dirs(const char* path);

#endif /* _TFILE_H */
