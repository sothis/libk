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
#include "ntwrap.h"
#endif

#include "tfile.h"

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
	void* data = mmap(0, len + diff, PROT_READ, MAP_PRIVATE, fd, map_off);
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

__export_function int k_pres_create(struct pres_file_t* pf, const char* name)
{
	memset(pf, 0, sizeof(struct pres_file_t));

#ifdef NDEBUG
	pf->fd = tcreat(name, 0400);
#else
	pf->fd = tcreat(name, 0600);
#endif
	if (pf->fd == -1)
		return -1;

	pf->hdr.magic = PRES_MAGIC;
	pf->hdr.version = PRES_VER;
	pf->hdr.detached_header_size = sz_detached_hdr;
	pf->hdr.detached_header_start = sz_file_header;

	pf->dhdr.resource_table_size = sz_res_tbl;

	pf->rtbl = calloc(1, sz_res_tbl);
	if (!pf->rtbl) {
		trollback_and_close(pf->fd);
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

__export_function int k_pres_add_file(struct pres_file_t* pf, const char* name)
{
	k_hash_t* skein512 = 0;
	uint8_t buf[32768];
	size_t filebytes = 0;
	size_t namelen = strlen(name)+1;

	skein512 = k_hash_init(HASHSUM_SKEIN_512, 512);
	if (!skein512)
		goto unrecoverable_err;

	if (pf->is_corrupt) {
		errno = EINVAL;
		goto err;
	}

	int fd = open(name, O_RDONLY | O_NOATIME);
	if (fd == -1) {
		k_hash_finish(skein512);
		return 1;
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

	ssize_t nread;
	while ((nread = read(fd, buf, 32768)) > 0) {
		ssize_t nwritten, total = 0;
		k_hash_update(skein512, buf, nread);
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
		k_hash_finish(skein512);
		return 1;
	}
	if(close(fd))
		goto unrecoverable_err;

	if (fsync(pf->fd))
		goto unrecoverable_err;

	pf->cur_resentries++;

	k_hash_final(skein512,
		pf->rtbl->table[pf->cur_resentries-1].data_digest);

	k_hash_reset(skein512);
	k_hash_update(skein512, name, namelen);
	k_hash_final(skein512,
		pf->rtbl->table[pf->cur_resentries-1].filename_digest);

	pf->rtbl->table[pf->cur_resentries-1].id = pf->cur_resentries;
	pf->rtbl->table[pf->cur_resentries-1].data_size = filebytes;
	pf->rtbl->table[pf->cur_resentries-1].data_offset =
		pf->cur_datapoolsize;

	pf->rtbl->table[pf->cur_resentries-1].filename_size = namelen;
	pf->rtbl->table[pf->cur_resentries-1].filename_offset =
		pf->cur_stringpoolsize;
	pf->cur_stringpoolsize += namelen;

	pf->cur_datapoolsize += filebytes;
	pf->cur_stringpoolstart = pf->cur_datapoolstart + pf->cur_datapoolsize;
	pf->cur_rtbl_start = pf->cur_stringpoolstart + pf->cur_stringpoolsize;

	pf->cur_filesize = pf->cur_rtbl_start + pf->dhdr.resource_table_size +
		pf->cur_resentries*sz_res_tbl_entry;


	k_hash_finish(skein512);
	return 0;
unrecoverable_err:
	pf->is_corrupt = 1;
err:
	if (skein512)
		k_hash_finish(skein512);
	return -1;
}

__export_function int k_pres_commit_and_close(struct pres_file_t* pf)
{
	size_t s;
	k_hash_t* skein512 = 0;
	int res = 0;

	if (pf->is_corrupt) {
		errno = EINVAL;
		goto err_out;
	}

	skein512 = k_hash_init(HASHSUM_SKEIN_512, 512);
	if (!skein512)
		goto err_out;

	pf->hdr.filesize = pf->cur_filesize;
	pf->dhdr.resource_table_start = pf->cur_rtbl_start;
	pf->rtbl->entries = pf->cur_resentries;
	pf->rtbl->data_pool_start = pf->cur_datapoolstart;
	pf->rtbl->data_pool_size = pf->cur_datapoolsize;
	pf->rtbl->string_pool_start = pf->cur_stringpoolstart;
	pf->rtbl->string_pool_size = pf->cur_stringpoolsize;

	k_hash_update(skein512, &pf->hdr, sz_header_digest);
	k_hash_final(skein512, pf->hdr.digest);
	if (lseek(pf->fd, 0, SEEK_SET) == -1)
		goto err_out;
	s = sz_file_header;
	if (write(pf->fd, &pf->hdr, s) != s)
		goto err_out;

	k_hash_reset(skein512);
	k_hash_update(skein512, &pf->dhdr, sz_dheader_digest);
	k_hash_final(skein512, pf->dhdr.digest);
	if (lseek(pf->fd, pf->hdr.detached_header_start, SEEK_SET) == -1)
		goto err_out;
	s = sz_detached_hdr;
	if (write(pf->fd, &pf->dhdr, s) != s)
		goto err_out;

	if (lseek(pf->fd, pf->rtbl->string_pool_start, SEEK_SET) == -1)
		goto err_out;
	s = pf->stringpool.data_size;
	if (write(pf->fd, pf->stringpool.base, s) != s)
		goto err_out;

	k_hash_reset(skein512);
	k_hash_update(skein512, pf->rtbl, sz_rtbl_digest);
	k_hash_final(skein512, pf->rtbl->digest);
	for (size_t i = 0; i < pf->rtbl->entries; ++i) {
		k_hash_reset(skein512);
		k_hash_update(skein512, &pf->rtbl->table[i],
			sz_rtblentry_digest);
		k_hash_final(skein512, pf->rtbl->table[i].digest);
	}

	if (lseek(pf->fd, pf->dhdr.resource_table_start, SEEK_SET) == -1)
		goto err_out;
	s = sz_res_tbl + pf->rtbl->entries*sz_res_tbl_entry;
	if (write(pf->fd, pf->rtbl, s) != s)
		goto err_out;

	res = tcommit_and_close(pf->fd);

	goto out;
err_out:
	trollback_and_close(pf->fd);
	res = -1;
out:
	if (skein512)
		k_hash_finish(skein512);
	pool_free(&pf->stringpool);
	free(pf->rtbl);
	return res;
}

/* TODO: add verifications, like filesize, offsets, filemagic, etc. */
__export_function int k_pres_open(struct pres_file_t* pf, const char* name)
{
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

	struct mmap_t map;
	pres_map(&map, pf->fd, sz_file_header, 0);
	memcpy(&pf->hdr, map.mem, sz_file_header);
	pres_unmap(&map);

	pres_map(&map, pf->fd, pf->hdr.detached_header_size,
		pf->hdr.detached_header_start);
	memcpy(&pf->dhdr, map.mem, pf->hdr.detached_header_size);
	pres_unmap(&map);

	pres_map(&map, pf->fd, pf->dhdr.resource_table_size,
		pf->dhdr.resource_table_start);
	struct pres_resource_table_t* rtbl = map.mem;
	uint64_t entries = rtbl->entries;
	pres_unmap(&map);

	pres_map(&map, pf->fd,
		pf->dhdr.resource_table_size+entries*sz_res_tbl_entry,
		pf->dhdr.resource_table_start);
	rtbl = map.mem;
	pf->rtbl = calloc(1,
		pf->dhdr.resource_table_size+entries*sz_res_tbl_entry);
	memcpy(pf->rtbl, rtbl,
		pf->dhdr.resource_table_size+entries*sz_res_tbl_entry);
	pres_unmap(&map);

	pool_alloc(&pf->stringpool, 0);
	pres_map(&map, pf->fd, pf->rtbl->string_pool_size,
		pf->rtbl->string_pool_start);
	pool_append(&pf->stringpool, map.mem, pf->rtbl->string_pool_size);
	pres_unmap(&map);

	return 0;
}

__export_function int k_pres_list(struct pres_file_t* pf)
{
	uint64_t i = 0, e = pf->rtbl->entries;
	struct pres_resource_table_entry_t* table = pf->rtbl->table;

	printf("resources: %lu\n", (unsigned long)e);
	for (i = 0; i < e; ++i) {
		size_t fn_off = table[i].filename_offset;
		const char* fn = pool_getmem(&pf->stringpool, fn_off);
		printf("\t%lu: %s\t\t\t%lu bytes\n",
			(unsigned long)table[i].id, fn,
			(unsigned long)table[i].data_size);
	}
	return 0;
}

__export_function int k_pres_close(struct pres_file_t* pf)
{
	close(pf->fd);
	pool_free(&pf->stringpool);
	free(pf->rtbl);

	return 0;
}
