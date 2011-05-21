/*
 * prng.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _KPRNG_H
#define _KPRNG_H

#include <stdint.h>
#include <stddef.h>

typedef struct k_prng_t k_prng_t;

extern k_prng_t* k_prng_init
(enum prng_e prng);

extern void k_prng_finish
(k_prng_t* c);

extern void k_prng_set_seed
(k_prng_t* c, const void* seed, size_t seed_bytes);

extern void k_prng_update
(k_prng_t* c, void* output, size_t bytes);

extern uint8_t k_prng_get_uint8
(k_prng_t* c);

extern uint16_t k_prng_get_uint16
(k_prng_t* c);

extern uint32_t k_prng_get_uint32
(k_prng_t* c);

extern uint64_t k_prng_get_uint64
(k_prng_t* c);


#endif /* _KPRNG_H */
