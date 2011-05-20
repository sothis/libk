#include <string.h>
#include "bcmode_desc.h"
#include "utils/xor.h"

static void ofb_crypt
(struct k_bc_t* m, const void* i, void* o, size_t worker)
{
	size_t bs = m->blockcipher->block_size;
	uint8_t* iv = m->worker_ivs[0];
	uint8_t b[bs];

	memcpy(b, i, bs);

	m->blockcipher->encrypt(m->schedule, iv, o);
	memcpy(iv, o, bs);
	xorb_64(o, o, b, bs);
}


static const char* const authors[] = {
	"Janos Laube <janos.dev@gmail.com>",
	0
};

blockciphermode_start(OFB, "Output Feedback")
	.authors			= authors,
	.decrypt_uses_encrypt_transform	= 1,
	.encrypt			= &ofb_crypt,
	.decrypt			= &ofb_crypt,
blockciphermode_end
