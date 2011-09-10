/*
 * libk - pres.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <libk/libk.h>
#include "utils/sections.h"
#include "utils/err.h"
#include "utils/mem.h"
#include "utils/dumphx.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef __WINNT__
#include <sys/mman.h>
#else
#include "utils/ntwrap.h"
#endif


#if defined(__DARWIN__)
#define O_NOATIME	0
#elif defined(__WINNT__)
#define O_NOATIME	_O_BINARY
#endif

static int pres_map(struct mmap_t* res, int fd, size_t len, off_t offset)
{
	off_t map_off, diff;

	memset(res, 0, sizeof(struct mmap_t));
	/* TODO: determine pagesize once at program startup */
	map_off = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
	diff = offset - map_off;
	void* data = mmap(0, len + diff, PROT_READ|PROT_WRITE, MAP_PRIVATE,
		fd, map_off);
	if (data == MAP_FAILED)
		return -1;
	res->mem = data + diff;
	res->off = diff;
	res->len = len + diff;
	return 0;
}

static int pres_unmap(struct mmap_t* res)
{
	return munmap(res->mem - res->off, res->len);
}

static int pres_rollback(struct pres_file_t* pf)
{
	if (ftruncate(pf->fd, pf->cur_stringpoolstart)) {
		return -1;
	}
	if (lseek(pf->fd, 0, SEEK_END) == -1) {
		return -1;
	}
	if (fsync(pf->fd)) {
		return -1;
	}
	return 0;
}

__export_function int k_pres_init_new_resource
(struct pres_file_t* pf)
{
	int res = 0;
	uint8_t data_nonce[PRES_MAX_IV_LENGTH];


	if (pf->is_corrupt)
		goto unrecoverable_err;

	if (pf->scipher) {
		memset(data_nonce, 0, PRES_MAX_IV_LENGTH);
		k_prng_update(pf->prng, data_nonce, pf->nonce_size);
	}

	if (pf->cur_allocedentries < (pf->cur_resentries+1)) {
		pf->cur_allocedentries += 32768;
		void* temp = realloc(pf->rtbl,
			sz_res_tbl + pf->cur_allocedentries*sz_res_tbl_entry);
		if (!temp)
			goto unrecoverable_err;
		pf->rtbl = temp;
		memset(&pf->rtbl->table[pf->cur_resentries], 0,
			32768*sz_res_tbl_entry);
	}

	k_hash_reset(pf->hash);
	if (pf->scipher) {
		k_sc_set_key(pf->scipher, data_nonce, pf->key, pf->hdr.keysize);
		memcpy(pf->rtbl->table[pf->cur_resentries].data_iv,
			data_nonce, pf->nonce_size);
	}
	pf->rtbl->table[pf->cur_resentries].data_size = 0;

	goto out;
unrecoverable_err:
	pf->is_corrupt = 1;
	res = -1;
out:
	return res;
}

__export_function int k_pres_append_to_new_resource
(struct pres_file_t* pf, const void* data, size_t length)
{
	int res = -1;
	size_t filebytes = 0;
	ssize_t nwritten, total = 0;
	size_t blocks = (length / PRES_IOBUF_SIZE);
	size_t rest = (length % PRES_IOBUF_SIZE);

	if (pf->is_corrupt)
		goto out;

	for (size_t i = 0; i < blocks; ++i) {
		memmove(pf->iobuf, data+(i*PRES_IOBUF_SIZE), PRES_IOBUF_SIZE);
		k_hash_update(pf->hash, pf->iobuf, PRES_IOBUF_SIZE);
		if (pf->scipher)
			k_sc_update(pf->scipher, pf->iobuf,
				pf->iobuf, PRES_IOBUF_SIZE);
		while (total != PRES_IOBUF_SIZE) {
			nwritten = write(pf->fd, pf->iobuf+total,
				PRES_IOBUF_SIZE-total);
			if (nwritten < 0)
				goto recoverable_err;
			total += nwritten;
		}
	}
	filebytes += total;
	total = 0;
	if (rest) {
		memmove(pf->iobuf, data+(blocks*PRES_IOBUF_SIZE), rest);
		k_hash_update(pf->hash, pf->iobuf, rest);
		if (pf->scipher)
			k_sc_update(pf->scipher, pf->iobuf,
				pf->iobuf, rest);
		while (total != rest) {
			nwritten = write(pf->fd, pf->iobuf+total,
				rest-total);
			if (nwritten < 0)
				goto recoverable_err;
			total += nwritten;
		}
	}
	filebytes += total;

	pf->rtbl->table[pf->cur_resentries].data_size += filebytes;

	res = 0;
	goto out;
recoverable_err:
	res = 1;
	pres_rollback(pf);
out:
	return res;
}

