/*
 * algorithms.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

/* ID's must be unique and assigned ID's must not be reassigned anymore */

enum keytype_e {
	BLK_CIPHER_KEY_ENCRYPT	= 0,
	BLK_CIPHER_KEY_DECRYPT	= 1,
};

enum blockcipher_e {
	BLK_CIPHER_NOOP				= 0,
	BLK_CIPHER_THREEFISH_256		= 1,
	BLK_CIPHER_THREEFISH_512		= 2,
	BLK_CIPHER_THREEFISH_1024		= 3,
	BLK_CIPHER_AES				= 4,
	/* insert new block ciphers above this line and adjust
	 * BLK_CIPHER_MAX_SUPPORT below accordingly */
	BLK_CIPHER_MAX_SUPPORT			= BLK_CIPHER_AES
};

enum streamcipher_e {
	STREAM_CIPHER_NOOP			= 0,
	STREAM_CIPHER_ARC4			= 1,
	/* insert new stream ciphers above this line and adjust
	 * STREAM_CIPHER_MAX_SUPPORT below accordingly */
	STREAM_CIPHER_MAX_SUPPORT		= STREAM_CIPHER_ARC4
};

enum prng_e {
	PRNG_NOOP				= 0,
	PRNG_MT19937_32				= 1,
	/* insert new prngs above this line and adjust
	 * PRNG_MAX_SUPPORT below accordingly */
	PRNG_MAX_SUPPORT			= PRNG_MT19937_32
};
