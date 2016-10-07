/*
 * ktool - main.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>

#ifndef __WINNT__
#define MKDIR_MODE ,0700
#else
#include <windows.h>
#define MKDIR_MODE
#endif

#include <libk/libk.h>
#include <utils/dumphx.h>
#include <utils/xor.h>

#ifndef __WINNT__
static void __term_handler(int sig, siginfo_t* info, void* unused)
{
	printf("\n");
	exit(0);
}
#endif


#ifdef __WINNT__
static uint32_t oldicp, oldocp;
static void __cleanup(void)
{
	SetConsoleOutputCP(oldocp);
	SetConsoleCP(oldicp);
}
#endif

static void __init(void)
{
#ifndef __WINNT__
	struct sigaction sa;

	if (sigemptyset(&sa.sa_mask))
		exit(1);

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = __term_handler;

	/* SIGPIPE, SIGABRT and friends are considered as bugs. no cleanup
	 * in these cases, fix the program or replace defective
	 * hardware :) */
	if (sigaction(SIGHUP, &sa, 0))
		exit(1);
	if (sigaction(SIGINT, &sa, 0))
		exit(1);
	if (sigaction(SIGQUIT, &sa, 0))
		exit(1);
	if (sigaction(SIGTERM, &sa, 0))
		exit(1);
#else
	atexit(__cleanup);
	oldicp = GetConsoleCP();
	oldocp = GetConsoleOutputCP();
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif
}

static struct pres_file_t* _open_pres_container
(const char* filename, int writable, char** passout)
{
	struct pres_file_t* p = malloc(sizeof(struct pres_file_t));
	if (!p)
		return 0;

	char* pass = 0;
	int r = k_pres_needs_pass(filename);
	if (r < 0)
		goto err_out;

	if (r) {
		pass = k_get_pass("enter password  : ");
		if (!pass)
			goto err_out;
	}

	if (k_pres_open(p, filename, pass, writable))
		goto err_out;

	goto out;
err_out:
	if (p) {
		free(p);
		p = 0;
	}
out:
	if (pass && !passout)
		free(pass);
	else if (pass && passout)
		*passout = pass;
	return p;
}

static int _close_pres_container(struct pres_file_t* p)
{
	int res = 0;
	if (k_pres_close(p))
		res = -1;
	free(p);
	return res;
}


static struct pres_file_t _cur_pres;
static int
ft_walk(const char* path, size_t baseoff)
{
	int r;

	/* we don't need uuids in this usage case, therefore set them to 0 */
	if ((r = k_pres_add_file(&_cur_pres, path, baseoff, 0)) != 0) {
		if (r == 1)
			printf("skipped: '%s'\n", path);
		if (r == -1) {
			perror("pres_add_file");
			return -1;
		}
	}
	return 0;
}

static int import_directory
(const char* directory, const char* filename, const char* pass)
{
	struct pres_options_t o = {
		.name		= filename,
		.hashsum 	= HASHSUM_SKEIN_1024,
	};

	if (pass) {
		o.blockcipher = BLK_CIPHER_THREEFISH_1024;
		o.ciphermode = BLK_CIPHER_MODE_CTR;
		o.keysize = 1024;
		o.pass = pass;
	}

	if (k_pres_create(&_cur_pres, &o)) {
		perror("pres_create");
		return -1;
	}

	char* cwd = getcwd(0, 0);
	if (!cwd) {
		perror("getcwd");
		return -1;
	}
	if (chdir(directory)) {
		free(cwd);
		perror("chdir");
		return -1;
	}

	printf("importing '%s' ...\n", directory);
	if (k_ftw(".", ft_walk)) {
		free(cwd);
		perror("k_winftw");
		return -1;
	}

	if (k_pres_close(&_cur_pres)) {
		free(cwd);
		perror("pres_close");
		return -1;
	}
	if (chdir(cwd)) {
		free(cwd);
		perror("chdir");
		return -1;
	}
	free(cwd);
	return 0;
}

