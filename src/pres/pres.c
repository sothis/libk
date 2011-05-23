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

#include "utils/tfile.h"

#if defined(__DARWIN__) || defined(__WINNT__)
#define O_NOATIME		0
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

/* TODO: cleanup error handling here */
__export_function int k_pres_create
(struct pres_file_t* pf, struct pres_options_t* opt)
{
	memset(pf, 0, sizeof(struct pres_file_t));

	if (!opt->hashsum) {
		errno = EINVAL;
		return -1;
	}
	pf->hash = k_hash_init(opt->hashsum, opt->hashsize);
	if (!pf->hash)
		return -1;

#ifdef NDEBUG
	pf->fd = tcreat(opt->name, 0400);
#else
	pf->fd = tcreat(opt->name, 0600);
#endif
	if (pf->fd == -1) {
		k_hash_finish(pf->hash);
		return -1;
	}

	pf->hdr.magic = PRES_MAGIC;
	pf->hdr.version = PRES_VER;
	pf->hdr.hashfunction = opt->hashsum;
	pf->hdr.hashsize = k_hash_digest_size(pf->hash);
	if (opt->streamcipher)
		pf->hdr.cipher = opt->streamcipher;
	else {
		pf->hdr.cipher = opt->blockcipher;
		pf->hdr.ciphermode = opt->ciphermode;
	}
	pf->hdr.keysize = opt->keysize;
	pf->hdr.tweaksize = opt->tweaksize;
	pf->hdr.detached_header_size = sz_detached_hdr;
	pf->hdr.detached_header_start = sz_file_header;

	pf->dhdr.resource_table_size = sz_res_tbl;

	pf->rtbl = calloc(1, sz_res_tbl);
	if (!pf->rtbl) {
		trollback_and_close(pf->fd);
		k_hash_finish(pf->hash);
		return -1;
	}

	pf->cur_rtbl_start = sz_detached_hdr + sz_file_header;
	pf->cur_datapoolstart = sz_detached_hdr + sz_file_header;
	pf->cur_stringpoolstart = sz_detached_hdr + sz_file_header;
	pf->cur_filesize = sz_file_header + sz_detached_hdr + sz_res_tbl;

	pool_alloc(&pf->stringpool, 0);

	/* this creates a hole for the file header, i.e. a sparse file on
	 * filesystems which supports it */
	if (lseek(pf->fd, pf->cur_datapoolstart, SEEK_SET) == -1) {
		trollback_and_close(pf->fd);
		free(pf->rtbl);
		k_hash_finish(pf->hash);
		return -1;
	}

	if (opt->blockcipher && opt->ciphermode) {
		pf->scipher = k_sc_init_with_blockcipher(opt->blockcipher,
			opt->ciphermode, 0);
	} else if (opt->streamcipher) {
		pf->scipher = k_sc_init(opt->streamcipher);
	}

	if (pf->hdr.cipher && !opt->key) {
		trollback_and_close(pf->fd);
		free(pf->rtbl);
		k_hash_finish(pf->hash);
		return -1;
	}
	if (pf->hdr.cipher && !pf->scipher) {
		trollback_and_close(pf->fd);
		free(pf->rtbl);
		k_hash_finish(pf->hash);
		return -1;
	}
	if (pf->hdr.cipher &&
	k_sc_set_key(pf->scipher, opt->key, opt->keysize)) {
		trollback_and_close(pf->fd);
		free(pf->rtbl);
		k_hash_finish(pf->hash);
		return -1;
	}

	return 0;
}

static int pres_rollback(struct pres_file_t* pf, size_t ssize)
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
	pf->rtbl = realloc(pf->rtbl,
		sz_res_tbl + (pf->cur_resentries)*sz_res_tbl_entry);
	pool_detach(&pf->stringpool, ssize);
	return 0;
}

