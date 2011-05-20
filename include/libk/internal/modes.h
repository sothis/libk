/* ID's must be unique and assigned ID's must not be reassigned anymore */

enum bcmode_e {
	BLK_CIPHER_MODE_NOOP			= 0,
	BLK_CIPHER_MODE_ECB			= 1,
	BLK_CIPHER_MODE_CBC			= 2,
	BLK_CIPHER_MODE_CFB			= 3,
	BLK_CIPHER_MODE_OFB			= 4,
	BLK_CIPHER_MODE_CTR			= 5,
	/* insert new modes above this line and adjust
	 * BLK_CIPHER_MODE_MAX_SUPPORT below accordingly */
	BLK_CIPHER_MODE_MAX_SUPPORT		= BLK_CIPHER_MODE_CTR
};

/* if the blockcipher mode turns the block cipher into a stream cipher
 * i.e. no padding is required and decryption uses encrypt transform,
 * add the mode below */
enum bcstreammode_e {
	BLK_CIPHER_STREAMMODE_CFB		= BLK_CIPHER_MODE_CFB,
	BLK_CIPHER_STREAMMODE_OFB		= BLK_CIPHER_MODE_OFB,
	BLK_CIPHER_STREAMMODE_CTR		= BLK_CIPHER_MODE_CTR,
};
