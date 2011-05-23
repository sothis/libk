/*
 * libk - blockcipher.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

/* TODO:
 *	- currently _every_ cipher instance allocates a workbench, which
 * 	  in turn starts always cpu count threads per default. think about
 * 	  if we can make this more efficient.
 * */

#include <string.h>
#include "blockcipher/modes/bcmode_desc.h"
#include "utils/workbench.h"
#include "utils/mem.h"
#include "utils/err.h"

section_prologue(__blockciphers, struct blockcipher_desc);
section_prologue(__bcmodes, struct bcmode_desc);

static inline const struct bcmode_desc* k_bcmode_get_by_id
(enum bcmode_e id)
{
	foreach_section_item(struct bcmode_desc, modes, __bcmodes) {
		if (id == modes[i].id)
			return &modes[i];
	}
	return NULL;
}

static inline const struct blockcipher_desc* bc_get_by_id
(enum blockcipher_e id)
{
	foreach_section_item(struct blockcipher_desc, ciphers, __blockciphers) {
		if (id == ciphers[i].id)
			return &ciphers[i];
	}
	return NULL;
}

static inline size_t get_blocks_per_worker
(size_t workers, size_t blocks, size_t worker)
{
	return (blocks/workers) + (((blocks%workers) >= (worker + 1)) ? 1 : 0);
}

static inline size_t distribute_work
(
	struct mode_param_t pa[],
	k_bc_t* m,
	const void* i,
	void* o,
	size_t b)
{
	size_t bs = m->blockcipher->block_size;
	size_t nw = m->worker_threads;
	size_t worker = 0, distributed = 0;
	pa[worker].m = m;
	pa[worker].worker_num = 0;
	pa[worker].in = i;
	pa[worker].out = o;
	distributed += pa[worker].nblocks = get_blocks_per_worker(nw, b, 0);
	while ((++worker < nw) && (distributed < b)) {
		distributed += pa[worker].nblocks =
			get_blocks_per_worker(nw, b, worker);
		pa[worker].m = m;
		pa[worker].worker_num = worker;
		pa[worker].in = pa[worker-1].in + pa[worker-1].nblocks*bs;
		pa[worker].out = pa[worker-1].out + pa[worker-1].nblocks*bs;
	}
	return worker;
}

static inline void free_mode_resources
(struct k_bc_t* c)
{
	if (c->workbench)
		workbench_delete(c->workbench);
	if (c->worker_ivs) {
		for(size_t i = 0; i < c->worker_threads; i++)
			if (c->worker_ivs[i])
				k_free(c->worker_ivs[i]);
		k_free(c->worker_ivs);
	}
	c->mode = 0;
	c->tweaksize = 0;
	c->keysize = 0;
	c->workbench = 0;
	c->worker_ivs = 0;
	c->worker_threads = 1;
}

__export_function struct k_bc_t* k_bc_init
(enum blockcipher_e cipher)
{
	enum k_error_e err = K_ESUCCESS;
	struct k_bc_t* c = 0;

	c = k_calloc(1, sizeof(struct k_bc_t));
	if (!c) {
		err = K_ENOMEM;
		goto k_bcmode_init_err;
	}
	c->blockcipher = bc_get_by_id(cipher);
	if (!c->blockcipher) {
		err = K_ENOCIPHER;
		goto k_bcmode_init_err;
	}
	if (!c->blockcipher->schedule_size) {
		err = K_ESCHEDZERO;
		goto k_bcmode_init_err;
	}
	c->schedule = k_locked_calloc(1, c->blockcipher->schedule_size,
		&c->alloced_schedsize);
	if (!c->schedule) {
		err = K_ENOMEM;
		goto k_bcmode_init_err;
	}
	if (k_bcmode_set_mode(c, BLK_CIPHER_MODE_NOOP, 0))
		goto k_bcmode_init_err;

	return c;

k_bcmode_init_err:
	k_bc_finish(c);
	k_error(err);
	return NULL;
}