__export_function int k_pres_commit_new_resource
(struct pres_file_t* pf, const char* name, size_t basename_off, uint64_t uuid)
{
	int res = -1;
	size_t namelen = strlen(name)+1;

	if (pf->is_corrupt)
		goto out;

	if (namelen == 1)
		goto recoverable_err;

	if (pool_append(&pf->stringpool, name, namelen))
		goto unrecoverable_err;
#if 1
	/* TODO: make this optional */
	if (fsync(pf->fd))
		goto unrecoverable_err;
#endif
	pf->cur_resentries++;

	k_hash_final(pf->hash,
		pf->rtbl->table[pf->cur_resentries-1].data_digest);

	k_hash_reset(pf->hash);
	k_hash_update(pf->hash, name, namelen);
	k_hash_final(pf->hash,
		pf->rtbl->table[pf->cur_resentries-1].filename_digest);

	pf->rtbl->table[pf->cur_resentries-1].id = pf->cur_resentries;
	pf->rtbl->table[pf->cur_resentries-1].uuid = uuid;
	pf->rtbl->table[pf->cur_resentries-1].data_offset =
		pf->cur_datapoolsize;

	pf->rtbl->table[pf->cur_resentries-1].filename_size = namelen;
	pf->rtbl->table[pf->cur_resentries-1].filename_offset =
		pf->cur_stringpoolsize;
	pf->rtbl->table[pf->cur_resentries-1].basename_offset =
		basename_off;
	pf->cur_stringpoolsize += namelen;

	pf->cur_datapoolsize += pf->rtbl->table[pf->cur_resentries-1].data_size;
	pf->cur_stringpoolstart = pf->cur_datapoolstart + pf->cur_datapoolsize;
	pf->cur_rtbl_start = pf->cur_stringpoolstart + pf->cur_stringpoolsize;

	pf->cur_filesize = pf->cur_rtbl_start + pf->dhdr.resource_table_size +
		pf->cur_resentries*sz_res_tbl_entry;

	res = 0;
	goto out;
unrecoverable_err:
	pf->is_corrupt = 1;
	res = -1;
	goto out;
recoverable_err:
	res = 1;
	pres_rollback(pf);
out:
	return res;
}

__export_function int k_pres_add_file
(struct pres_file_t* pf, const char* name, size_t basename_off, uint64_t uuid)
{
	int res = 0, fd = -1;
	res = k_pres_init_new_resource(pf);
	if (res)
		goto out;
	res = 1;

	#ifdef __WINNT__
	wchar_t* wc = utf8_to_ucs2(name);
	if (!wc)
		fd = -1;
	else {
		fd = _wopen(wc, O_RDONLY|_O_BINARY);
		free(wc);
	}
	#else
	fd = open(name, O_RDONLY | O_NOATIME);
	#endif

	if (fd == -1) {
		goto out;
	}

	/* strip leading '.' and '/' components */
	size_t name_off = 0;
	const char* n = name;
	while (*n && *n != '/') {
		name_off++;
		n++;
	}
	if (*n) {
		name_off++;
		name += name_off;
		basename_off -= name_off;
	}

	/* not adding hidden objects */
	n = name;
	if (!memcmp(n, ".", 1))
		goto out;
	while (*n) {
		if (!memcmp(n, "/.", 2))
			goto out;
		n++;
	}

	ssize_t nread;
	while ((nread = read(fd, pf->iobuf, PRES_IOBUF_SIZE)) > 0) {
		res = k_pres_append_to_new_resource(pf, pf->iobuf, nread);
		if (res)
			goto out;
	}
	if (nread < 0)
		goto out;

	res = k_pres_commit_new_resource(pf, name, basename_off, uuid);
out:
	if (fd != -1)
		close(fd);
	return res;
}

static inline int _addu64(uint64_t* res, uint64_t a, uint64_t b)
{
	uint64_t c = a + b;
	if (!b || (c > a)) {
		*res = c;
		return 0;
	}
	return -1;
}

static k_sc_t* _init_streamcipher
(struct pres_file_header_t* hdr)
{
	k_sc_t* c = 0;

	if (hdr->ciphermode)
		c = k_sc_init_with_blockcipher(hdr->cipher, hdr->ciphermode, 0);
	else
		/* for simplicity we use nonce bits == key bits here */
		c = k_sc_init(hdr->cipher, hdr->keysize);

	if (!c)
		return 0;

	return c;
}

#if 0
static void _cryptonce(k_sc_t* c, void* mem, const void* nonce, size_t s)
{
	k_sc_set_nonce(c, nonce);
	k_sc_update(c, mem, mem, s);
}
#endif

static int _open_pres(const char* name, uint32_t writable)
{
	struct stat st;
	int fd = -1;

	if (!writable)
		fd = open(name, O_RDONLY|O_NOATIME);
	else
		fd = open(name, O_RDWR);
	if (fd == -1)
		goto failed;
	if (fstat(fd, &st))
		goto failed;
	if (!S_ISREG(st.st_mode))
		goto failed;
	if (st.st_size < (sz_file_header + sz_detached_hdr))
		goto failed;

	return fd;
failed:
	if (fd)
		close(fd);
	return -1;
}

