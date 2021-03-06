/*
 * libk - ntwrap.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _NTWRAP_H
#define _NTWRAP_H

#include <stddef.h>
#include <sys/types.h>
#include <direct.h>

#define lseek lseek64

#ifndef fstat
#define fstat fstat64
#endif

#ifndef stat
#define stat stat64
#endif

#define off_t off64_t

#define MAP_FAILED	((void*)~0ul)
#define PROT_READ       0x01
#define PROT_WRITE      0x02
#define MAP_PRIVATE     0x02
#define _SC_PAGE_SIZE	0x1e

#define WC_ERR_INVALID_CHARS	0x00000080

long sysconf(int name);
int mkstemp(char *template);
int fchmod(int fildes, mode_t mode);
int fsync(int fd);
void* mmap(void* start, size_t length, int prot, int flags, int fd, off_t off);
int munmap(void* start, size_t length);

wchar_t* utf8_to_ucs2(const char* utf8_str);
char* ucs2_to_utf8(const wchar_t* ucs2_str);

#endif /* _NTWRAP_H */