__export_function int k_pres_add_file
(struct pres_file_t* pf, const char* name, size_t basename_off)
{
	uint8_t buf[32768];
	size_t filebytes = 0;
	uint8_t data_nonce[128];

	/* TODO: fix constant nonce size */
	k_prng_t* prng = k_prng_init(PRNG_PLATFORM);
	k_prng_update(prng, data_nonce, 128);
	k_prng_finish(prng);

	if (pf->is_corrupt) {
		errno = EINVAL;
		goto unrecoverable_err;
	}

	int fd = open(name, O_RDONLY | O_NOATIME);
	if (fd == -1) {
		return 1;
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
	size_t namelen = strlen(name)+1;

	/* not adding hidden objects */
	n = name;
	if (!memcmp(n, ".", 1)) {
		close(fd);
		return 1;
	}
	while (*n) {
		if (!memcmp(n, "/.", 2)) {
			close(fd);
			return 1;
		}
		n++;
	}

	if (pool_append(&pf->stringpool, name, namelen)) {
		goto unrecoverable_err;
	}

	void* temp = realloc(pf->rtbl,
		sz_res_tbl + (pf->cur_resentries+1)*sz_res_tbl_entry);
	if (!temp)
		goto unrecoverable_err;

	pf->rtbl = temp;
	memset(&pf->rtbl->table[pf->cur_resentries], 0, sz_res_tbl_entry);

	k_hash_reset(pf->hash);
	if (pf->hdr.cipher)
		k_sc_set_nonce(pf->scipher, data_nonce);
	ssize_t nread;
	while ((nread = read(fd, buf, 32768)) > 0) {
		ssize_t nwritten, total = 0;
		k_hash_update(pf->hash, buf, nread);
		if (pf->hdr.cipher)
			k_sc_update(pf->scipher, buf, buf, nread);
		while (total != nread) {
			nwritten = write(pf->fd, buf + total, nread - total);
			if (nwritten < 0) {
				close(fd);
				goto unrecoverable_err;
			}
			total += nwritten;
		}
		filebytes += total;
	}
	if (nread < 0) {
		close(fd);
		if (pres_rollback(pf, namelen))
			goto unrecoverable_err;
		return 1;
	}
	if(close(fd))
		goto unrecoverable_err;

	if (fsync(pf->fd))
		goto unrecoverable_err;

	pf->cur_resentries++;

	k_hash_final(pf->hash,
		pf->rtbl->table[pf->cur_resentries-1].data_digest);

	k_hash_reset(pf->hash);
	k_hash_update(pf->hash, name, namelen);
	k_hash_final(pf->hash,
		pf->rtbl->table[pf->cur_resentries-1].filename_digest);

	pf->rtbl->table[pf->cur_resentries-1].id = pf->cur_resentries;
	pf->rtbl->table[pf->cur_resentries-1].data_size = filebytes;
	pf->rtbl->table[pf->cur_resentries-1].data_offset =
		pf->cur_datapoolsize;

	pf->rtbl->table[pf->cur_resentries-1].filename_size = namelen;
	pf->rtbl->table[pf->cur_resentries-1].filename_offset =
		pf->cur_stringpoolsize;
	pf->rtbl->table[pf->cur_resentries-1].basename_offset =
		basename_off;
	pf->cur_stringpoolsize += namelen;

	pf->cur_datapoolsize += filebytes;
	pf->cur_stringpoolstart = pf->cur_datapoolstart + pf->cur_datapoolsize;
	pf->cur_rtbl_start = pf->cur_stringpoolstart + pf->cur_stringpoolsize;

	pf->cur_filesize = pf->cur_rtbl_start + pf->dhdr.resource_table_size +
		pf->cur_resentries*sz_res_tbl_entry;

	/* TODO: fix constant nonce size */
	if (pf->hdr.cipher)
		memcpy(pf->rtbl->table[pf->cur_resentries-1].data_iv,
			data_nonce, 128);

	return 0;
unrecoverable_err:
	pf->is_corrupt = 1;
	return -1;
}

__export_function int k_pres_commit_and_close(struct pres_file_t* pf)
{
	uint8_t dhdr_nonce[128];
	uint8_t rtbl_nonce[128];
	uint8_t spool_nonce[128];
	size_t s;
	int res = 0;

	if (pf->is_corrupt) {
		errno = EINVAL;
		goto err_out;
	}

	/* TODO: fix constant nonce size */
	k_prng_t* prng = k_prng_init(PRNG_PLATFORM);
	k_prng_update(prng, dhdr_nonce, 128);
	k_prng_update(prng, rtbl_nonce, 128);
	k_prng_update(prng, spool_nonce, 128);
	k_prng_finish(prng);

	pf->hdr.filesize = pf->cur_filesize;
	pf->dhdr.resource_table_start = pf->cur_rtbl_start;
	pf->rtbl->entries = pf->cur_resentries;
	pf->rtbl->data_pool_start = pf->cur_datapoolstart;
	pf->rtbl->data_pool_size = pf->cur_datapoolsize;
	pf->rtbl->string_pool_start = pf->cur_stringpoolstart;
	pf->rtbl->string_pool_size = pf->cur_stringpoolsize;

	/* TODO: fix constant nonce size */
	if (pf->hdr.cipher) {
		memcpy(pf->hdr.detached_header_iv, dhdr_nonce, 128);
		memcpy(pf->dhdr.resource_table_iv, rtbl_nonce, 128);
		memcpy(pf->rtbl->string_pool_iv, spool_nonce, 128);
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
	if (pf->hdr.cipher) {
		k_sc_set_nonce(pf->scipher, dhdr_nonce);
		k_sc_update(pf->scipher, &pf->dhdr, &pf->dhdr, s);
	}
	if (write(pf->fd, &pf->dhdr, s) != s)
		goto err_out;

	if (lseek(pf->fd, pf->rtbl->string_pool_start, SEEK_SET) == -1)
		goto err_out;
	s = pf->stringpool.data_size;
	if (pf->hdr.cipher) {
		k_sc_set_nonce(pf->scipher, spool_nonce);
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
	if (pf->hdr.cipher) {
		k_sc_set_nonce(pf->scipher, rtbl_nonce);
		k_sc_update(pf->scipher, pf->rtbl, pf->rtbl, s);
	}
	if (write(pf->fd, pf->rtbl, s) != s)
		goto err_out;

	res = tcommit_and_close(pf->fd);

	goto out;
err_out:
	trollback_and_close(pf->fd);
	res = -1;
out:
	k_hash_finish(pf->hash);
	k_sc_finish(pf->scipher);
	pool_free(&pf->stringpool);
	free(pf->rtbl);
	return res;
}

/* TODO:
 * - check offset+size against filesize, don't forget overflow checking
 * - if some digest checks fail, it might be still possible to access a valid
 *   resource. only if the entry in the resource table is corrupt we consider
 *   the resource as unaccessible
 * - cleanup error handling
 * */
__export_function int k_pres_open
(struct pres_file_t* pf, const char* name, const void* key)
{
	uint8_t digest_chk[PRES_MAX_DIGEST_LENGTH];

	memset(pf, 0, sizeof(struct pres_file_t));
	pf->fd = open(name, O_RDONLY);
	if (pf->fd == -1)
		return -1;

	struct stat st;
	if (fstat(pf->fd, &st)) {
		close(pf->fd);
		return -1;
	}

	if (!S_ISREG(st.st_mode)) {
		close(pf->fd);
		errno = EINVAL;
		return -1;
	}

	if (st.st_size < sz_file_header) {
		close(pf->fd);
		errno = EINVAL;
		return -1;
	}

	struct mmap_t map;
	pres_map(&map, pf->fd, sz_file_header, 0);
	memcpy(&pf->hdr, map.mem, sz_file_header);
	pres_unmap(&map);
	if (pf->hdr.magic != PRES_MAGIC) {
		close(pf->fd);
		errno = EINVAL;
		return -1;
	}
	/* currently no back and forward compatibility */
	if (pf->hdr.version != PRES_VER) {
		close(pf->fd);
		errno = EINVAL;
		return -1;
	}
	if (pf->hdr.filesize != st.st_size) {
		close(pf->fd);
		errno = EINVAL;
		return -1;
	}
	if (!pf->hdr.hashfunction ||
	pf->hdr.hashfunction > HASHSUM_MAX_SUPPORT) {
		close(pf->fd);
		errno = EINVAL;
		return -1;
	}
	if (!pf->hdr.hashsize ||
	pf->hdr.hashsize > PRES_MAX_DIGEST_LENGTH*8) {
		close(pf->fd);
		errno = EINVAL;
		return -1;
	}

	pf->hash = k_hash_init(pf->hdr.hashfunction, pf->hdr.hashsize);
	if (!pf->hash) {
		close(pf->fd);
		return -1;
	}

	k_hash_update(pf->hash, &pf->hdr, sz_header_digest);
	k_hash_final(pf->hash, digest_chk);
	if (memcmp(pf->hdr.digest, digest_chk, (pf->hdr.hashsize + 7) / 8)) {
		k_hash_finish(pf->hash);
		close(pf->fd);
		return -1;
	}

	/* following entities might be encrypted */
	if (pf->hdr.cipher) {
		if (pf->hdr.ciphermode)
			pf->scipher = k_sc_init_with_blockcipher(pf->hdr.cipher,
				pf->hdr.ciphermode, 0);
		else
			pf->scipher = k_sc_init(pf->hdr.cipher);

		if (!pf->scipher ||
		k_sc_set_key(pf->scipher, key, pf->hdr.keysize)) {
			k_hash_finish(pf->hash);
			close(pf->fd);
			if (pf->scipher)
				k_sc_finish(pf->scipher);
			return -1;
		}
	}

	pres_map(&map, pf->fd, sz_detached_hdr, sz_file_header);
	memcpy(&pf->dhdr, map.mem, sz_detached_hdr);
	pres_unmap(&map);

	if (pf->hdr.cipher) {
		k_sc_set_nonce(pf->scipher, pf->hdr.detached_header_iv);
		k_sc_update(pf->scipher, &pf->dhdr, &pf->dhdr, sz_detached_hdr);
	}

	k_hash_reset(pf->hash);
	k_hash_update(pf->hash, &pf->dhdr, sz_dheader_digest);
	k_hash_final(pf->hash, digest_chk);
	if (memcmp(pf->dhdr.digest, digest_chk, (pf->hdr.hashsize + 7) / 8)) {
		k_hash_finish(pf->hash);
		close(pf->fd);
		if (pf->scipher)
			k_sc_finish(pf->scipher);
		return -1;
	}

	pf->rtbl = calloc(1, sz_res_tbl);
	pres_map(&map, pf->fd, sz_res_tbl, pf->dhdr.resource_table_start);
	memcpy(pf->rtbl, map.mem, sz_res_tbl);
	pres_unmap(&map);

	if (pf->hdr.cipher) {
		k_sc_set_nonce(pf->scipher, pf->dhdr.resource_table_iv);
		k_sc_update(pf->scipher, pf->rtbl, pf->rtbl, sz_res_tbl);
	}

	uint64_t entries = pf->rtbl->entries;
	free(pf->rtbl);

	pres_map(&map, pf->fd,
		pf->dhdr.resource_table_size+entries*sz_res_tbl_entry,
		pf->dhdr.resource_table_start);
	pf->rtbl = calloc(1,
		pf->dhdr.resource_table_size+entries*sz_res_tbl_entry);
	memcpy(pf->rtbl, map.mem,
		pf->dhdr.resource_table_size+entries*sz_res_tbl_entry);
	pres_unmap(&map);

	if (pf->hdr.cipher) {
		k_sc_set_nonce(pf->scipher, pf->dhdr.resource_table_iv);
		k_sc_update(pf->scipher, pf->rtbl, pf->rtbl,
			pf->dhdr.resource_table_size+entries*sz_res_tbl_entry);
	}


	k_hash_reset(pf->hash);
	k_hash_update(pf->hash, pf->rtbl, sz_rtbl_digest);
	k_hash_final(pf->hash, digest_chk);
	if (memcmp(pf->rtbl->digest, digest_chk, (pf->hdr.hashsize + 7) / 8)) {
		k_hash_finish(pf->hash);
		free(pf->rtbl);
		close(pf->fd);
		if (pf->scipher)
			k_sc_finish(pf->scipher);
		return -1;
	}

	pool_alloc(&pf->stringpool, 0);
	pres_map(&map, pf->fd, pf->rtbl->string_pool_size,
		pf->rtbl->string_pool_start);
	pool_append(&pf->stringpool, map.mem, pf->rtbl->string_pool_size);
	pres_unmap(&map);

	if (pf->hdr.cipher) {
		k_sc_set_nonce(pf->scipher, pf->rtbl->string_pool_iv);
		k_sc_update(pf->scipher, pf->stringpool.base,
			pf->stringpool.base, pf->stringpool.data_size);
	}

	uint64_t i = 0, e = pf->rtbl->entries;
	struct pres_resource_table_entry_t* table = pf->rtbl->table;
	for (i = 0; i < e; ++i) {
		size_t fn_off = table[i].filename_offset;
		const char* fn = pool_getmem(&pf->stringpool, fn_off);
		size_t fns = strlen(fn)+1;

		k_hash_reset(pf->hash);
		k_hash_update(pf->hash, &table[i], sz_rtblentry_digest);
		k_hash_final(pf->hash, digest_chk);
		if (memcmp(table[i].digest, digest_chk,
		(pf->hdr.hashsize + 7) / 8)) {
			k_hash_finish(pf->hash);
			free(pf->rtbl);
			pool_free(&pf->stringpool);
			close(pf->fd);
			if (pf->scipher)
				k_sc_finish(pf->scipher);
			return -1;
		}

		k_hash_reset(pf->hash);
		k_hash_update(pf->hash, fn, fns);
		k_hash_final(pf->hash, digest_chk);
		if (memcmp(table[i].filename_digest, digest_chk,
		(pf->hdr.hashsize + 7) / 8)) {
			k_hash_finish(pf->hash);
			free(pf->rtbl);
			pool_free(&pf->stringpool);
			close(pf->fd);
			if (pf->scipher)
				k_sc_finish(pf->scipher);
			return -1;
		}
	}

	return 0;
}


__export_function uint64_t k_pres_res_count(struct pres_file_t* pf)
{
	return pf->rtbl->entries;
}

__export_function const char* k_pres_res_name_by_id
(struct pres_file_t* pf, uint64_t id)
{
	struct pres_resource_table_entry_t* table = pf->rtbl->table;
	return pool_getmem(&pf->stringpool, table[id-1].filename_offset);
}

__export_function const char* k_pres_res_basename_by_id
(struct pres_file_t* pf, uint64_t id)
{
	struct pres_resource_table_entry_t* table = pf->rtbl->table;
	return pool_getmem(&pf->stringpool,
		table[id-1].filename_offset + table[id-1].basename_offset);
}

__export_function uint64_t k_pres_res_id_by_name
(struct pres_file_t* pf, const char* name)
{
	uint64_t i = 0, e = pf->rtbl->entries;
	struct pres_resource_table_entry_t* table = pf->rtbl->table;

	for (i = 0; i < e; ++i) {
		size_t fn_off = table[i].filename_offset;
		const char* fn = pool_getmem(&pf->stringpool, fn_off);
		if (!strcmp(fn, name))
			return table[i].id;
	}
	return 0;
}

__export_function void k_pres_res_by_id
(struct pres_file_t* pf, struct pres_res_t* res, uint64_t id)
{
	struct pres_resource_table_entry_t* table = pf->rtbl->table;

	memset(&res->map, 0, sizeof(struct mmap_t));
	res->size = table[id-1].data_size;
	res->absoff = pf->rtbl->data_pool_start+table[id-1].data_offset;
	res->fd = pf->fd;
	res->scipher = pf->scipher;
	if (res->scipher)
		k_sc_set_nonce(res->scipher, table[id-1].data_iv);
}

__export_function uint64_t k_pres_res_size
(struct pres_res_t* res)
{
	return res->size;
}

__export_function void* k_pres_res_map
(struct pres_res_t* res, uint64_t length, uint64_t offset)
{
	if (!length) {
		length = res->size;
		offset = 0;
	}
	pres_map(&res->map, res->fd, length, res->absoff+offset);

	if (res->scipher)
		k_sc_update(res->scipher, res->map.mem,
			res->map.mem, length);

	return res->map.mem;
}

__export_function void k_pres_res_unmap
(struct pres_res_t* res)
{
	pres_unmap(&res->map);
}


__export_function int k_pres_close(struct pres_file_t* pf)
{
	if (pf->scipher)
		k_sc_finish(pf->scipher);
	close(pf->fd);
	pool_free(&pf->stringpool);
	free(pf->rtbl);
	k_hash_finish(pf->hash);
	return 0;
}