static int _verify_file_header(struct pres_file_t* pf)
{
	uint8_t digest_chk[PRES_MAX_DIGEST_LENGTH];
	enum k_error_e err = K_ESUCCESS;
	struct stat st;
	int digest_bits = pf->hdr.hashsize;
	int hashfn = pf->hdr.hashfunction;
	size_t digest_bytes = (digest_bits + 7) / 8;

	if (fstat(pf->fd, &st))
		goto invalid;
	if (pf->hdr.magic != PRES_MAGIC)
		goto invalid;
	/* currently no back and forward compatibility */
	if (pf->hdr.version != PRES_VER)
		goto invalid;
	if (pf->hdr.filesize != st.st_size)
		goto invalid;
	if (!hashfn || hashfn > HASHSUM_MAX_SUPPORT)
		goto invalid;
	if (!digest_bytes || digest_bytes > PRES_MAX_DIGEST_LENGTH)
		goto invalid;

	pf->hash = k_hash_init(hashfn, digest_bits);
	if (!pf->hash)
		goto invalid;

	k_hash_update(pf->hash, &pf->hdr, sz_header_digest);
	k_hash_final(pf->hash, digest_chk);
	if (memcmp(pf->hdr.digest, digest_chk, digest_bytes)) {
		err = K_EWRONGDIGEST_FHDR;
		goto invalid;
	}

	if (pf->hdr.ciphermode > BLK_CIPHER_MODE_MAX_SUPPORT)
		goto invalid;

	if (pf->hdr.ciphermode) {
		if (!pf->hdr.cipher || pf->hdr.cipher > BLK_CIPHER_MAX_SUPPORT)
			goto invalid;
	} else if (pf->hdr.cipher && pf->hdr.cipher > STREAM_CIPHER_MAX_SUPPORT)
		goto invalid;

	if (pf->hdr.cipher && !pf->hdr.keysize)
		goto invalid;

	uint64_t dhdr_end = 0;
	if (_addu64(&dhdr_end, pf->hdr.detached_header_start,
	pf->hdr.detached_header_size))
		goto invalid;

	if (dhdr_end > pf->hdr.filesize)
		goto invalid;

	pf->cur_filesize = pf->hdr.filesize;

	return 0;

invalid:
	k_error(err);
	return -1;
}

static int _get_file_header(struct pres_file_t* pf)
{
	struct mmap_t map;
	if (pres_map(&map, pf->fd, sz_file_header, 0))
		return -1;
	memcpy(&pf->hdr, map.mem, sz_file_header);
	pres_unmap(&map);

	if (_verify_file_header(pf))
		return -1;

	return 0;
}

static int _verify_detached_header(struct pres_file_t* pf)
{
	uint8_t digest_chk[PRES_MAX_DIGEST_LENGTH];
	enum k_error_e err = K_ESUCCESS;
	size_t digest_bytes = (pf->hdr.hashsize + 7) / 8;

	k_hash_reset(pf->hash);
	k_hash_update(pf->hash, &pf->dhdr, sz_dheader_digest);
	k_hash_final(pf->hash, digest_chk);
	if (memcmp(pf->dhdr.digest, digest_chk, digest_bytes)) {
		err = K_EWRONGDIGEST_DHDR;
		goto invalid;
	}

	uint64_t rtbl_end = 0;
	if (_addu64(&rtbl_end, pf->dhdr.resource_table_start,
	pf->dhdr.resource_table_size))
		goto invalid;

	if (rtbl_end > pf->hdr.filesize)
		goto invalid;

	return 0;

invalid:
	k_error(err);
	return -1;
}

static int _get_detached_header(struct pres_file_t* pf)
{
	struct mmap_t map;
	if (pres_map(&map, pf->fd, sz_detached_hdr, sz_file_header))
		return -1;
	memcpy(&pf->dhdr, map.mem, sz_detached_hdr);
	pres_unmap(&map);

	if (pf->scipher) {
		k_sc_set_key(pf->scipher, pf->hdr.detached_header_iv,
			pf->key, pf->hdr.keysize);
		k_sc_update(pf->scipher, &pf->dhdr, &pf->dhdr, sz_detached_hdr);
	}

	if (_verify_detached_header(pf))
		return -1;

	pf->cur_rtbl_start = pf->dhdr.resource_table_start;

	return 0;
}

static int _verify_rtbl_entries(struct pres_file_t* pf)
{
	uint8_t digest_chk[PRES_MAX_DIGEST_LENGTH];
	size_t digest_bytes = (pf->hdr.hashsize + 7) / 8;

	/* TODO: maybe don't make this a hard error. just mark the
	 * entry as corrupt and try to procceed as far as possible */
	for (uint64_t i = 0; i < pf->rtbl->entries; ++i) {
		struct pres_resource_table_entry_t* e = &pf->rtbl->table[i];
		/* entry was marked as deleted */
		if (!e->id)
			continue;

		k_hash_reset(pf->hash);
		k_hash_update(pf->hash, e, sz_rtblentry_digest);
		k_hash_final(pf->hash, digest_chk);
		if (memcmp(e->digest, digest_chk, digest_bytes))
			goto invalid;

		uint64_t filename_end = 0;
		if (_addu64(&filename_end,
		e->filename_offset + pf->rtbl->string_pool_start,
		e->filename_size))
			goto invalid;
		if (filename_end > pf->hdr.filesize)
			goto invalid;

		uint64_t data_end = 0;
		if (_addu64(&data_end,
		e->data_offset + pf->rtbl->data_pool_start,
		e->data_size))
			goto invalid;
		if (data_end > pf->hdr.filesize)
			goto invalid;
	}

	return 0;
invalid:
	return -1;
}

static int _get_rtbl_entries(struct pres_file_t* pf)
{
	struct mmap_t map;
	size_t entries_size = pf->rtbl->entries*sz_res_tbl_entry;
	size_t new_size = sz_res_tbl + entries_size;
	size_t entries_off = pf->dhdr.resource_table_start+sz_res_tbl;

	void* t = realloc(pf->rtbl, new_size);
	if (!t)
		return -1;
	pf->rtbl = t;

	if (pres_map(&map, pf->fd, entries_size, entries_off))
		return -1;

	memcpy(pf->rtbl->table, map.mem, entries_size);
	pres_unmap(&map);

	if (pf->scipher)
		k_sc_update(pf->scipher, pf->rtbl->table, pf->rtbl->table,
			entries_size);

	if (_verify_rtbl_entries(pf))
		return -1;

	pf->cur_allocedentries = pf->rtbl->entries;
	pf->cur_resentries =  pf->rtbl->entries;

	return 0;
}