#ifndef __WINNT__ /* see main() for explanation */
static size_t get_basename_offset(const char* filename)
{
	const char* c;
	size_t b = 0, s = strlen(filename);

	if (!s)
		return 0;

	c = filename + s - 1;

	while ((*c != '/') && (*c != '\\')) {
		++b;
		--c;
		if (b == s)
			break;
	}
	b = s - b;
	return b;
}

static int add_file
(const char* container, const char* filename, const char* pass)
{
	int r, res = 0;
	struct pres_file_t* p = _open_pres_container(container, 1, 0);
	if (!p)
		goto err_out;

	size_t b = get_basename_offset(filename);
	/* we don't need uuids in this usage case, therefore set them to 0 */
	if ((r = k_pres_add_file(p, filename, b, 0)) != 0) {
		if (r == 1)
			printf("skipped: '%s'.\n", filename);
		if (r == -1) {
			printf("error adding: '%s', container corrupt.\n",
				filename);
			goto err_out;
		}
	} else
		printf("added '%s'.\n", filename);

	goto out;
err_out:
	perror("add_file");
	res = -1;
out:
	if (p)
		_close_pres_container(p);
	return res;
}
#endif

static int export_all(const char* filename, const char* dir)
{
	int res = 0;
	struct pres_file_t* p = _open_pres_container(filename, 0, 0);

	if (!p)
		goto err_out;


	mkdir(dir MKDIR_MODE);
	if (chdir(dir))
		goto err_out;

	uint64_t e = k_pres_res_count(p);
	printf("exporting %"PRIu64" items from '%s' ...\n", e, filename);

	for (uint64_t i = 1; i <= e; ++i) {
		k_pres_export_id(p, i, 1);
	}

	goto out;

err_out:
	perror("export_all");
	res = -1;
out:
	if (p)
		_close_pres_container(p);
	return res;

}

static int export_id(const char* filename, const char* dir, uint64_t id)
{
	const char* basename;
	int res = 0;
	struct pres_file_t* p = _open_pres_container(filename, 0, 0);

	if (!p)
		goto err_out;

	uint64_t e = k_pres_res_count(p);
	if (!id || (id > e)) {
		printf("invalid id: %"PRIu64"\n", id);
		goto err_out;
	}

	mkdir(dir MKDIR_MODE);
	if (chdir(dir))
		goto err_out;

	k_pres_res_name_by_id(p, id, &basename);
	printf("exporting %"PRIu64":\t'%s'\n", id, basename);

	if (k_pres_export_id(p, id, 0))
		goto err_out;

	goto out;
err_out:
	perror("export_id");
	res = -1;
out:
	if (p)
		_close_pres_container(p);
	return res;
}

static int delete_id(const char* filename, uint64_t id)
{
	int res = 0;
	struct pres_file_t* p = _open_pres_container(filename, 1, 0);
	if (!p)
		goto err_out;

	uint64_t e = k_pres_res_count(p);
	if (!id || (id > e)) {
		printf("invalid id: %"PRIu64"\n", id);
		goto err_out;
	}

	const char* name = k_pres_res_name_by_id(p, id, 0);
	if (!strlen(name)) {
		printf("%"PRIu64" is already deleted\n", id);
		goto out;
	}
	k_pres_delete_id(p, id);
	printf("deleted %"PRIu64"\n", id);

	goto out;
err_out:
	perror("delete_id");
	res = -1;
out:
	if (p)
		_close_pres_container(p);
	return res;
}

static int list_all(const char* filename)
{
	int res = 0;
	struct pres_file_t* p = _open_pres_container(filename, 0, 0);

	if (!p)
		goto err_out;

	uint64_t e = k_pres_res_count(p);
	for (uint64_t i = 1; i <= e; ++i) {
		const char* name = k_pres_res_name_by_id(p, i, 0);
		printf("%"PRIu64":\t'%s'\n", i, name);
	}

	goto out;
err_out:
	perror("list_all");
	res = -1;
out:
	if (p)
		_close_pres_container(p);
	return res;
}

static int repack(const char* filename, const char* new_file)
{
	int res = 0;
	char* pass = 0;
	struct pres_file_t* p = _open_pres_container(filename, 0, &pass);

	if (!p)
		goto err_out;

	res = k_pres_repack(p, new_file, pass);
	if (res)
		goto err_out;

	goto out;
err_out:
	perror("repack");
	res = -1;
out:
	if (pass)
		free(pass);
	if (p)
		_close_pres_container(p);
	return res;
}

