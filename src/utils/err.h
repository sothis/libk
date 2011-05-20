#ifndef _ERR_H
#define _ERR_H

#include <libk/libk.h>

#define k_error(e) k_post_error(e, K_LFATAL, __LINE__, __FILE__)
#define k_warn(e) k_post_error(e, K_LWARN, __LINE__, __FILE__)

void k_post_error(enum k_error_e, enum k_errorlevel_e, const int, const char*);

#endif /* _ERR_H */