__export_function int32_t k_bcmode_set_mode
(struct k_bc_t* c, enum bcmode_e mode, size_t max_workers)
{
	enum k_error_e err = K_ESUCCESS;

	free_mode_resources(c);

	c->mode = k_bcmode_get_by_id(mode);
	if (!c->mode) {
		err = K_ENOMODE;
		goto k_bc_set_mode_err;
	}

	if (c->mode->encrypt_parallelizable ||
	c->mode->decrypt_parallelizable) {
		c->workbench = workbench_create(max_workers);
		if (!c->workbench) {
			err = K_ENOMEM;
			goto k_bc_set_mode_err;
		}
		c->worker_threads = workbench_get_running_workers(c->workbench);
		if (!c->worker_threads) {
			err = K_ENOWORKERS;
			goto k_bc_set_mode_err;
		}
	} else
		c->worker_threads = 1;

	c->worker_ivs = k_calloc(c->worker_threads, sizeof(uint8_t*));
	if (!c->worker_ivs) {
		err = K_ENOMEM;
		goto k_bc_set_mode_err;
	}
	for(size_t i = 0; i < c->worker_threads; i++) {
		c->worker_ivs[i] = k_calloc(c->blockcipher->block_size,
			sizeof(uint8_t));
		if (!c->worker_ivs[i]) {
			err = K_ENOMEM;
			goto k_bc_set_mode_err;
		}
	}

	return 0;

k_bc_set_mode_err:
	free_mode_resources(c);
	k_error(err);
	return -1;
}

__export_function void k_bc_finish
(struct k_bc_t* c)
{
	if (c) {
		free_mode_resources(c);
		if (c->schedule)
			k_locked_free(c->schedule, c->alloced_schedsize);
		k_free(c);
	}
}

/* low level api */

__export_function void k_bc_set_encrypt_key
(struct k_bc_t* c, const void* k, uint32_t bits)
{
	c->blockcipher->set_encrypt_key(c->schedule, k, bits);
}

__export_function void k_bc_set_decrypt_key
(struct k_bc_t* c, const void* k, uint32_t bits)
{
	c->blockcipher->set_decrypt_key(c->schedule, k, bits);
}

__export_function void k_bc_set_tweak
(struct k_bc_t* c, const void* t, uint32_t bits)
{
	c->blockcipher->set_tweak(c->schedule, t, bits);
}

__export_function void k_bc_encrypt
(struct k_bc_t* c, const void* i, void* o)
{
	c->blockcipher->encrypt(c->schedule, i, o);
}

__export_function void k_bc_decrypt
(struct k_bc_t* c, const void* i, void* o)
{
	c->blockcipher->decrypt(c->schedule, i, o);
}

__export_function size_t k_bc_get_blocksize
(struct k_bc_t* c)
{
	return c->blockcipher->block_size;
}

/* high level api using a specific blockcipher mode */

static uint32_t is_keysize_supported
(const struct blockcipher_desc* c, uint32_t keysize)
{
	if (keysize && c->key_range) {
		size_t i = 0;
		while(c->key_range[i].from) {
			if (is_in_range32(&c->key_range[i], keysize))
				return 1;
			i++;
		}
	}
	return 0;
}

static uint32_t is_tweaksize_supported
(const struct blockcipher_desc* c, uint32_t tweaksize)
{
	if (tweaksize && c->tweak_range) {
		size_t i = 0;
		while(c->tweak_range[i].from) {
			if (is_in_range32(&c->tweak_range[i], tweaksize))
				return 1;
			i++;
		}
	}
	return 0;
}

static uint32_t is_insecure
(const struct blockcipher_desc* c, uint32_t keysize, uint32_t tweaksize)
{
	if (c->insecure)
		return 1;
	if (keysize && c->insecure_key_range) {
		size_t i = 0;
		while(c->insecure_key_range[i].from) {
			if (is_in_range32(&c->insecure_key_range[i],
			keysize))
				return 1;
			i++;
		}
	}
	if (tweaksize && c->insecure_tweak_range) {
		size_t i = 0;
		while(c->insecure_tweak_range[i].from) {
			if (is_in_range32(&c->insecure_tweak_range[i],
			tweaksize))
				return 1;
			i++;
		}
	}
	return 0;
}