static void print_help(void)
{
	k_print_version();
	fprintf(stderr, " ktool test                          " \
		"- run unittests\n");
	fprintf(stderr, " ktool perf                          " \
		"- run benchmarks\n");
	fprintf(stderr, " ktool ls    <inpres>                " \
		"- list contents of pres container\n");
	fprintf(stderr, " ktool rep   <inpres> <outpres>      " \
		"- prune deleted resources\n");
	fprintf(stderr, " ktool imp   <indir>  <outpres>      " \
		"- import directory to new pres container\n");
#ifndef __WINNT__ /* see main() for explanation */
	fprintf(stderr, " ktool add   <inpres> <infile>       " \
		"- add single file to pres container\n");
#endif
	fprintf(stderr, " ktool imps  <indir>  <outpres>      " \
		"- import directory to new encrypted pres\n");
	fprintf(stderr, " ktool exp   <inpres> <outdir>       " \
		"- export everything from pres container\n");
	fprintf(stderr, " ktool expid <inpres> <outdir> <id>  " \
		"- export specific id from pres container\n");
	fprintf(stderr, " ktool delid <inpres> <id>           " \
		"- delete specific id from pres container\n");
	fprintf(stderr, " ktool version                       " \
		"- print version information\n");
	fprintf(stderr, " ktool help                          " \
		"- print this\n");
}


void uci_block_reverse(unsigned char out[16], unsigned char in[16])
{
	out[0] = in[15];
	out[1] = in[14];
	out[2] = in[13];
	out[3] = in[12];

	out[4] = in[11];
	out[5] = in[10];
	out[6] = in[9];
	out[7] = in[8];

	out[8] = in[7];
	out[9] = in[6];
	out[10] = in[5];
	out[11] = in[4];

	out[12] = in[3];
	out[13] = in[2];
	out[14] = in[1];
	out[15] = in[0];
}

void uci_blind(uint32_t addr, unsigned char aes_block[16])
{
	const uint32_t addr_mask = addr & 0xfffffff0;
	uint32_t addr_blind[4];

	addr_blind[0] = addr_mask + 0;
	addr_blind[1] = addr_mask + 4;
	addr_blind[2] = addr_mask + 8;
	addr_blind[3] = addr_mask + 12;

	xorb_64(aes_block, aes_block, addr_blind, 16);
}

void uci_encrypt(
	const unsigned char key[16],
	uint32_t addr,
	unsigned char plain[16],
	unsigned char cipher[16]
) {
	unsigned char temp[16];
	k_bc_t* aes = k_bc_init(BLK_CIPHER_AES);
	k_bc_set_encrypt_key(aes, key, 128);

		uci_blind(addr, plain);

		uci_block_reverse(temp, plain);
		k_bc_encrypt(aes, temp, temp);
		uci_block_reverse(cipher, temp);

	k_bc_finish(aes);
}

void uci_decrypt(
	const unsigned char key[16],
	uint32_t addr,
	unsigned char cipher[16],
	unsigned char plain[16]
) {
	unsigned char temp[16];
	k_bc_t* aes = k_bc_init(BLK_CIPHER_AES);
	k_bc_set_decrypt_key(aes, key, 128);

		uci_block_reverse(temp, cipher);
		k_bc_decrypt(aes, temp, temp);
		uci_block_reverse(plain, temp);

		uci_blind(addr, plain);

	k_bc_finish(aes);
}

void uci_mac(
	const unsigned char key[16],
	unsigned char input[16],
	unsigned char truncated_mac[2]
) {
	unsigned char temp[16];
	k_bc_t* aes = k_bc_init(BLK_CIPHER_AES);
	k_bc_set_encrypt_key(aes, key, 128);

		uci_block_reverse(temp, input);
		k_bc_encrypt(aes, temp, temp);
		uci_block_reverse(input, temp);

		truncated_mac[0] = input[0];
		truncated_mac[1] = input[1];

	k_bc_finish(aes);
}

