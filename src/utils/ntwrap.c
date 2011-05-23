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
	while(*s) {
		if (*s == 'X')
			*s = randchar();
		s++;
	}
	return open(template, O_RDWR|O_CREAT|O_EXCL, 0600);
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
	off_t len;
	struct stat st;
	uint64_t o = off;
	uint32_t l = o & 0xFFFFFFFF;
	uint32_t h = (o >> 32) & 0xFFFFFFFF;

	if (!fstat(fd, &st))
		len = st.st_size;
	else
		return MAP_FAILED;

	if ((length + off) > len)
		length = len - off;

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