static int _verify_rtbl(struct pres_file_t* pf)
{
	int res = 0;
	uint8_t digest_chk[PRES_MAX_DIGEST_LENGTH];
	k_hash_t* hash = 0;
	int digest_bits = pf->hdr.hashsize;
	int hashfn = pf->hdr.hashfunction;
	size_t digest_bytes = (digest_bits + 7) / 8;

	hash = k_hash_init(hashfn, digest_bits);
	if (!hash)
		goto invalid;

	k_hash_reset(pf->hash);
	k_hash_update(hash, pf->rtbl, sz_rtbl_digest);
	k_hash_final(hash, digest_chk);
	if (memcmp(pf->rtbl->digest, digest_chk, digest_bytes))
		goto invalid;

	uint64_t stringpool_end = 0;
	if (_addu64(&stringpool_end, pf->rtbl->string_pool_start,
	pf->rtbl->string_pool_size))
		goto invalid;

	if (stringpool_end > pf->hdr.filesize)
		goto invalid;

	uint64_t datapool_end = 0;
	if (_addu64(&datapool_end, pf->rtbl->data_pool_start,
	pf->rtbl->data_pool_size))
		goto invalid;

	if (datapool_end > pf->hdr.filesize)
		goto invalid;

	if (!pf->rtbl->entries)
		goto invalid;

	goto valid;
invalid:
	res = -1;
valid:
	if (hash)
		k_hash_finish(hash);
	return res;
}

static int _get_rtbl(struct pres_file_t* pf)
{
	struct mmap_t map;

	pf->rtbl = calloc(1, sz_res_tbl);
	if (!pf->rtbl)
		goto fail;

	if (pres_map(&map, pf->fd, sz_res_tbl, pf->dhdr.resource_table_start))
		goto fail;
	memcpy(pf->rtbl, map.mem, sz_res_tbl);
	pres_unmap(&map);

	if (pf->scipher) {
		k_sc_set_key(pf->scipher, pf->dhdr.resource_table_iv,
			pf->key, pf->hdr.keysize);
		k_sc_update(pf->scipher, pf->rtbl, pf->rtbl, sz_res_tbl);
	}

	if (_verify_rtbl(pf))
		goto fail;

	if (_get_rtbl_entries(pf))
		goto fail;

	pf->cur_datapoolstart = pf->rtbl->data_pool_start;
	pf->cur_datapoolsize = pf->rtbl->data_pool_size;
	pf->cur_stringpoolstart = pf->rtbl->string_pool_start;
	pf->cur_stringpoolsize = pf->rtbl->string_pool_size;

	return 0;
fail:
	if (pf->rtbl) {
		free(pf->rtbl);
		pf->rtbl = 0;
	}
	return -1;
}

static int _verify_stringpool(struct pres_file_t* pf)
{
	uint8_t digest_chk[PRES_MAX_DIGEST_LENGTH];
	size_t digest_bytes = (pf->hdr.hashsize + 7) / 8;

	/* TODO: maybe don't make this a hard error. if the data is valid, we
	 * still can export it, just the name is lost. */
	for (uint64_t i = 0; i < pf->rtbl->entries; ++i) {
		struct pres_resource_table_entry_t* e = &pf->rtbl->table[i];
		/* entry was marked as deleted */
		if (!e->id)
			continue;

		uint64_t fn_off = e->filename_offset;
		uint64_t base_off = e->basename_offset;
		const char* fn = pool_getmem(&pf->stringpool, fn_off);
		uint64_t fns = strlen(fn)+1;
		if (base_off >= fns-1)
			goto invalid;

		k_hash_reset(pf->hash);
		k_hash_update(pf->hash, fn, fns);
		k_hash_final(pf->hash, digest_chk);
		if (memcmp(e->filename_digest, digest_chk, digest_bytes))
			goto invalid;
	}

	return 0;
invalid:
	return -1;
}

static int _get_stringpool(struct pres_file_t* pf)
{
	struct mmap_t map;
	if (pool_alloc(&pf->stringpool, PRES_IOBUF_SIZE))
		return -1;

	if (pres_map(&map, pf->fd, pf->rtbl->string_pool_size,
		pf->rtbl->string_pool_start)) {
		pool_free(&pf->stringpool);
		return -1;
	}

	if (pool_append(&pf->stringpool, map.mem, pf->rtbl->string_pool_size)) {
		pool_free(&pf->stringpool);
		pres_unmap(&map);
		return -1;
	}

	pres_unmap(&map);

	if (pf->scipher) {
		k_sc_set_key(pf->scipher, pf->rtbl->string_pool_iv,
			pf->key, pf->hdr.keysize);
		k_sc_update(pf->scipher, pf->stringpool.base,
			pf->stringpool.base, pf->stringpool.data_size);
	}

	if (_verify_stringpool(pf)) {
		pool_free(&pf->stringpool);
		return -1;
	}

	return 0;
}

