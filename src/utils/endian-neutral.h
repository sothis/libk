/*
 * libk - endian-neutral.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _ENDIAN_NEUTRAL_H
#define _ENDIAN_NEUTRAL_H

#include <stdint.h>

#ifdef __BIG_ENDIAN__

#define _get_uint32_b(i) (*((const uint32_t*)((const uint8_t*)i)))

#define _put_uint32_b(o, i) {*((uint32_t*)((uint8_t*)o)) = i;}

#define _get_uint16_l(i) (			\
	((uint16_t)((const uint8_t*)i)[1]<<8)^	\
	((uint16_t)((const uint8_t*)i)[0]) )

#define _put_uint16_l(o, i) {			\
	((uint8_t*)o)[1] = (uint16_t)((i)>>8);	\
	((uint8_t*)o)[0] = (uint16_t)(i); }

#define _get_uint32_l(i) (			\
	((uint32_t)((const uint8_t*)i)[3]<<24)^	\
	((uint32_t)((const uint8_t*)i)[2]<<16)^	\
	((uint32_t)((const uint8_t*)i)[1]<<8)^	\
	((uint32_t)((const uint8_t*)i)[0]) )

#define _put_uint32_l(o, i) {			\
	((uint8_t*)o)[3] = (uint32_t)((i)>>24);	\
	((uint8_t*)o)[2] = (uint32_t)((i)>>16);	\
	((uint8_t*)o)[1] = (uint32_t)((i)>>8);	\
	((uint8_t*)o)[0] = (uint32_t)(i); }

#define _get_uint64_l(i) (			\
	((uint64_t)((const uint8_t*)i)[7]<<56)^	\
	((uint64_t)((const uint8_t*)i)[6]<<48)^	\
	((uint64_t)((const uint8_t*)i)[5]<<40)^	\
	((uint64_t)((const uint8_t*)i)[4]<<32)^	\
	((uint64_t)((const uint8_t*)i)[3]<<24)^	\
	((uint64_t)((const uint8_t*)i)[2]<<16)^	\
	((uint64_t)((const uint8_t*)i)[1]<<8)^	\
	((uint64_t)((const uint8_t*)i)[0]) )

#define _put_uint64_l(o, i) {			\
	((uint8_t*)o)[7] = (uint64_t)((i)>>56);	\
	((uint8_t*)o)[6] = (uint64_t)((i)>>48);	\
	((uint8_t*)o)[5] = (uint64_t)((i)>>40);	\
	((uint8_t*)o)[4] = (uint64_t)((i)>>32);	\
	((uint8_t*)o)[3] = (uint64_t)((i)>>24);	\
	((uint8_t*)o)[2] = (uint64_t)((i)>>16);	\
	((uint8_t*)o)[1] = (uint64_t)((i)>>8);	\
	((uint8_t*)o)[0] = (uint64_t)(i); }

#else /* __BIG_ENDIAN__ */

#define _get_uint32_b(i) (			\
	((uint32_t)((const uint8_t*)i)[0]<<24)^	\
	((uint32_t)((const uint8_t*)i)[1]<<16)^	\
	((uint32_t)((const uint8_t*)i)[2]<<8)^	\
	((uint32_t)((const uint8_t*)i)[3]) )

#define _put_uint32_b(o, i) {			\
	((uint8_t*)o)[0] = (uint32_t)((i)>>24);	\
	((uint8_t*)o)[1] = (uint32_t)((i)>>16);	\
	((uint8_t*)o)[2] = (uint32_t)((i)>>8);	\
	((uint8_t*)o)[3] = (uint32_t)(i); }

#define _get_uint16_l(i) (*((const uint16_t*)((const uint8_t*)i)))

#define _put_uint16_l(o, i) {*((uint16_t*)((uint8_t*)o)) = i;}

#define _get_uint32_l(i) (*((const uint32_t*)((const uint8_t*)i)))

#define _put_uint32_l(o, i) {*((uint32_t*)((uint8_t*)o)) = i;}

#define _get_uint64_l(i) (*((const uint64_t*)((const uint8_t*)i)))

#define _put_uint64_l(o, i) {*((uint64_t*)((uint8_t*)o)) = i;}

#endif /* __BIG_ENDIAN__ */

static inline void copy_uint64_t(uint64_t* to, const void* from, size_t n)
{
	for (size_t i = 0; i < n; ++i) {
		to[i] = _get_uint64_l(from + i*sizeof(uint64_t));
	}
}

static inline void feed_forward64(const void* input, void* output, size_t n)
{
	for (size_t i = 0; i < n; ++i) {
		_put_uint64_l(output + (i * sizeof(uint64_t)),
			_get_uint64_l(output + (i * sizeof(uint64_t))) ^
			_get_uint64_l(input + (i * sizeof(uint64_t))));
	}
}

#endif /* _ENDIAN_NEUTRAL_H */
