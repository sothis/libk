#ifndef _MEM_H
#define _MEM_H

#include <stddef.h>
#include <stdint.h>

size_t k_maxlocked_mem(void);

void* k_malloc(size_t size);
void* k_calloc(size_t num, size_t size);
void k_free(void* mem);

void* k_locked_malloc(size_t size, size_t* allocated);
void* k_locked_calloc(size_t num, size_t size, size_t* allocated);
void k_locked_free(void* mem, size_t l);

void k_memset(void* mem, uint8_t set, size_t len);

#endif /* _MEM_H */
