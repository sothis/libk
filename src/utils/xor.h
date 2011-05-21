/*
 * xor.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _XOR_H
#define _XOR_H

#include <stddef.h>
#include <stdint.h>

static inline void xorb_8(void* o, const void* i0, const void* i1, size_t l)
{
	while(l--) {
		((uint8_t*)o)[l] = ((uint8_t*)i0)[l]^((uint8_t*)i1)[l];
	}
}

static inline void xorb_16(void* o, const void* i0, const void* i1, size_t l)
{
	size_t r = l % sizeof(uint16_t);
	l = l / sizeof(uint16_t);
	while(l--) {
		((uint16_t*)o)[l] = ((uint16_t*)i0)[l]^((uint16_t*)i1)[l];
	}
	xorb_8(o, i0, i1, r);
}

static inline void xorb_32(void* o, const void* i0, const void* i1, size_t l)
{
	size_t r = l % sizeof(uint32_t);
	l = l / sizeof(uint32_t);
	while(l--) {
		((uint32_t*)o)[l] = ((uint32_t*)i0)[l]^((uint32_t*)i1)[l];
	}
	xorb_8(o, i0, i1, r);
}

static inline void xorb_64(void* o, const void* i0, const void* i1, size_t l)
{
	size_t r = l % sizeof(uint64_t);
	l = l / sizeof(uint64_t);

	while(l--) {
		((uint64_t*)o)[l] = ((uint64_t*)i0)[l]^((uint64_t*)i1)[l];
	}
	xorb_8(o, i0, i1, r);
}

#endif /* _XOR_H */
