/*
 * libk - err.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <libk/libk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "mem.h"
#include "sections.h"

static const char* const _err_messages[] = {
	"success",
	"insufficent memory",
	"specified cipher not found",
	"key schedule/context/state must not be of zero size",
	"key schedule size exceeds allowed maximum locked memory size, "
		"see 'ulimit'",
	"selected cipher does not specify a valid key size or key size range",
	"the specified keysize is not supported by the selected cipher",
	"the cipher or the specified keysize was marked as insecure",
	"unable to create worker threads",
	"no set_encrypt_key/set_decrypt_key or transform implementation "
		"for the specified cipher available",
	"specified mode not found",
	"invalid key type",
	"the requested operation requires a vaild blockcipher mode to be set",
	"invalid tweak size",
	"current blockcipher is not tweakable",
	"specified hashsum not found",
	"hash context must not be of zero size",
	"hash context size exceeds allowed maximum locked memory size, "
		"see 'ulimit'",
	"the hashsum was marked as insecure",
	"cannot open unix random device",
	"file header digest does not match (file corrupt)",
	"detached file header digest does not match\n"
		"(wrong password or file corrupt)",
	"mode is not suitable for streamcipher usage",
	"metadata file exists, container is probably corrupted but can be "
		"repaired",
};

static void _default_err_handler
(enum k_error_e e, enum k_errorlevel_e l, const char* s)
{
	if (l == K_LFATAL) {
		fprintf(stderr, "libk error: %s (code: %u)\n", s, e);
		/* enable this only for debugging purposes */
		/* exit(e); */
	} else {
		fprintf(stderr, "libk warning: %s (code: %u)\n", s, e);
	}
}

static pthread_key_t tls_error_handler;
static void destruct_tls(void* value)
{
	pthread_setspecific(tls_error_handler, NULL);
}

__attribute__((constructor))
static void _init_error_handling(void)
{
	pthread_key_create(&tls_error_handler, &destruct_tls);
}

__attribute__((destructor))
static void _fini_error_handling(void)
{
	pthread_key_delete(tls_error_handler);
}

void k_post_error
(enum k_error_e err, enum k_errorlevel_e lvl, const int line, const char* file)
{
	k_err_fn handle_err = (k_err_fn)pthread_getspecific(tls_error_handler);
	if (!handle_err) {
		pthread_setspecific(tls_error_handler, &_default_err_handler);
		handle_err = &_default_err_handler;
	}
	if (handle_err == &_default_err_handler)
		fprintf(stderr, "%s:%u -> ", file, line);
	handle_err(err, lvl, _err_messages[err]);
}

__export_function void k_set_error_handler
(k_err_fn error_handler)
{
	if (!error_handler)
		pthread_setspecific(tls_error_handler, &_default_err_handler);
	else
		pthread_setspecific(tls_error_handler, error_handler);
}
