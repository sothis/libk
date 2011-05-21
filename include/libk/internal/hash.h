/*
 * libk - hash.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _KHASH_H
#define _KHASH_H

#include <stdint.h>
#include <stddef.h>

typedef struct k_hash_t k_hash_t;

extern k_hash_t* k_hash_init
(enum hashsum_e hashsum, uint32_t output_bits);

extern void k_hash_finish
(k_hash_t* c);

extern void k_hash_reset
(k_hash_t* c);

extern void k_hash_update
(k_hash_t* c, const void* input, size_t bytes);

extern void k_hash_final
(k_hash_t* c, void* output);

extern uint32_t k_hash_digest_size
(k_hash_t* c);


#endif /* _KHASH_H */
