/*
 * prng_desc.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _PRNG_DESC_H
#define _PRNG_DESC_H

#include <libk/libk.h>
#include <stddef.h>
#include <stdint.h>
#include "utils/sections.h"

typedef void (*prng_init_fn)(
	void*		state,
	const void*	seed,
	size_t		seed_bytes
);

typedef void (*prng_update_fn)(
	void*		state,
	void*		output
);


struct prng_desc {
	/* the human readable prng name */
	const char*			name;
	/* the authors of the code module */
	const char* const*		authors;

	const prng_init_fn		init;
	const prng_update_fn		update;

	/* word size in bits, if similar prng's differ in their
	 * word size, add a new prng module for each one */
	const uint32_t			word_size;
	/* the size in bytes of the used context structure */
	const size_t			context_size;
	/* the prng id, as defined in include/libk/internal/algorithms.h */
	const uint32_t			id;
} __attribute__((_section_alignment));


/* internal context structure for public apis */
struct k_prng_t {
	const struct prng_desc*			prng;
	void*					ctx;
	size_t					alloced_ctxsize;
} __attribute__((_section_alignment));


#define _prng_entry_					\
	__attribute__((section(_DATA_SEGMENT "__prngs"),\
	used, _section_alignment, externally_visible)) const

#define prng_start(_id, _name)			\
	_prng_entry_ struct prng_desc 		\
	__prng_##_id = {			\
		.id		= PRNG_##_id,	\
		.name		= _name,

#define prng_end				\
	};

#endif /* _PRNG_DESC_H */
