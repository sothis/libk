/*
 * pres.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _PRES_H
#define _PRES_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include "mempool.h"

/* NOTE: _every_ change in the on-disk structures have to result in a
 * version change */
#define PRES_VER		0x00000001u
#define PRES_MAGIC		0x0701198123421337ull
#define PRES_MAX_DIGEST_LENGTH	1024
#define PRES_MAX_IV_LENGTH	1024
#define PRES_MAX_SIG_LENGTH	32768


/*
 * _start pointers are absolute offsets from file start
 * _offset pointers are relative offsets from the corresponding _start pointer
 * _size values are the structure sizes in bytes, string sizes include the zero
 * 	terminating byte
 * _entries are ordinal counts of fixed sized elements in a table
 */

struct pres_file_header_t {
	uint64_t	magic;
	uint32_t	version;
	/* used hashfunction and digestsize, _all_ digests are calculated
	 * with that function. NOTE: string digests are calculated including
	 * the terminating zero byte. */
//	uint32_t	hashfunction;
//	uint32_t	hashsize;
	/* if set, _all_ entities have to be signed */
//	uint32_t	pres_signed;
	/* used cipher. when not encrypted, set to 0. if set, _all_ entities
	 * have to be encrypted. TODO: think about if we sign the
	 * data before or after encryption. the latter one allows verification
	 * without decryption but may leak information about the count of
	 * resources in the container due to padding zero bytes in the
	 * signature. */
//	uint32_t	cipher;
	/* if the used cipher is a plain streamcipher, set ciphermode to 0 */
//	uint32_t	ciphermode;
//	uint32_t	keysize;
//	uint32_t	tweaksize;
	uint64_t	filesize;

	uint64_t	detached_header_size;
	uint64_t	detached_header_start;
//	uint8_t		detached_header_iv[PRES_MAX_IV_LENGTH];

	/* if pres_signed is not set, the content is irrelevant, but will be
	 * used in digest calculations */
//	uint8_t		signature[PRES_MAX_SIG_LENGTH];
	/* must contain the message digest for all data above. it's a
	 * mandatory field, the used hashfunction is identified by
	 * pres_file_header_t.hashfunction */
	uint8_t		digest[PRES_MAX_DIGEST_LENGTH];
} __attribute__((packed));

struct pres_detached_header_t {
	uint64_t	resource_table_size;
	uint64_t	resource_table_start;
//	uint8_t		resource_table_iv[PRES_MAX_IV_LENGTH];

//	uint8_t		signature[PRES_MAX_SIG_LENGTH];
	uint8_t		digest[PRES_MAX_DIGEST_LENGTH];
} __attribute__((packed));

struct pres_resource_table_entry_t {
	uint64_t	id;

	/* relative pointer into string pool */
//	uint64_t	name_size;
//	uint64_t	name_offset;
//	uint8_t		name_iv[PRES_MAX_IV_LENGTH];
//	uint8_t		name_signature[PRES_MAX_SIG_LENGTH];
//	uint8_t		name_digest[PRES_MAX_DIGEST_LENGTH];

	/* relative pointer into string pool */
	uint64_t	filename_size;
	uint64_t	filename_offset;
//	uint8_t		filename_iv[PRES_MAX_IV_LENGTH];
//	uint8_t		filename_signature[PRES_MAX_SIG_LENGTH];
	uint8_t		filename_digest[PRES_MAX_DIGEST_LENGTH];

	/* relative pointer into string pool */
//	uint64_t	description_size;
//	uint64_t	description_offset;
//	uint8_t		description_iv[PRES_MAX_IV_LENGTH];
//	uint8_t		description_signature[PRES_MAX_SIG_LENGTH];
//	uint8_t		description_digest[PRES_MAX_DIGEST_LENGTH];

	/* relative pointer into data pool */
	uint64_t	data_size;
	uint64_t	data_offset;
//	uint8_t		data_iv[PRES_MAX_IV_LENGTH];
//	uint8_t		data_signature[PRES_MAX_SIG_LENGTH];
	uint8_t		data_digest[PRES_MAX_DIGEST_LENGTH];

//	uint8_t		signature[PRES_MAX_SIG_LENGTH];
	uint8_t		digest[PRES_MAX_DIGEST_LENGTH];
} __attribute__((packed));

