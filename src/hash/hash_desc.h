/*
 * libk - hash_desc.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _HASH_DESC_H
#define _HASH_DESC_H

#include <libk/libk.h>
#include <stddef.h>
#include <stdint.h>
#include "utils/sections.h"

typedef void (*hash_init_fn)(
	void*		context,
	uint32_t	output_bits
);

typedef void (*hash_update_fn)(
	void*		context,
	const void*	input,
	size_t		bytes,
	uint32_t	output_bits
);

typedef void (*hash_final_fn)(
	void*		context,
	void*		digest,
	uint32_t	output_bits
);

typedef void (*hash_finish_fn)(
	void*		context
);

struct hash_desc {
	/* the human readable hashsum name */
	const char*			name;
	/* the authors of the code module */
	const char* const*		authors;

	const hash_init_fn		init;
	const hash_update_fn		update;
	const hash_final_fn		final;
	/* if the hashsum itself allocates memory (if using a blockcipher
	 * inside libk via public apis for example) set this with a function
	 * releasing the memory. it will be called when invoking
	 * k_hash_finish(). it is optional */
	const hash_finish_fn		finish;

	/* state size in bits, if similar hashsums differ in their state size,
	 * add a new hashsum module for each one */
	const uint32_t			state_size;
	/* the size in bytes of the used context structure */
	const size_t			context_size;
	/* set this flag in order to produce warnings when the hashsum is
	 * being used */
	const uint32_t			insecure;

	/* the hash id, as defined in include/libk/internal/hashsums.h */
	const uint32_t			id;
} __attribute__((_section_alignment));

/* internal context structure for public apis */
struct k_hash_t {
	const struct hash_desc*		hash;
	void*				context;
	size_t				alloced_ctxsize;
	uint32_t			output_bits;
} __attribute__((_section_alignment));


#define _hash_entry_						\
	__attribute__((section(_DATA_SEGMENT "__hashsums"),	\
	used, _section_alignment, externally_visible)) const

#define hashsum_start(_id, _name)				\
	_hash_entry_ struct hash_desc		 		\
	__hashsum_##_id = {					\
		.id		= HASHSUM_##_id,		\
		.name		= _name,

#define hashsum_end						\
	};

#endif /* _HASH_DESC_H */