__export_function int32_t k_bcmode_set_key
(struct k_bc_t* c, const void* k, uint32_t bits, enum keytype_e t)
{
	enum k_error_e err = K_ESUCCESS;

	if (!c->mode) {
		err = K_ENOMODESET;
		goto k_bcmode_set_key_err;
	}

	c->keytype = t;
	c->keysize = bits;

	if (c->mode->insecure)
		c->insecure_encryption = 1;

	if (!is_keysize_supported(c->blockcipher, c->keysize)) {
		err = K_EINVKEYSIZE;
		goto k_bcmode_set_key_err;
	}

	if (is_insecure(c->blockcipher, c->keysize, c->tweaksize))
		c->insecure_encryption = 1;

	if (c->insecure_encryption) {
		k_warn(K_EINSECUREENC);
	}

	switch (c->keytype) {
		case BLK_CIPHER_KEY_ENCRYPT:
			c->blockcipher->set_encrypt_key(c->schedule, k,
				c->keysize);
			break;
		case BLK_CIPHER_KEY_DECRYPT:
			c->mode->decrypt_uses_encrypt_transform ?
			c->blockcipher->set_encrypt_key(c->schedule, k,
				c->keysize) :
			c->blockcipher->set_decrypt_key(c->schedule, k,
				c->keysize);
			break;
		default:
			err = K_EINVKEYTYPE;
			goto k_bcmode_set_key_err;
	}
	return 0;

k_bcmode_set_key_err:
	c->keytype = BLK_CIPHER_KEY_ENCRYPT;
	c->keysize = 0;
	k_error(err);
	return -1;
}

__export_function int32_t k_bcmode_set_tweak
(struct k_bc_t* c, const void* t, uint32_t bits)
{
	if (!c->blockcipher->set_tweak) {
		k_error(K_ENOTWEAKSET);
		return -1;
	}

	if (!is_tweaksize_supported(c->blockcipher, c->tweaksize)) {
		k_error(K_EINVTWEAKSIZE);
		return -1;
	}
	c->tweaksize = bits;
	c->blockcipher->set_tweak(c->schedule, t, c->tweaksize);
	return 0;
}

__export_function void k_bcmode_set_iv
(struct k_bc_t* c, const void* iv)
{
	memcpy(c->worker_ivs[0], iv, c->blockcipher->block_size);
}

static void parallel_worker
(void* arg)
{
	struct mode_param_t* p = arg;
	struct k_bc_t* m = p->m;
	size_t bs = m->blockcipher->block_size;
	const void* i = p->in;
	void* o = p->out;
	size_t nblocks = p->nblocks;

	if (m->keytype == BLK_CIPHER_KEY_ENCRYPT)
		for (size_t b = 0; b < nblocks; ++b)
			m->mode->encrypt(m,i+(b*bs),o+(b*bs),p->worker_num);
	else
		for (size_t b = 0; b < nblocks; ++b)
			m->mode->decrypt(m,i+(b*bs),o+(b*bs),p->worker_num);
}

static void process_block_parallel
(struct k_bc_t* m, const void* i, void* o, size_t b)
{
	struct mode_param_t pa[m->worker_threads];
	size_t worker = distribute_work(pa, m, i, o, b);

	if (m->mode->prepare_iv && worker > 1)
		m->mode->prepare_iv(m, pa, worker);

	workbench_set_worker(m->workbench, &parallel_worker);
	for (size_t w = 0; w < worker; ++w) {
		workbench_commit(m->workbench, &pa[w]);
	}
	workbench_join(m->workbench);

	/* set main iv to the one produced by the last worker */
	if (m->mode->prepare_iv && worker > 1)
		memcpy(m->worker_ivs[0], m->worker_ivs[worker-1],
			m->blockcipher->block_size);
}

static void process_block
(struct k_bc_t* c, const void* i, void* o)
{
	if (c->keytype == BLK_CIPHER_KEY_ENCRYPT)
		c->mode->encrypt(c, i, o, 0);
	else
		c->mode->decrypt(c, i, o, 0);
}

__export_function void k_bcmode_update
(struct k_bc_t* c, const void* i, void* o, size_t blocks)
{
	size_t bs = c->blockcipher->block_size;

	if ((c->worker_threads > 1) && ((blocks / c->worker_threads) >= 512)) {
		if (c->keytype == BLK_CIPHER_KEY_ENCRYPT &&
			c->mode->encrypt_parallelizable)
			process_block_parallel(c, i, o, blocks);

		else if (c->keytype == BLK_CIPHER_KEY_DECRYPT &&
			c->mode->decrypt_parallelizable)
			process_block_parallel(c, i, o, blocks);
	} else
		for (size_t b = 0; b < blocks; ++b)
			process_block(c, i+(b*bs), o+(b*bs));
}
