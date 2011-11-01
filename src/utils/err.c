/*
 * libk - err.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include "err.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>

#ifdef __LINUX__
#include <execinfo.h>
#endif

#include "mem.h"
#include "sections.h"

/* a sample how to use kerrno in a library function, which uses itself stdc
 * library calls */
kerror_t _kerrno_sample_function(int* res, int parm_a, int parm_b)
{
	/* always set kernno to 0 as very first instruction */
	kerrno = K_ESUCCESS;

	/* locals */
	void* some_mem = 0;

	/* set kerrno to errno if some stdc function fails */
	some_mem = malloc(5);
	if (!some_mem) {
		kerrno = errno;
		goto out;
	}

	/* set kerrno to custom error code > 4095, if some own code wants
	 * to fail */
	if (!parm_b) {
		kerrno = K_ESCHEDZERO;
		goto out;
	}

	*res = parm_a/parm_b;

out:
	if (some_mem)
		free(some_mem);
	/* always return -kerrno here if return type is kerror_t
	 * or zero if return type is a pointer. if return type is an integral
	 * value (i.e. it has a semantic meaning) the caller has to check
	 * kerrno manually, or, if the only valid values are positive, return
	 * the result and -kerrno selectively. */
	return -kerrno;
}

/* a sample how to use kerrno in a library function, which uses itself a
 * function returning kerror_t */
kerror_t _kerrno_sample_function2(int* res)
{
	/* always set kernno to 0 as very first instruction */
	kerrno = K_ESUCCESS;

	/* locals */
	int a = 5, b = 0, c = 0;

	/* don't re-assign kerrno here! just pass through. */
	if (_kerrno_sample_function(&c, a, b))
		goto out;

	*res = c;
out:
	return -kerrno;
}


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
	fflush(stderr);
}

static pthread_key_t tls_error_handler;
static pthread_key_t tls_kerrno;

__export_function kerror_t* _kerrno(void)
{
	kerror_t* r = pthread_getspecific(tls_kerrno);

	if (!r) {
		r = calloc(1, sizeof(*r));
		/* what to do if r == 0 here? */
		pthread_setspecific(tls_kerrno, r);
	}
	return r;
}

__export_function void set_kerrno_with_trace(kerror_t errnum)
{
	if (!errnum)
		return;
	if (errnum < 0)
		errnum = -errnum;

	kerrno = errnum;
#ifdef __LINUX__
	void* return_addresses[2048];
	int naddr = backtrace(return_addresses,
		sizeof(return_addresses)/sizeof(void*));

	backtrace_symbols_fd(return_addresses, naddr, fileno(stderr));
	fflush(stderr);
#endif
}

__export_function extern const char* kstrerror(kerror_t errnum)
{
	const char* errstr = 0;
	if (errnum < 0)
		errnum = -errnum;

	if (errnum > K_LASTERRNO) {
		errstr = "Unknown libk Error";
	} else if (errnum && (errnum < K_FIRSTERRNO)) {
		errstr = strerror(errno);
	} else {
		errstr = _err_messages[ERR_IDX(errnum)];
	}
	return errstr;
}

__export_function void kperror(const char* str)
{
	if (str && *str) {
		fprintf(stderr, "%s: ", str);
	}
	fprintf(stderr, "%s\n", kstrerror(kerrno));
	fflush(stderr);
}

__attribute__((constructor))
static void _init_error_handling(void)
{
	/* maybe initialize syslog logging here */
	pthread_key_create(&tls_error_handler, 0);
	pthread_key_create(&tls_kerrno, 0);
	/* initialize main-thread kerrno */
	kerrno = K_ESUCCESS;
}

__attribute__((destructor))
static void _fini_error_handling(void)
{
	pthread_key_delete(tls_error_handler);
	pthread_key_delete(tls_kerrno);
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
	kerrno = err;
	handle_err(err, lvl, _err_messages[ERR_IDX(err)]);
}

__export_function void k_set_error_handler
(k_err_fn error_handler)
{
	if (!error_handler)
		pthread_setspecific(tls_error_handler, &_default_err_handler);
	else
		pthread_setspecific(tls_error_handler, error_handler);
}

