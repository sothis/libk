/*
 * libk - ntwrap.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include "ntwrap.h"
#include <stdint.h>
#include <sys/stat.h>
#include <windows.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

long sysconf(int name)
{
	if (name == _SC_PAGE_SIZE) {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return si.dwAllocationGranularity;
	}
	return -1;
}

static int seed = 0;
static char randchar(void)
{
	int s;
	char r[3];
	if (!seed)
		seed = time(0);
	else
		seed += 1337;
	srand(seed);
	r[0] = rand() % 10 + 48;
	r[1] = rand() % 26 + 65;
	r[2] = rand() % 26 + 97;
	s = rand() % 3;
	return r[s];
}

int mkstemp(char* template)
{
	/* NOTE: this is quick and dirty, don't rely on it. this implementation
	 * is for development purposes only */

	char* s = template;
	if (!s)
		return -1;
	size_t l = strlen(template);
	if (!l)
		return -1;

	while (l--) {
		char* last = s+l;
		if (*last == 'X')
			*last = randchar();
		else
			break;
	}

	wchar_t* wc = utf8_to_ucs2(template);
	if (!wc)
		return -1;
	return _wopen(wc, O_RDWR|O_CREAT|O_EXCL|_O_BINARY, 0600);
}

int fchmod(int fd, mode_t mode)
{
	return 0;
}

int fsync(int fd)
{
	return 0;
}

/* this implementation only supports MAP_PRIVATE with PROT_READ|PROT_WRITE
 * at the moment */
void* mmap(void* start, size_t length, int prot, int flags, int fd, off_t off)
{
	HANDLE hmap;
	void* temp;
	uint64_t o = off;
	uint32_t l = o & 0xffffffff;
	uint32_t h = (o >> 32) & 0xffffffff;

	hmap = CreateFileMapping((HANDLE)_get_osfhandle(fd), 0, PAGE_WRITECOPY,
		0, 0, 0);

	if (!hmap)
		return MAP_FAILED;

	temp = MapViewOfFileEx(hmap, FILE_MAP_COPY, h, l, length, start);

	if (!CloseHandle(hmap))
		return MAP_FAILED;

	return temp ? temp : MAP_FAILED;
}

int munmap(void* start, size_t length)
{
	if (!UnmapViewOfFile(start))
		return -1;
	return 0;
}

static int32_t _get_ucs2_length(const char* utf8_str)
{
	int32_t res;
	res = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_str, -1,
		0, 0);
	return res;
}

static int32_t _get_utf8_length(const wchar_t* ucs2_str)
{
	int32_t res;
	res = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, ucs2_str, -1,
		0, 0, 0, 0);
	return res;
}

wchar_t* utf8_to_ucs2(const char* utf8_str)
{
	int32_t s = _get_ucs2_length(utf8_str);
	if (!s)
		return 0;
	wchar_t* wcstr = calloc(s, sizeof(wchar_t));
	if (!wcstr)
		return 0;
	if (s != MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_str,
	-1, wcstr, s)) {
		free(wcstr);
		return 0;
	}
	return wcstr;
}

char* ucs2_to_utf8(const wchar_t* ucs2_str)
{
	int32_t s = _get_utf8_length(ucs2_str);
	if (!s)
		return 0;
	char* mbstr = calloc(s, sizeof(char));
	if (!mbstr)
		return 0;
	if (s != WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, ucs2_str,
	-1, mbstr, s, 0, 0)) {
		free(mbstr);
		return 0;
	}
	return mbstr;
}
