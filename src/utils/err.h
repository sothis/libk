/*
 * err.h
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

#define k_error(e) k_post_error(e, K_LFATAL, __LINE__, __FILE__)
#define k_warn(e) k_post_error(e, K_LWARN, __LINE__, __FILE__)

void k_post_error(enum k_error_e, enum k_errorlevel_e, const int, const char*);

#endif /* _ERR_H */
