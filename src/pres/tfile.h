#ifndef _TFILE_H
#define _TFILE_H

#include <sys/types.h>

int tcreat(const char* name, mode_t mode);
int tcommit_and_close(int fd);
void trollback_and_close(int fd);

#endif /* _TFILE_H */
