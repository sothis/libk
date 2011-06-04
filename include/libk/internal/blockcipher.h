/*
 * libk - blockcipher.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _KBLOCKCIPHER_H
#define _KBLOCKCIPHER_H

#include <stdint.h>
#include <stddef.h>


typedef struct k_bc_t k_bc_t;

extern k_bc_t* k_bc_init
(enum blockcipher_e cipher);

extern void k_bc_finish
(k_bc_t* c);


/* low level api with no error checking */

extern void k_bc_set_encrypt_key
(k_bc_t* c, const void* k, uint32_t bits);

extern void k_bc_set_decrypt_key
(k_bc_t* c, const void* k, uint32_t bits);

extern void k_bc_set_tweak
(k_bc_t* c, const void* t, uint32_t bits);

extern void k_bc_encrypt
(k_bc_t* c, const void* input, void* output);

extern void k_bc_decrypt
(k_bc_t* c, const void* input, void* output);

extern size_t k_bc_get_blocksize
(k_bc_t* c);


/* high level api using a specific blockcipher mode */

extern int32_t k_bcmode_set_mode
(k_bc_t* c, enum bcmode_e mode, int32_t max_workers);

extern int32_t k_bcmode_produces_keystream
(enum bcmode_e mode);

extern int32_t k_bcmode_set_key
(k_bc_t* c, const void* k, uint32_t bits, enum keytype_e t);

extern int32_t k_bcmode_set_tweak
(k_bc_t* c, const void* t, uint32_t bits);

extern void k_bcmode_set_iv
(k_bc_t* c, const void* iv);

extern const void* k_bcmode_get_iv
(k_bc_t* c);

extern void k_bcmode_backup_iv
(k_bc_t* c);

extern void k_bcmode_restore_iv
(k_bc_t* c);

extern void k_bcmode_update
(k_bc_t* c, const void* input, void* output, size_t blocks);


#endif /* _KBLOCKCIPHER_H */
