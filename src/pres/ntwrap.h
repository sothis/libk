#ifndef _NTWRAP_H
#define _NTWRAP_H

#include <stddef.h>
#include <sys/types.h>

#define MAP_FAILED	((void*)~0ul)
#define PROT_READ       0x01
#define MAP_PRIVATE     0x02
#define _SC_PAGE_SIZE	0x1e

long sysconf(int name);
int mkstemp(char *template);
int fchmod(int fildes, mode_t mode);
int fsync(int fd);
void* mmap(void* start, size_t length, int prot, int flags, int fd, off_t off);
int munmap(void* start, size_t length);

#endif /* _NTWRAP_H */
