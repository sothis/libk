/*
 * threefish.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _THREEFISH_H
#define _THREEFISH_H

#include "../blockcipher_desc.h"
#include "utils/unittest_desc.h"
#include "utils/endian-neutral.h"
#include <stdint.h>
#include <string.h>

enum tf_constants_e {
	tf_parity		= 0x1bd11bdaa9fc1a22ull,
};

#define m(w1,w2,r,k0,k1)(\
	o##w2+=k1,o##w1+=o##w2+k0,o##w2=((o##w2<<r)|\
	(o##w2>>(64-r)))^o##w1)

#define u(w1,w2,r,k0,k1)(\
	o##w2=(((o##w2^o##w1)>>r)|((o##w2^o##w1)<<(64-r))),\
	o##w1-=o##w2+k0,o##w2-=k1)

#endif /* _THREEFISH_H */