static int _pres_open_key
(struct pres_file_t* pf, const char* name, const void* key, uint32_t writable)
{
	memset(pf, 0, sizeof(struct pres_file_t));
	pf->fd = -1;

	pf->fd = _open_pres(name, writable);
	if (pf->fd == -1)
		goto failed;
	if (_get_file_header(pf))
		goto failed;
	if (pf->hdr.cipher) {
		if (!key)
			goto failed;
		pf->key = k_calloc((pf->hdr.keysize+7)/8, 1);
		if (!pf->key)
			goto failed;
		memcpy(pf->key, key, (pf->hdr.keysize+7)/8);
		pf->scipher = _init_streamcipher(&pf->hdr);
		pf->nonce_size = k_sc_get_nonce_bytes(pf->scipher);
		if (!pf->scipher)
			goto failed;
	}
	pf->prng = k_prng_init(PRNG_PLATFORM);
	if (!pf->prng)
		goto failed;

	if (_get_detached_header(pf))
		goto failed;

	if (_get_rtbl(pf))
		goto failed;

	if (_get_stringpool(pf))
		goto failed;

	pf->iobuf = malloc(PRES_IOBUF_SIZE);
	if (!pf->iobuf)
		goto failed;

	if (lseek(pf->fd, pf->cur_datapoolstart+pf->cur_datapoolsize,
	SEEK_SET) == -1)
		goto failed;

	pf->is_open = 1;
	pf->is_writable = writable;
	return 0;
failed:
	if (pf->key)
		k_free(pf->key);
	if (pf->hash)
		k_hash_finish(pf->hash);
	if (pf->prng)
		k_prng_finish(pf->prng);
	if (pf->iobuf)
		free(pf->iobuf);
	if (pf->scipher)
		k_sc_finish(pf->scipher);
	if (pf->rtbl)
		free(pf->rtbl);
	if (pf->stringpool.alloced)
		pool_free(&pf->stringpool);
	if (pf->fd != -1)
		close(pf->fd);
	return -1;
}

__export_function int k_pres_needs_pass(const char* name)
{
	struct pres_file_t pf;
	memset(&pf, 0, sizeof(struct pres_file_t));

	pf.fd = _open_pres(name, 0);
	if (pf.fd == -1)
		return -1;
	if (_get_file_header(&pf)) {
		close(pf.fd);
		return -1;
	}
	close(pf.fd);
	if (pf.hash)
		k_hash_finish(pf.hash);

	if (pf.hdr.cipher)
		return 1;
	return 0;
}

/* TODO: add k_pres_open variant with key instead of pass */
__export_function int k_pres_open
(struct pres_file_t* pf, const char* name, const char* pass, uint32_t writable)
{
	void* key = 0;
	int res = -1;
	memset(pf, 0, sizeof(struct pres_file_t));

	if (pass)  {
		/* retrieve the kdf salt from the file */
		pf->fd = _open_pres(name, 0);
		if (pf->fd == -1)
			return -1;
		if (_get_file_header(pf)) {
			close(pf->fd);
			return -1;
		}
		close(pf->fd);
		if (pf->hash)
			k_hash_finish(pf->hash);

		key = _k_key_derive_simple1024(pass,
			pf->hdr.kdf_salt, PRES_KDF_ITERATIONS);
	}

	res = _pres_open_key(pf, name, key, writable);
	free(key);
	return res;
}

static int _set_file_header
(struct pres_file_t* pf, struct pres_options_t* opt)
{
	if (!opt->name)
		return -1;
	if (!opt->hashsum)
		return -1;
	pf->fd = -1;
	pf->hdr.magic = PRES_MAGIC;
	pf->hdr.version = PRES_VER;
	pf->hdr.hashfunction = opt->hashsum;
	pf->hdr.hashsize = k_hash_digest_bits(pf->hash);
	if (opt->streamcipher)
		pf->hdr.cipher = opt->streamcipher;
	else if (opt->blockcipher) {
		pf->hdr.cipher = opt->blockcipher;
		pf->hdr.ciphermode = opt->ciphermode;
	}
	pf->hdr.keysize = opt->keysize;
	pf->hdr.tweaksize = opt->tweaksize;
	pf->hdr.detached_header_size = sz_detached_hdr;
	pf->hdr.detached_header_start = sz_file_header;
	pf->dhdr.resource_table_size = sz_res_tbl;

	pf->cur_rtbl_start = sz_detached_hdr + sz_file_header;
	pf->cur_datapoolstart = sz_detached_hdr + sz_file_header;
	pf->cur_stringpoolstart = sz_detached_hdr + sz_file_header;
	pf->cur_filesize = sz_file_header + sz_detached_hdr + sz_res_tbl;

	if (!pf->hdr.hashsize)
		return -1;

	strncpy(pf->hdr.libk_version_string, k_version_string(),
		(sizeof(pf->hdr.libk_version_string)) - 1);

	return 0;
}

