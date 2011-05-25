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

__export_function void* _k_key_derive_simple1024
(	const char*	pass,
	void*		salt,
	uint64_t	iter)
{
	k_hash_t* h = k_hash_init(HASHSUM_SKEIN_1024, 1024);
	if (!h)
		return 0;
	size_t sp = strlen(pass);
	size_t digestbytes = (k_hash_digest_size(h) + 7) / 8;

	void* inp = malloc(sp+digestbytes);
	if (!inp) {
		k_hash_finish(h);
		return 0;
	}
	void* outp = malloc(digestbytes);
	if (!outp) {
		free(inp);
		k_hash_finish(h);
		return 0;
	}

	memcpy(inp, pass, sp);
	memcpy(inp+sp, salt, digestbytes);

	k_hash_update(h, inp, sp+digestbytes);
	k_hash_final(h, outp);
	free(inp);

	for (uint64_t i = 0; i < iter; ++i) {
		k_hash_reset(h);
		k_hash_update(h, outp, digestbytes);
		k_hash_final(h, outp);
	}
	k_hash_finish(h);

	return outp;
}
