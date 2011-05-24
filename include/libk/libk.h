/*
 * libk - libk.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _LIBK_H
#define _LIBK_H

#include <libk/internal/ktypes.h>
#include <libk/internal/algorithms.h>
#include <libk/internal/modes.h>
#include <libk/internal/errors.h>
#include <libk/internal/hashsums.h>

#include <libk/internal/blockcipher.h>
#include <libk/internal/streamcipher.h>
#include <libk/internal/prng.h>
#include <libk/internal/hash.h>

#include <libk/internal/pres.h>
#include <libk/internal/mempool.h>
#include <libk/internal/tfile.h>

typedef void (*k_err_fn)(enum k_error_e, enum k_errorlevel_e, const char*);

extern void k_set_error_handler
(k_err_fn error_handler);

extern int k_run_unittests
(int verbose);


#endif /* _LIBK_H */
