/*
 * tfile.h
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

int tcreat(const char* name, mode_t mode);
int tcommit_and_close(int fd);
void trollback_and_close(int fd);

#endif /* _TFILE_H */