__export_function int k_pres_create
(struct pres_file_t* pf, struct pres_options_t* opt)
{
	memset(pf, 0, sizeof(struct pres_file_t));

	pf->hash = k_hash_init(opt->hashsum, opt->hashsize);
	if (!pf->hash)
		goto err_out;
	pf->prng = k_prng_init(PRNG_PLATFORM);
	if (!pf->prng)
		goto err_out;

	if (_set_file_header(pf, opt))
		goto err_out;

	pf->rtbl = calloc(1, sz_res_tbl);
	if (!pf->rtbl)
		goto err_out;
	pf->cur_allocedentries = 0;

	pf->iobuf = malloc(PRES_IOBUF_SIZE);
	if (!pf->iobuf)
		goto err_out;

	if (pf->hdr.cipher) {
		if (!opt->key && !opt->pass)
			goto err_out;
		if (opt->key) {
			pf->scipher = _init_streamcipher(&pf->hdr);
			pf->key = k_calloc((pf->hdr.keysize+7)/8, 1);
			if (!pf->key)
				goto err_out;
			memcpy(pf->key, opt->key, (pf->hdr.keysize+7)/8);
		} else {
			k_prng_update(pf->prng, pf->hdr.kdf_salt,
				PRES_MAX_IV_LENGTH);
			pf->key = _k_key_derive_simple1024(opt->pass,
				pf->hdr.kdf_salt, PRES_KDF_ITERATIONS);
			if (!pf->key)
				goto err_out;
			pf->scipher = _init_streamcipher(&pf->hdr);
		}
		if (!pf->scipher)
			goto err_out;
		pf->nonce_size = k_sc_get_nonce_bytes(pf->scipher);
	}

#ifdef NDEBUG
	/* make this optional */
	/* pf->fd = k_tcreat(opt->name, 0400); */
	pf->fd = k_tcreat(opt->name, 0600);
#else
	pf->fd = k_tcreat(opt->name, 0600);
#endif
	if (pf->fd == -1)
		goto err_out;

	if (pool_alloc(&pf->stringpool, PRES_IOBUF_SIZE))
		goto err_out;

	if (lseek(pf->fd, pf->cur_datapoolstart, SEEK_SET) == -1)
		goto err_out;

	pf->is_open = 1;
	pf->is_writable = 1;
	return 0;

err_out:
	if (pf->iobuf)
		free(pf->iobuf);
	if (pf->hash)
		k_hash_finish(pf->hash);
	if (pf->stringpool.alloced)
		pool_free(&pf->stringpool);
	if (pf->prng)
		k_prng_finish(pf->prng);
	if (pf->scipher)
		k_sc_finish(pf->scipher);
	if (pf->fd != -1)
		k_trollback_and_close(pf->fd);
	if (pf->rtbl)
		free(pf->rtbl);
	if (pf->key)
		k_free(pf->key);
	return -1;
}

static int _commit_and_close(struct pres_file_t* pf)
{
	uint8_t dhdr_nonce[PRES_MAX_IV_LENGTH];
	uint8_t rtbl_nonce[PRES_MAX_IV_LENGTH];
	uint8_t spool_nonce[PRES_MAX_IV_LENGTH];
	size_t s;
	int res = 0;

	if (pf->is_corrupt)
		goto err_out;

	if (pf->scipher) {
		memset(dhdr_nonce, 0, PRES_MAX_IV_LENGTH);
		memset(rtbl_nonce, 0, PRES_MAX_IV_LENGTH);
		memset(spool_nonce, 0, PRES_MAX_IV_LENGTH);
		k_prng_update(pf->prng, dhdr_nonce, pf->nonce_size);
		k_prng_update(pf->prng, rtbl_nonce, pf->nonce_size);
		k_prng_update(pf->prng, spool_nonce, pf->nonce_size);
	}

	pf->hdr.filesize = pf->cur_filesize;
	pf->dhdr.resource_table_start = pf->cur_rtbl_start;
	pf->rtbl->entries = pf->cur_resentries;
	pf->rtbl->data_pool_start = pf->cur_datapoolstart;
	pf->rtbl->data_pool_size = pf->cur_datapoolsize;
	pf->rtbl->string_pool_start = pf->cur_stringpoolstart;
	pf->rtbl->string_pool_size = pf->cur_stringpoolsize;

	if (pf->scipher) {
		memcpy(pf->hdr.detached_header_iv, dhdr_nonce, pf->nonce_size);
		memcpy(pf->dhdr.resource_table_iv, rtbl_nonce, pf->nonce_size);
		memcpy(pf->rtbl->string_pool_iv, spool_nonce, pf->nonce_size);
	}

	k_hash_reset(pf->hash);
	k_hash_update(pf->hash, &pf->hdr, sz_header_digest);
	k_hash_final(pf->hash, pf->hdr.digest);
	if (lseek(pf->fd, 0, SEEK_SET) == -1)
		goto err_out;
	s = sz_file_header;
	if (write(pf->fd, &pf->hdr, s) != s)
		goto err_out;

	k_hash_reset(pf->hash);
	k_hash_update(pf->hash, &pf->dhdr, sz_dheader_digest);
	k_hash_final(pf->hash, pf->dhdr.digest);

	if (lseek(pf->fd, pf->hdr.detached_header_start, SEEK_SET) == -1)
		goto err_out;
	s = sz_detached_hdr;
	if (pf->scipher) {
		k_sc_set_key(pf->scipher, dhdr_nonce,
			pf->key, pf->hdr.keysize);
		k_sc_update(pf->scipher, &pf->dhdr, &pf->dhdr, s);
	}
	if (write(pf->fd, &pf->dhdr, s) != s)
		goto err_out;

	if (lseek(pf->fd, pf->rtbl->string_pool_start, SEEK_SET) == -1)
		goto err_out;
	s = pf->stringpool.data_size;
	if (pf->scipher) {
		k_sc_set_key(pf->scipher, spool_nonce,
			pf->key, pf->hdr.keysize);
		k_sc_update(pf->scipher, pf->stringpool.base,
			pf->stringpool.base, s);
	}
	if (write(pf->fd, pf->stringpool.base, s) != s)
		goto err_out;

	k_hash_reset(pf->hash);
	k_hash_update(pf->hash, pf->rtbl, sz_rtbl_digest);
	k_hash_final(pf->hash, pf->rtbl->digest);
	for (size_t i = 0; i < pf->rtbl->entries; ++i) {
		k_hash_reset(pf->hash);
		k_hash_update(pf->hash, &pf->rtbl->table[i],
			sz_rtblentry_digest);
		k_hash_final(pf->hash, pf->rtbl->table[i].digest);
	}

	if (lseek(pf->fd, pf->cur_rtbl_start, SEEK_SET) == -1)
		goto err_out;
	s = sz_res_tbl + pf->rtbl->entries*sz_res_tbl_entry;
	if (pf->scipher) {
		k_sc_set_key(pf->scipher, rtbl_nonce,
			pf->key, pf->hdr.keysize);
		k_sc_update(pf->scipher, pf->rtbl, pf->rtbl, s);
	}
	if (write(pf->fd, pf->rtbl, s) != s)
		goto err_out;

	res = k_tcommit_and_close(pf->fd);

	goto out;
err_out:
	k_trollback_and_close(pf->fd);
	res = -1;
out:
	if (pf->hash)
		k_hash_finish(pf->hash);
	if (pf->prng)
		k_prng_finish(pf->prng);
	if (pf->scipher)
		k_sc_finish(pf->scipher);
	if (pf->stringpool.alloced)
		pool_free(&pf->stringpool);
	if (pf->rtbl)
		free(pf->rtbl);
	if (pf->iobuf)
		free(pf->iobuf);
	if (pf->key)
		k_free(pf->key);
	memset(pf, 0, sizeof(struct pres_file_t));
	return res;
}

