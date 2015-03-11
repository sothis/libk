/*
 * libk - simple.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <libk/libk.h>
#include <string.h>
#include <stdlib.h>
#include "utils/sections.h"
#include "utils/mem.h"

__export_function void* _k_key_derive_simple1024
(	const char*	pass,
	void*		salt,
	size_t		salt_bytes,
	uint64_t	iter)
{
	k_hash_t* h = k_hash_init(HASHSUM_SKEIN_1024, 1024);
	if (!h)
		return 0;
	size_t sp = strlen(pass);
	size_t digestbytes = k_hash_digest_bytes(h);

	void* inp = k_malloc(sp+digestbytes);
	if (!inp) {
		k_hash_finish(h);
		return 0;
	}
	void* outp = k_malloc(digestbytes);
	if (!outp) {
		k_free(inp);
		k_hash_finish(h);
		return 0;
	}

	memcpy(inp, pass, sp);
	memcpy(inp+sp, salt, salt_bytes);

	k_hash_update(h, inp, sp+digestbytes);
	k_hash_final(h, outp);
	k_free(inp);

	for (uint64_t i = 0; i < iter; ++i) {
		k_hash_reset(h);
		k_hash_update(h, outp, digestbytes);
		k_hash_final(h, outp);
	}
	k_hash_finish(h);

	return outp;
}

__export_function void* _k_key_derive_skein_1024
(	const char*	pass,
	void*		salt,
	size_t		salt_bytes,
	size_t		key_bytes,
	uint64_t	iter)
{
	size_t passlen;
	size_t digestbytes;
	unsigned char* keyout;

	if (!pass || !salt || !salt_bytes || !key_bytes)
		return 0;

	passlen = strlen(pass);
	if (!passlen)
		return 0;

	k_hash_t* h = k_hash_init(HASHSUM_SKEIN_1024, 1024);
	if (!h)
		return 0;

	digestbytes = k_hash_digest_bytes(h);


	if (!key_bytes || (key_bytes > digestbytes)) {
		k_hash_finish(h);
		return 0;
	}

	void* inp = k_calloc(1, passlen+salt_bytes+digestbytes);
	if (!inp) {
		k_hash_finish(h);
		return 0;
	}
	void* outp = k_malloc(digestbytes);
	if (!outp) {
		k_free(inp);
		k_hash_finish(h);
		return 0;
	}

	memcpy(inp, pass, passlen);
	memcpy(inp+passlen, salt, salt_bytes);

	k_hash_update(h, inp, passlen+salt_bytes);
	k_hash_final(h, outp);
	k_free(inp);

	for (uint64_t i = 0; i < iter; ++i) {
		k_hash_reset(h);
		k_hash_update(h, outp, digestbytes);
		k_hash_final(h, outp);
	}
	k_hash_finish(h);

	keyout = malloc(key_bytes);
	if (!keyout) {
		k_free(outp);
		return 0;
	}

	memcpy(keyout, outp, key_bytes);
	k_free(outp);

	return keyout;
}
