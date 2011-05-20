#include <libk/libk.h>
#include "utils/unittest_desc.h"


_unittest_data_ uint8_t _plain[] = {
	0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
	0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
	0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
	0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
	0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
	0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
	0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
	0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};

_unittest_data_ uint8_t _iv[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

_unittest_data_ uint8_t _key_192[] = {
	0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52,
	0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5,
	0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b
};

_unittest_data_ uint8_t _cipher_ecb[] = {
	0xbd, 0x33, 0x4f, 0x1d, 0x6e, 0x45, 0xf2, 0x5f,
	0xf7, 0x12, 0xa2, 0x14, 0x57, 0x1f, 0xa5, 0xcc,
	0x97, 0x41, 0x04, 0x84, 0x6d, 0x0a, 0xd3, 0xad,
	0x77, 0x34, 0xec, 0xb3, 0xec, 0xee, 0x4e, 0xef,
	0xef, 0x7a, 0xfd, 0x22, 0x70, 0xe2, 0xe6, 0x0a,
	0xdc, 0xe0, 0xba, 0x2f, 0xac, 0xe6, 0x44, 0x4e,
	0x9a, 0x4b, 0x41, 0xba, 0x73, 0x8d, 0x6c, 0x72,
	0xfb, 0x16, 0x69, 0x16, 0x03, 0xc1, 0x8e, 0x0e
};

_unittest_data_ uint8_t _cipher_cbc[] = {
	0x4f, 0x02, 0x1d, 0xb2, 0x43, 0xbc, 0x63, 0x3d,
	0x71, 0x78, 0x18, 0x3a, 0x9f, 0xa0, 0x71, 0xe8,
	0xb4, 0xd9, 0xad, 0xa9, 0xad, 0x7d, 0xed, 0xf4,
	0xe5, 0xe7, 0x38, 0x76, 0x3f, 0x69, 0x14, 0x5a,
	0x57, 0x1b, 0x24, 0x20, 0x12, 0xfb, 0x7a, 0xe0,
	0x7f, 0xa9, 0xba, 0xac, 0x3d, 0xf1, 0x02, 0xe0,
	0x08, 0xb0, 0xe2, 0x79, 0x88, 0x59, 0x88, 0x81,
	0xd9, 0x20, 0xa9, 0xe6, 0x4f, 0x56, 0x15, 0xcd
};

_unittest_data_ uint8_t _cipher_cfb[] = {
	0xcd, 0xc8, 0x0d, 0x6f, 0xdd, 0xf1, 0x8c, 0xab,
	0x34, 0xc2, 0x59, 0x09, 0xc9, 0x9a, 0x41, 0x74,
	0x67, 0xce, 0x7f, 0x7f, 0x81, 0x17, 0x36, 0x21,
	0x96, 0x1a, 0x2b, 0x70, 0x17, 0x1d, 0x3d, 0x7a,
	0x2e, 0x1e, 0x8a, 0x1d, 0xd5, 0x9b, 0x88, 0xb1,
	0xc8, 0xe6, 0x0f, 0xed, 0x1e, 0xfa, 0xc4, 0xc9,
	0xc0, 0x5f, 0x9f, 0x9c, 0xa9, 0x83, 0x4f, 0xa0,
	0x42, 0xae, 0x8f, 0xba, 0x58, 0x4b, 0x09, 0xff
};

_unittest_data_ uint8_t _cipher_ofb[] = {
	0xcd, 0xc8, 0x0d, 0x6f, 0xdd, 0xf1, 0x8c, 0xab,
	0x34, 0xc2, 0x59, 0x09, 0xc9, 0x9a, 0x41, 0x74,
	0xfc, 0xc2, 0x8b, 0x8d, 0x4c, 0x63, 0x83, 0x7c,
	0x09, 0xe8, 0x17, 0x00, 0xc1, 0x10, 0x04, 0x01,
	0x8d, 0x9a, 0x9a, 0xea, 0xc0, 0xf6, 0x59, 0x6f,
	0x55, 0x9c, 0x6d, 0x4d, 0xaf, 0x59, 0xa5, 0xf2,
	0x6d, 0x9f, 0x20, 0x08, 0x57, 0xca, 0x6c, 0x3e,
	0x9c, 0xac, 0x52, 0x4b, 0xd9, 0xac, 0xc9, 0x2a
};

_unittest_data_ uint8_t _cipher_ctr[] = {
	0xcd, 0xc8, 0x0d, 0x6f, 0xdd, 0xf1, 0x8c, 0xab,
	0x34, 0xc2, 0x59, 0x09, 0xc9, 0x9a, 0x41, 0x74,
	0xa9, 0x0b, 0x2b, 0x2b, 0x80, 0xde, 0xda, 0x53,
	0x2d, 0x54, 0x27, 0x53, 0x7f, 0x8c, 0xbd, 0x88,
	0xea, 0xff, 0x8f, 0xd0, 0x59, 0xa7, 0x99, 0x5d,
	0x02, 0x33, 0xe0, 0x64, 0xf0, 0x4f, 0x16, 0x31,
	0xd4, 0xd2, 0x23, 0xfa, 0x51, 0x80, 0x5f, 0x30,
	0xdf, 0x67, 0xf2, 0xaa, 0x58, 0x23, 0xf0, 0xa7
};


#define bcmode_unittest(_name, _desc, _mode, _type, _in, _expected, _ov)\
unittest(_name, _desc)							\
{									\
	k_bc_t* c;							\
	uint8_t _out[sizeof(_in)];					\
	memcpy(_out, _in, sizeof(_in));					\
	if ((c = k_bc_init(BLK_CIPHER_AES)) == 0) {			\
		*details = "k_bcmode_init failed";			\
		return -1;						\
	}								\
	k_bcmode_set_mode(c, _mode, 0);					\
	uint32_t bs = k_bc_get_blocksize(c);				\
	if (k_bcmode_set_key(c, _key_192, sizeof(_key_192)*8, _type)) {	\
		*details = "k_bcmode_set_key failed";			\
		return -1;						\
	}								\
	k_bcmode_set_iv(c, _iv);					\
	if (!_ov) {							\
		k_bcmode_update(c, _in, _out, 3);			\
		k_bcmode_update(c, _in+(3*bs), _out+(3*bs), 1);		\
	} else {							\
		k_bcmode_update(c, _out, _out, 2);			\
		k_bcmode_update(c, _out+(2*bs), _out+(2*bs), 2);	\
	}								\
	k_bc_finish(c);							\
	if (!memcmp(_out, _expected, sizeof(_in))) {			\
		return 0;						\
	}								\
	else {								\
		dumphx("expected", _expected, sizeof(_in));		\
		dumphx("result  ", _out, sizeof(_in));			\
		return -1;						\
	}								\
	return -1;							\
}

bcmode_unittest(k_bc_update_ecb_e, "ECB encryption",
	BLK_CIPHER_MODE_ECB, BLK_CIPHER_KEY_ENCRYPT, _plain, _cipher_ecb, 0);

bcmode_unittest(k_bc_update_ecb_d, "ECB decryption",
	BLK_CIPHER_MODE_ECB, BLK_CIPHER_KEY_DECRYPT, _cipher_ecb, _plain, 0);

bcmode_unittest(k_bc_update_cbc_e, "CBC encryption",
	BLK_CIPHER_MODE_CBC, BLK_CIPHER_KEY_ENCRYPT, _plain, _cipher_cbc, 0);

bcmode_unittest(k_bc_update_cbc_d, "CBC decryption",
	BLK_CIPHER_MODE_CBC, BLK_CIPHER_KEY_DECRYPT, _cipher_cbc, _plain, 0);

bcmode_unittest(k_bc_update_cfb_e, "CFB encryption",
	BLK_CIPHER_MODE_CFB, BLK_CIPHER_KEY_ENCRYPT, _plain, _cipher_cfb, 0);

bcmode_unittest(k_bc_update_cfb_d, "CFB decryption",
	BLK_CIPHER_MODE_CFB, BLK_CIPHER_KEY_DECRYPT, _cipher_cfb, _plain, 0);

bcmode_unittest(k_bc_update_ofb_e, "OFB encryption",
	BLK_CIPHER_MODE_OFB, BLK_CIPHER_KEY_ENCRYPT, _plain, _cipher_ofb, 0);

bcmode_unittest(k_bc_update_ofb_d, "OFB decryption",
	BLK_CIPHER_MODE_OFB, BLK_CIPHER_KEY_DECRYPT, _cipher_ofb, _plain, 0);

bcmode_unittest(k_bc_update_ctr_e, "CTR encryption",
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, _plain, _cipher_ctr, 0);

bcmode_unittest(k_bc_update_ctr_d, "CTR decryption",
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_DECRYPT, _cipher_ctr, _plain, 0);

bcmode_unittest(k_bc_update_oecb_e, "Ov ECB encryption",
	BLK_CIPHER_MODE_ECB, BLK_CIPHER_KEY_ENCRYPT, _plain, _cipher_ecb, 1);

bcmode_unittest(k_bc_update_oecb_d, "Ov ECB decryption",
	BLK_CIPHER_MODE_ECB, BLK_CIPHER_KEY_DECRYPT, _cipher_ecb, _plain, 1);

bcmode_unittest(k_bc_update_ocbc_e, "Ov CBC encryption",
	BLK_CIPHER_MODE_CBC, BLK_CIPHER_KEY_ENCRYPT, _plain, _cipher_cbc, 1);

bcmode_unittest(k_bc_update_ocbc_d, "Ov CBC decryption",
	BLK_CIPHER_MODE_CBC, BLK_CIPHER_KEY_DECRYPT, _cipher_cbc, _plain, 1);

bcmode_unittest(k_bc_update_ocfb_e, "Ov CFB encryption",
	BLK_CIPHER_MODE_CFB, BLK_CIPHER_KEY_ENCRYPT, _plain, _cipher_cfb, 1);

bcmode_unittest(k_bc_update_ocfb_d, "Ov CFB decryption",
	BLK_CIPHER_MODE_CFB, BLK_CIPHER_KEY_DECRYPT, _cipher_cfb, _plain, 1);

bcmode_unittest(k_bc_update_oofb_e, "Ov OFB encryption",
	BLK_CIPHER_MODE_OFB, BLK_CIPHER_KEY_ENCRYPT, _plain, _cipher_ofb, 1);

bcmode_unittest(k_bc_update_oofb_d, "Ov OFB decryption",
	BLK_CIPHER_MODE_OFB, BLK_CIPHER_KEY_DECRYPT, _cipher_ofb, _plain, 1);

bcmode_unittest(k_bc_update_octr_e, "Ov CTR encryption",
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_ENCRYPT, _plain, _cipher_ctr, 1);

bcmode_unittest(k_bc_update_octr_d, "Ov CTR decryption",
	BLK_CIPHER_MODE_CTR, BLK_CIPHER_KEY_DECRYPT, _cipher_ctr, _plain, 1);