__export_function uint64_t k_pres_res_count(struct pres_file_t* pf)
{
	return pf->rtbl->entries;
}

__export_function const char* k_pres_res_name_by_id
(struct pres_file_t* pf, uint64_t id, const char** basename)
{
	struct pres_resource_table_entry_t* table = pf->rtbl->table;
	const char* res = 0;

	/* entry was marked as deleted */
	if (!table[id-1].id) {
		res = "";
		if (basename)
			*basename = "";
	} else {
		res = pool_getmem(&pf->stringpool,
			table[id-1].filename_offset);
		if (basename)
			*basename = res + table[id-1].basename_offset;
	}
	return res;
}

__export_function uint64_t k_pres_res_id_by_name
(struct pres_file_t* pf, const char* name)
{
	uint64_t i = 0, e = pf->rtbl->entries;
	struct pres_resource_table_entry_t* table = pf->rtbl->table;

	for (i = 0; i < e; ++i) {
		/* entry was marked as deleted */
		if (!table[i].id)
			continue;
		size_t fn_off = table[i].filename_offset;
		const char* fn = pool_getmem(&pf->stringpool, fn_off);
		if (!strcmp(fn, name))
			return table[i].id;
	}
	return 0;
}

__export_function uint64_t k_pres_res_id_by_uuid
(struct pres_file_t* pf, uint64_t uuid)
{
	uint64_t i = 0, e = pf->rtbl->entries;
	struct pres_resource_table_entry_t* table = pf->rtbl->table;

	for (i = 0; i < e; ++i) {
		/* entry was marked as deleted */
		if (!table[i].id)
			continue;
		if (table[i].uuid == uuid)
			return table[i].id;
	}
	return 0;
}

__export_function int k_pres_res_open
(struct pres_file_t* pf, struct pres_res_t* res, uint64_t id)
{
	struct pres_resource_table_entry_t* table = pf->rtbl->table;

	memset(res, 0, sizeof(struct pres_res_t));
	/* entry was marked as deleted */
	if (!table[id-1].id)
		return -1;

	res->uuid = table[id-1].uuid;
	res->size = table[id-1].data_size;
	res->absoff = pf->rtbl->data_pool_start+table[id-1].data_offset;
	res->fd = pf->fd;
	res->scipher = pf->scipher;
	if (res->scipher) {
		k_sc_set_key(pf->scipher, table[id-1].data_iv, pf->key,
			pf->hdr.keysize);
	}
	return 0;
}

__export_function uint64_t k_pres_res_size
(struct pres_res_t* res)
{
	return res->size;
}


__export_function uint64_t k_pres_res_uuid
(struct pres_res_t* res)
{
	return res->uuid;
}

__export_function void* k_pres_res_map
(struct pres_res_t* res, uint64_t length, uint64_t offset)
{
	if (!length) {
		length = res->size;
		offset = 0;
	}
#if 0
	pres_map(&res->map, res->fd, length, res->absoff+offset);

#else
	res->map.mem = malloc(length);
	ssize_t nread;
	size_t total = 0;
	if (lseek(res->fd, res->absoff+offset, SEEK_SET) == -1)
		return 0;
	while ((nread = read(res->fd, res->map.mem+total, length-total)) > 0) {
		total += nread;
		if (total == length)
			break;
	}
	if (nread < 0) {
		free(res->map.mem);
		res->map.mem = 0;
		return 0;
	}
#endif

	if (res->scipher)
		k_sc_update(res->scipher, res->map.mem,
			res->map.mem, length);

	return res->map.mem;
}

__export_function void k_pres_res_unmap
(struct pres_res_t* res)
{
#if 0
	pres_unmap(&res->map);
#else
	if(res->map.mem) {
		free(res->map.mem);
		res->map.mem = 0;
	}
#endif
}