const unsigned char enc_key[] = {
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
};

const unsigned char int_key[] = {
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
};

const unsigned char uci_test_vector[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,

	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,

	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,

	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,

	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,

	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,

	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,

	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
};

void uci_calc_enc_block(const unsigned char key[16], uint32_t addr, unsigned char in[128], unsigned char out[128])
{
	for (uint32_t i = 0; i < 128; i += 16) {
		uci_encrypt(key, addr + i, in + i, out + i);
	}
}

void uci_calc_mac_block(const unsigned char key[16], unsigned char in[128], unsigned char out[16])
{
	for (uint32_t i = 0; i < 128; i += 16) {
		uci_mac(key, in + i, out + (i/8));
	}
}

static void run_uci_test(void)
{
	unsigned char mac_block[16];
	unsigned char cipher_block[128];
	unsigned char input[128];

	memcpy(input, uci_test_vector, 128);

	dumphx("test vector (p)", input, 128);
	uci_calc_mac_block(int_key, input, mac_block);
	dumphx("mac value (p)", mac_block, 16);

	memcpy(input, uci_test_vector, 128);

	uci_calc_enc_block(enc_key, 0x70000000, input, cipher_block);
	dumphx("test vector (c)", cipher_block, 128);
	uci_calc_mac_block(int_key, cipher_block, mac_block);
	dumphx("mac value (c)", mac_block, 16);

#if 0
	uci_encrypt(enc_key, 0x70000000, plain0, buf0);
	dumphx("encrypted", buf0, 16);

	uci_mac(int_key, buf0, tmac);
	dumphx("truncated mac", tmac, 2);

	uci_decrypt(enc_key, 0x70000000, buf0, buf0);
	dumphx("decrypted", buf0, 16);
#endif
}



























int main(int argc, char* argv[], char* envp[])
{
	__init();

	if (argc < 2) {
		print_help();
		return -1;
	}

	if (!strcmp(argv[1], "imp") && (argc > 3)) {
		return import_directory(argv[2], argv[3], 0);
	}
#ifndef __WINNT__
/* k_pres_add_file() expects a relative filename without backslashes,
 * the filetree-walk abstraction does this for us, whereas add_file
 * doesn't. */
	if (!strcmp(argv[1], "add") && (argc > 3)) {
		return add_file(argv[2], argv[3], 0);
	}
#endif
	if (!strcmp(argv[1], "imps") && (argc > 3)) {
		char* p = k_get_pass("enter password  : ");
		if (!p)
			return -1;
		char* p2 = k_get_pass("retype password : ");
		if (!p2) {
			free(p);
			return -1;
		}
		if (strcmp(p, p2)) {
			printf("passwords do not match\n");
			free(p);
			free(p2);
			return -1;
		}
		free(p2);
		int res = import_directory(argv[2], argv[3], p);
		free(p);
		return res;
	}
	if (!strcmp(argv[1], "exp") && (argc > 3))
		return export_all(argv[2], argv[3]);
	if (!strcmp(argv[1], "expid") && (argc > 4))
		return export_id(argv[2], argv[3], strtoull(argv[4], 0, 10));
	if (!strcmp(argv[1], "delid") && (argc > 3))
		return delete_id(argv[2], strtoull(argv[3], 0, 10));
	if (!strcmp(argv[1], "ls") && (argc > 2))
		return list_all(argv[2]);
	if (!strcmp(argv[1], "rep") && (argc > 3))
		return repack(argv[2], argv[3]);
	if (!strcmp(argv[1], "test")) {
		int failed = k_run_unittests(1);
		if (failed)
			printf("failed unittests: %u\n", failed);
		else
			printf("passed all unittests\n");
		return failed;
	}
	if (!strcmp(argv[1], "perf")) {
		k_run_benchmarks(1);
		return 0;
	}
	if (!strcmp(argv[1], "uci")) {
		run_uci_test();
		return 0;
	}
	if (!strcmp(argv[1], "version")) {
		k_print_version();
		return 0;
	}
	if (!strcmp(argv[1], "help")) {
		print_help();
		return 0;
	}
	print_help();
	return -1;
}
