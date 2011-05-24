/*
 * libk - streamcipher_desc.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _STREAMCIPHER_DESC_H
#define _STREAMCIPHER_DESC_H

#include <libk/libk.h>
#include <stddef.h>
#include <stdint.h>
#include "utils/sections.h"

typedef void (*sc_init_fn)(
	void*		state,
	const void*	key,
	uint32_t	keybits
);

typedef void (*sc_update_fn)(
	void*		state,
	const void*	input,
	void*		output,
	size_t		length
);


struct streamcipher_desc {
	/* the human readable cipher name */
	const char*			name;
	/* the authors of the code module */
	const char* const*		authors;

	const sc_init_fn		init;
	const sc_update_fn		update;

	/* supported key sizes in bits */
	const struct range32_t*		key_range;
	/* set this in order to produce warnings when a specific
	 * key size is being used */
	const struct range32_t* 	insecure_key_range;
	/* state size in bits, if similar streamciphers differ in their
	 * state size, add a new streamcipher module for each one */
	const uint32_t			state_size;
	/* the size in bytes of the used context structure */
	const size_t			context_size;
	/* set this flag in order to produce warnings when the cipher is
	 * being used */
	const uint32_t			insecure;
	/* the cipher id, as defined in include/libk/internal/algorithms.h */
	const uint32_t			id;
} __attribute__((_section_alignment));


/* internal context structure for public apis */
struct k_sc_t {
	const struct streamcipher_desc*		cipher;
	k_bc_t*					blockcipher;
	void*					ctx;
	size_t					alloced_ctxsize;
	void*					partial_block;
	void*					old_iv;
	int					have_partial_block;
} __attribute__((_section_alignment));


#define _streamcipher_entry_					\
	__attribute__((section(_DATA_SEGMENT "__streamciphers"),\
	used, _section_alignment, externally_visible)) const

#define streamcipher_start(_id, _name)			\
	_streamcipher_entry_ struct streamcipher_desc 	\
	__streamcipher_##_id = {			\
		.id		= STREAM_CIPHER_##_id,	\
		.name		= _name,

#define streamcipher_end				\
	};

#endif /* _STREAMCIPHER_DESC_H */
