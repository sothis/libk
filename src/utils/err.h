/*
 * libk - err.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _ERR_H
#define _ERR_H

#include <libk/libk.h>

#define k_error(e)	k_post_error(e, K_LFATAL, __LINE__, __FILE__)
#define k_warn(e)	k_post_error(e, K_LWARN, __LINE__, __FILE__)

void k_post_error(enum k_error_e, enum k_errorlevel_e, const int, const char*);

typedef int	kerror_t;

extern kerror_t* _kerrno(void);
#define	kerrno		(*_kerrno())

/* note: these functions aren't locale-dependant in constrast to their stdc
 * counterparts */
extern void kperror(const char* str);
/* note: we return const char* in contrast to the strerror() function */
extern const char* kstrerror(kerror_t errnum);

#endif /* _ERR_H */
