/*
 * libk - ktypes.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _KTYPES_H
#define _KTYPES_H

#include <stddef.h>
#include <stdint.h>

typedef int	kerror_t;


struct range8_t {
	uint8_t	from;
	uint8_t	to;
};

struct range16_t {
	uint16_t	from;
	uint16_t	to;
};

struct range32_t {
	uint32_t	from;
	uint32_t	to;
};

struct range64_t {
	uint64_t	from;
	uint64_t	to;
};

static inline uint32_t is_in_range8
(const struct range8_t* r, uint8_t val)
{
	return (val >= r->from) && (val <= r->to);
}

static inline uint32_t is_in_range16
(const struct range16_t* r, uint16_t val)
{
	return (val >= r->from) && (val <= r->to);
}

static inline uint32_t is_in_range32
(const struct range32_t* r, uint32_t val)
{
	return (val >= r->from) && (val <= r->to);
}

static inline uint32_t is_in_range64
(const struct range64_t* r, uint64_t val)
{
	return (val >= r->from) && (val <= r->to);
}

#endif /* _KTYPES_H */
