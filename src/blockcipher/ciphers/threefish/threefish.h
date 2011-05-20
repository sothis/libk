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
