#ifndef _BLOCKCIPHER_DESC_H
#define _BLOCKCIPHER_DESC_H

#include <libk/libk.h>
#include <stddef.h>
#include <stdint.h>
#include "utils/sections.h"

typedef void (*transform_fn)(
	const void*	schedule,
	const void*	input,
	void*		output
);

typedef void (*key_schedule_fn)(
	void*		schedule,
	const void*	key,
	const uint32_t	keybits
);

struct blockcipher_desc {
	/* the human readable cipher name */
	const char*			name;
	/* the authors of the code module */
	const char* const*		authors;

	const key_schedule_fn		set_encrypt_key;
	const key_schedule_fn		set_decrypt_key;
	const key_schedule_fn		set_tweak;

	const transform_fn		encrypt;
	const transform_fn		decrypt;

	/* block size in bytes, if similar ciphers differ in their block size,
	 * add a new blockcipher module for each one */
	const size_t			block_size;
	/* the size in bytes of the used key schedule structure */
	const size_t			schedule_size;
	/* supported key sizes in bits */
	const struct range32_t*		key_range;
	/* set this in order to produce warnings when a specific
	 * key size is being used */
	const struct range32_t* 	insecure_key_range;
	/* supported tweak sizes in bits */
	const struct range32_t* 	tweak_range;
	/* set this in order to produce warnings when a specific
	 * tweak size is being used */
	const struct range32_t*		insecure_tweak_range;
	/* set this flag in order to produce warnings when the cipher is
	 * being used */
	const uint32_t			insecure;
	/* the cipher id, as defined in include/libk/internal/algorithms.h */
	const uint32_t			id;
} __attribute__((_section_alignment));


#define _blockcipher_entry_					\
	__attribute__((section(_DATA_SEGMENT "__blockciphers"),	\
	used, _section_alignment, externally_visible)) const

#define blockcipher_start(_id, _name)			\
	_blockcipher_entry_ struct blockcipher_desc 	\
	__blockcipher_##_id = {				\
		.id		= BLK_CIPHER_##_id,	\
		.name		= _name,

#define blockcipher_end					\
	};

#endif /* _BLOCKCIPHER_DESC_H */
