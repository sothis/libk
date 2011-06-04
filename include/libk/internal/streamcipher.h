/*
 * libk - streamcipher.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _KSTREAMCIPHER_H
#define _KSTREAMCIPHER_H

#include <stdint.h>
#include <stddef.h>

typedef struct k_sc_t k_sc_t;

extern k_sc_t* k_sc_init
(enum streamcipher_e cipher, uint32_t noncebits);

extern k_sc_t* k_sc_init_with_blockcipher
(enum blockcipher_e cipher, enum bcmode_e mode, size_t max_workers);

extern void k_sc_finish
(k_sc_t* c);

extern int32_t k_sc_set_key
(k_sc_t* c, const void* nonce, const void* key, uint32_t keybits);

extern size_t k_sc_get_nonce_bytes
(k_sc_t* c);

extern void k_sc_update
(k_sc_t* c, const void* input, void* output, size_t bytes);

extern void k_sc_keystream
(k_sc_t* c, void* output, size_t bytes);


#endif /* _KSTREAMCIPHER_H */
