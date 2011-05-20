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


#endif /* _KHASH_H */