__export_function int k_pres_close(struct pres_file_t* pf)
{
	if (!pf->is_open)
		return -1;
	if (pf->is_writable)
		return _commit_and_close(pf);
	else {
		close(pf->fd);
		if (pf->hash)
			k_hash_finish(pf->hash);
		if (pf->prng)
			k_prng_finish(pf->prng);
		if (pf->scipher)
			k_sc_finish(pf->scipher);
		if (pf->stringpool.alloced)
			pool_free(&pf->stringpool);
		if (pf->rtbl)
			free(pf->rtbl);
		if (pf->iobuf)
			free(pf->iobuf);
		if (pf->key)
			k_free(pf->key);
		memset(pf, 0, sizeof(struct pres_file_t));
		return 0;
	}
}

__export_function void k_pres_delete_id
(struct pres_file_t* pf, uint64_t id)
{
	struct pres_resource_table_entry_t* table = pf->rtbl->table;
	memset(&table[id-1], 0, sz_res_tbl_entry);
}

__export_function int k_pres_export_id
(struct pres_file_t* pf, uint64_t id, uint32_t keep_dir_structure)
{
	uint8_t digest_chk[PRES_MAX_DIGEST_LENGTH];
	const char* basename;
	const char* name;
	int digest_bits = pf->hdr.hashsize;
	size_t digest_bytes = (digest_bits + 7) / 8;

	name = k_pres_res_name_by_id(pf, id, &basename);
	if (!strlen(name)) {
		/* TODO: if the container isn't encrypted, we are
		 * able to restore the data between two valid table entries */
		/* entity was marked as deleted */
		fprintf(stderr, "resource %lu: '%s' ->", (long)id, name);
		fprintf(stderr, " k_pres_export_id: resource was deleted\n");
		return -1;
	}

	if (keep_dir_structure && k_tcreate_dirs(name)) {
		fprintf(stderr, "resource %lu: '%s' ->", (long)id, name);
		perror(" k_tcreate_dirs");
		return -1;
	}

	struct pres_res_t r;
	k_pres_res_open(pf, &r, id);

	int fd = k_tcreat(keep_dir_structure ? name : basename, 0400);
	if (fd == -1) {
		fprintf(stderr, "resource %lu: '%s' ->", (long)id, name);
		perror(" k_tcreat");
		return -1;
	}

	uint64_t s = k_pres_res_size(&r);
	uint64_t mmap_window = PRES_IOBUF_SIZE;
	size_t niter = s / mmap_window;
	size_t nlast = s % mmap_window;

	k_hash_reset(pf->hash);
	for (uint64_t i = 0; i < niter; ++i) {
		void* m = k_pres_res_map(&r, mmap_window,
			i*mmap_window);
		size_t total = 0;
		ssize_t nwritten;
		k_hash_update(pf->hash, m, mmap_window);
		while (total != mmap_window) {
			nwritten = write(fd, m + total,
				mmap_window - total);
			if (nwritten < 0) {
				fprintf(stderr, "resource %lu: '%s' ->",
					(long)id, name);
				perror(" write");
				k_pres_res_unmap(&r);
				k_trollback_and_close(fd);
				return -1;
			}
			total += nwritten;
		}
		k_pres_res_unmap(&r);
	}
	if (nlast) {
		void* m = k_pres_res_map(&r, nlast,
			niter*mmap_window);
		size_t total = 0;
		ssize_t nwritten;
		k_hash_update(pf->hash, m, nlast);
		while (total != nlast) {
			nwritten = write(fd, m + total, nlast - total);
			if (nwritten < 0) {
				fprintf(stderr, "resource %lu: '%s' ->",
					(long)id, name);
				perror(" write");
				k_pres_res_unmap(&r);
				k_trollback_and_close(fd);
				return -1;
			}
			total += nwritten;
		}
		k_pres_res_unmap(&r);
	}

	k_hash_final(pf->hash, digest_chk);
	if (memcmp(pf->rtbl->table[id-1].data_digest, digest_chk,
	digest_bytes)) {
		fprintf(stderr, "resource %lu: '%s' ->", (long)id, name);
		fprintf(stderr, "data digest does not match\n");
		dumphx("expected", pf->rtbl->table[id-1].data_digest,
		digest_bytes);
		dumphx("have", digest_chk, digest_bytes);
		/* TODO: allow export of damaged data
		 * when explicitly requested */
		k_trollback_and_close(fd);
		return -1;
	}

	if (k_tcommit_and_close(fd)) {
		fprintf(stderr, "resource %lu: '%s' ->", (long)id, name);
		perror(" k_tcommit_and_close");
		return -1;
	}

	return 0;
}

__export_function int k_pres_repack
(struct pres_file_t* old, const char* filename, const char* pass)
{
	struct pres_file_t new;

	struct pres_options_t o = {
		.name		= filename,
		.hashsum 	= old->hdr.hashfunction,
	};

	if (pass) {
		o.blockcipher = old->hdr.cipher;
		o.ciphermode = old->hdr.ciphermode;
		o.keysize = old->hdr.keysize;
		o.pass = pass;
	}

	if (k_pres_create(&new, &o)) {
		perror("k_pres_create");
		goto err_out;
	}

	uint64_t e = k_pres_res_count(old);
	for (uint64_t i = 1; i <= e; ++i) {
		struct pres_res_t r;
		/* skipping deleted items here */
		if (k_pres_res_open(old, &r, i))
			continue;
	}

	k_pres_add_file(&new, "libk/Makefile", 5, 0);

	if (k_pres_close(&new)) {
		perror("k_pres_close new");
		goto err_out;
	}

	goto out;
err_out:
	return -1;
out:
	return 0;
}
