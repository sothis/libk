/*
 * libk - mem.c
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include "mem.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef __WINNT__
#include <sys/mman.h>
#include <sys/resource.h>
#else
#include <windows.h>
#endif

#if defined(__LINUX__)
#include <sys/prctl.h>
#endif

#include <stdio.h>

#define _vmemset(_ptr,_set,_len)					\
	do {								\
		volatile char *_vptr = (volatile char*)(_ptr);		\
		size_t _vlen = (_len);					\
		while(_vlen) { *_vptr = (_set); _vptr++; _vlen--; }	\
	} while(0)

static long _pagesize;
static size_t _max_locked_mem;


__attribute__((constructor))
static void _init_mem_properties(void)
{
#ifndef __WINNT__
	struct rlimit rlp;
#if defined(__LINUX__)
	if (prctl(PR_SET_DUMPABLE, 0, 0, 0, 0)) {
		fprintf(stderr, "unable to disable core dumps\n");
		exit(-1);
	}
#endif
	rlp.rlim_cur = 0;
	rlp.rlim_max = 0;
	if (setrlimit(RLIMIT_CORE, &rlp)) {
		fprintf(stderr, "unable to disable core dumps\n");
		exit(-1);
	}
	_pagesize = sysconf(_SC_PAGESIZE);
	if (_pagesize == -1 || !_pagesize) {
		fprintf(stderr, "unable to determine pagesize\n");
		exit(-1);
	}
#if defined(__LINUX__)
	if (sysconf(_SC_MEMLOCK_RANGE) == -1) {
		fprintf(stderr, "no memlocking available\n");
		exit(-1);
	}
#endif
	if (getrlimit(RLIMIT_MEMLOCK, &rlp)) {
		fprintf(stderr, "can't get memlock limits\n");
		exit(-1);
	}
	_max_locked_mem = rlp.rlim_cur;
#else
	size_t _min_locked_mem;

	if (!GetProcessWorkingSetSize(GetCurrentProcess(),
	&_min_locked_mem, &_max_locked_mem)) {
		fprintf(stderr, "can't get memlock limits\n");
		exit(-1);
	}
	/* typically 204800 bytes on 4k pagesized windows */
	_max_locked_mem = _min_locked_mem;

	SYSTEM_INFO si;
        GetSystemInfo(&si);
        _pagesize = si.dwPageSize;

        /* on windows there is no easy way to disable dumps on a
         * per-application basis. maybe CryptProtectMemory() with
         * CRYPTPROTECTMEMORY_SAME_PROCESS might help a bit in order to
         * protect the key schedule */
#endif
}

size_t k_maxlocked_mem(void)
{
	return _max_locked_mem;
}

void k_memset(void* mem, uint8_t set, size_t len)
{
	_vmemset(mem, set, len);
}

void* k_malloc(size_t size)
{
	void* mem;
	size_t l;

	if (!size)
		return 0;

	l = _pagesize*((size+(_pagesize-1))/_pagesize);
#ifndef __WINNT__
	int res = posix_memalign(&mem, _pagesize, l);
	if (res)
		return 0;
#else
	mem = _aligned_malloc(l, _pagesize);
#endif
	return mem;
}

void* k_locked_malloc(size_t size, size_t* allocated)
{
	void* mem = k_malloc(size);
	size_t l = _pagesize*((size+(_pagesize-1))/_pagesize);
#ifndef __WINNT__
	if (!mem)
		return 0;
	if (mlock(mem, l)) {
		k_free(mem);
		return 0;
	}
#else
	if (!VirtualLock(mem, l)) {
		k_free(mem);
		return 0;
	}
#endif
	*allocated = l;
	return mem;
}


void* k_calloc(size_t num, size_t size)
{
	void* mem;

	mem = k_malloc(num*size);
	if (!mem)
		return 0;
	k_memset(mem, 0, num*size);
	return mem;
}

void* k_locked_calloc(size_t num, size_t size, size_t* allocated)
{
	void* mem;

	mem = k_locked_malloc(num*size, allocated);
	if (!mem)
		return 0;
	k_memset(mem, 0, num*size);
	return mem;
}

void k_free(void* mem)
{
#ifndef __WINNT__
	free(mem);
#else
	_aligned_free(mem);
#endif
}

void k_locked_free(void* mem, size_t len)
{
	k_memset(mem, 0, len);
#ifndef __WINNT__
	munlock(mem, len);
#else
	VirtualUnlock(mem, len);
#endif
	k_free(mem);
}
