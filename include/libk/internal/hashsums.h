/*
 * hashsums.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

/* ID's must be unique and assigned ID's must not be reassigned anymore */

enum hashsum_e {
	HASHSUM_NOOP			= 0,
	HASHSUM_SKEIN_256		= 1,
	HASHSUM_SKEIN_512		= 2,
	HASHSUM_SKEIN_1024		= 3,
	HASHSUM_SHA1			= 4,
	HASHSUM_SHA1SSSE3		= 5,
	/* insert new hashsum above this line and adjust
	 * HASHSUM_MAX_SUPPORT below accordingly */
	HASHSUM_MAX_SUPPORT		= HASHSUM_SHA1
};
