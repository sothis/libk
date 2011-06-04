/*
 * libk - bcmode_desc.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _BCMODE_DESC_H
#define _BCMODE_DESC_H

#include "blockcipher/ciphers/blockcipher_desc.h"

typedef void (*mode_transform_fn)(
	k_bc_t*			mode,
	const void*		input,
	void*			output,
	size_t			worker_num
);

/* parameter block for parallel worker functions */
typedef struct mode_param_t {
	k_bc_t*			m;
	const void*		in;
	void*			out;
	size_t			nblocks;
	size_t			worker_num;
} mode_param_t;

typedef void (*prepare_iv_fn)(
	k_bc_t* 		m,
	struct mode_param_t	pa[],
	size_t			worker
);

struct bcmode_desc {
	/* the human readable cipher mode name */
	const char*			name;
	/* the authors of the code module */
	const char* const*		authors;

	const prepare_iv_fn		prepare_iv;

	const mode_transform_fn		encrypt;
	const mode_transform_fn		decrypt;

	/* the cipher mode id, as defined in include/libk/internal/modes.h */
	const uint32_t			id;
	/* set this flag in order to produce warnings when the mode is
	 * being used */
	const uint32_t			insecure;
	/* set this flag if the mode decryption requires the encryption key
	 * to be set */
	const uint32_t			decrypt_uses_encrypt_transform;
	/* set this flag if the mode generates a keystream, which can be
	 * precomputed, i.e. xoring against plaintext can be omitted because
	 * the mode does not depend on the result of the mode for the next
	 * block. this allows the mode to be used as a streamcipher or a
	 * csprng. */
	const uint32_t			keystream_precomputable;

	const uint32_t			encrypt_parallelizable;
	const uint32_t			decrypt_parallelizable;
} __attribute__((_section_alignment));


/* internal context structure for public apis */
struct k_bc_t {
	const struct bcmode_desc*	mode;
	const struct blockcipher_desc*	blockcipher;

	uint8_t**			worker_ivs;
	uint8_t*			iv_backup;
	void*				schedule;
	size_t				alloced_schedsize;

	void*				workbench;
	size_t				worker_threads;

	size_t				keysize;
	size_t				tweaksize;
	enum keytype_e			keytype;
	uint32_t			insecure_encryption;
} __attribute__((_section_alignment));

#define _blockciphermode_entry_					\
	__attribute__((section(_DATA_SEGMENT "__bcmodes"),	\
	used, _section_alignment, externally_visible)) const

#define blockciphermode_start(_id, _name)			\
	_blockciphermode_entry_ struct bcmode_desc 		\
	__blockciphermode_##_id = {				\
		.id		= BLK_CIPHER_MODE_##_id,	\
		.name		= _name,

#define blockciphermode_end					\
	};

#endif /* _BCMODE_DESC_H */