struct pres_resource_table_t {
	uint64_t				entries;

	uint64_t				string_pool_size;
	uint64_t				string_pool_start;

	uint64_t				data_pool_size;
	uint64_t				data_pool_start;

//	uint8_t					signature[PRES_MAX_SIG_LENGTH];
	uint8_t					digest[PRES_MAX_DIGEST_LENGTH];

	struct pres_resource_table_entry_t	table[];
} __attribute__((packed));

struct mmap_t {
	void*		mem;
	size_t		len;
	off_t		off;
};

struct pres_file_t {
	int				fd;
	int				is_corrupt;
	size_t				cur_filesize;
	size_t				cur_rtbl_start;
	size_t				cur_resentries;
	size_t				cur_datapoolstart;
	size_t				cur_datapoolsize;
	size_t				cur_stringpoolstart;
	size_t				cur_stringpoolsize;
	struct mempool_t		stringpool;
	struct pres_file_header_t	hdr;
	struct pres_detached_header_t	dhdr;
	struct pres_resource_table_t*	rtbl;
};

enum pres_structure_sizes_e {
	sz_file_header = sizeof(struct pres_file_header_t),
	sz_header_digest = sz_file_header - PRES_MAX_DIGEST_LENGTH,
	sz_detached_hdr = sizeof(struct pres_detached_header_t),
	sz_dheader_digest = sz_detached_hdr - PRES_MAX_DIGEST_LENGTH,
	sz_res_tbl = sizeof(struct pres_resource_table_t),
	sz_rtbl_digest = sz_res_tbl - PRES_MAX_DIGEST_LENGTH,
	sz_res_tbl_entry = sizeof(struct pres_resource_table_entry_t),
	sz_rtblentry_digest = sz_res_tbl_entry - PRES_MAX_DIGEST_LENGTH,
};


/* creates a new pres container. if it exists the function will succeed only
 * if we have read-write permissions. in that case the existing container
 * will be overwritten only if pres_commit_and_close() succeeds. on the other
 * hand, if it does not exist yet, it will be linked on the file system only
 * if pres_commit_and_close() succeeds too. when an unrecoverable error
 * occurs, the filesystem will be left in the state like it was before
 * pres_create() was called, except there is a bug or the process was killed
 * with SIGKILL and friends. then there might exist some stale temporary files
 * in the current working directory. in that case the existing container
 * (if any) is still in the state like it was before invoking pres_create(). */
extern int k_pres_create(struct pres_file_t* pf, const char* name);

/* adds an existing file to the resource container, might fail, but leaves
 * state untouched in that case, so that a consecutive call with another file
 * still can succeed without producing a corrupt resource container file.
 * if the pres container was marked as corrupt, pres_add_file is a noop
 * returns -1 and sets errno to EINVAL
 *
 * returns 0 if file was added successfully
 * returns 1 if there was an error while reading or opening the file,
 * 		but a consecutive call to pres_add_file is possible and safe
 * returns -1 if an unrecoverable error occured, the container might be
 * 		corrupt */
extern int k_pres_add_file(struct pres_file_t* pf, const char* name);

/* closes the container and commits changes. if an error occurs closes and
 * unlinks the temporary file. if the contatiner was marked as corrupt by
 * pres_add_file(), it closes and unlinks the temporary file too and sets errno
 * to EINVAL. in the case the container existed before it will be overwritten
 * only if this function succeeds, it's guaranteed that either the old or
 * the new container will exist on the filesystem, no state in between. */
extern int k_pres_commit_and_close(struct pres_file_t* pf);

/* opens existing container */
extern int k_pres_open(struct pres_file_t* pf, const char* name);

/* list resources in container on stdout */
extern int k_pres_list(struct pres_file_t* pf);

/* closes open container */
extern int k_pres_close(struct pres_file_t* pf);

#endif /* _PRES_H */
