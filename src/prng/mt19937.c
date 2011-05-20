/*
  Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. The names of its contributors may not be used to endorse or promote
  products derived from this software without specific prior written
  permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "prng_desc.h"
#include "utils/endian-neutral.h"
#include "utils/unittest_desc.h"

#define N		624
#define M		397
#define MATRIX_A	0x9908b0dfUL
#define UPPER_MASK	0x80000000UL
#define LOWER_MASK	0x7fffffffUL

static const uint32_t mag01[2] = {
	0x00000000, MATRIX_A
};

struct mt19937_t {
	uint32_t	mt[N];
	int32_t		mti;
	int32_t		initialized;
};

static void init_genrand(struct mt19937_t* c, uint32_t s)
{
	c->mt[0]= s;
	for (c->mti=1; c->mti<N; c->mti++) {
		c->mt[c->mti] = (1812433253 * (c->mt[c->mti-1] ^
			(c->mt[c->mti-1] >> 30)) + c->mti);
	}
	c->initialized = 1;
}

static void mt19937_init(void* state, const void* seed, size_t seed_bytes)
{
	struct mt19937_t* c = state;
	int32_t i, j, k;
	size_t key_length = seed_bytes/sizeof(uint32_t);
	const uint32_t* init_key = seed;
	init_genrand(c, 19650218);
	i=1; j=0;
	k = (N>key_length ? N : key_length);
	for (; k; k--) {
		c->mt[i] = (c->mt[i] ^ ((c->mt[i-1] ^ (c->mt[i-1] >> 30)) *
			1664525)) + _get_uint32_l(init_key+(j*sizeof(uint32_t))) + j;
		i++;
		j++;
		if (i>=N) {
			c->mt[0] = c->mt[N-1];
			i=1;
		}
		if (j>=key_length)
			j=0;
	}
	for (k=N-1; k; k--) {
		c->mt[i] = (c->mt[i] ^ ((c->mt[i-1] ^ (c->mt[i-1] >> 30)) *
			1566083941)) - i;
		i++;
		if (i>=N) {
			c->mt[0] = c->mt[N-1];
			i=1;
		}
	}
	c->mt[0] = 0x80000000;
}

static void mt19937_update(void* state, void* output)
{
	struct mt19937_t* c = state;
	uint32_t y;

	if (c->mti >= N) {
		int kk;

		if (!c->initialized)
			init_genrand(c, 5489);

		for (kk=0;kk<N-M;kk++) {
			y = (c->mt[kk]&UPPER_MASK)|(c->mt[kk+1]&LOWER_MASK);
			c->mt[kk] = c->mt[kk+M] ^ (y >> 1) ^ mag01[y&0x01];
		}
		for (;kk<N-1;kk++) {
			y = (c->mt[kk]&UPPER_MASK)|(c->mt[kk+1]&LOWER_MASK);
			c->mt[kk] = c->mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y&0x01];
		}
		y = (c->mt[N-1]&UPPER_MASK)|(c->mt[0]&LOWER_MASK);
		c->mt[N-1] = c->mt[M-1] ^ (y >> 1) ^ mag01[y&0x01];

		c->mti = 0;
	}

	y = c->mt[c->mti++];
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680;
	y ^= (y << 15) & 0xefc60000;
	y ^= (y >> 18);
	_put_uint32_l(output, y);
}


static const char* const authors[] = {
	"Makoto Matsumoto <m-mat@math.sci.hiroshima-u.ac.jp>",
	"Takuji Nishimura <m-mat@math.sci.hiroshima-u.ac.jp>",
	"Janos Laube <janos.dev@gmail.com>",
	0
};

prng_start(MT19937_32, "Mersenne Twister 19937-32")
	.authors		= authors,
	.word_size		= 32,
	.context_size		= sizeof(struct mt19937_t),
	.init			= &mt19937_init,
	.update			= &mt19937_update,
prng_end
