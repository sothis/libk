/*
 * libk - dumphx.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _DUMPHX_H
#define _DUMPHX_H

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

static inline void
dumphx(const char* header, const void* data, uint64_t length)
{
	printf("\n%-18.18s  | ", header);
	for(uint8_t j = 0; j < 2; ++j) {
		for (uint8_t i = 0; i < 8; ++i) {
			if (!j) {
				if (i%4)
					printf("%.2x ", i);
				else
					printf(" %.2x ", i);
			}
			else
				printf("%.1x ", i);
		}
		printf(" | ");
	}
	printf("\n--------------------|");
	printf("----------------------------|-----------------");
	for (uint64_t i = 0, a = 0; i < length; ++i) {
		if (i % 8 == 0) {
			if (i) {
				printf(" | ");
				for (uint64_t j = (i-8); j < i; ++j) {
					char c = ((const char*)data)[j];
					if (!isprint(c))
						c = '.';
					printf("%c ", c);
				}
			}
			printf(" | ");
			printf("\n0x%.16lx  | ", (unsigned long)a);
			a += 8;
		}
		if (i%4)
			printf("%.2x ",  ((const uint8_t*)data)[i]);
		else
			printf(" %.2x ",  ((const uint8_t*)data)[i]);

		if (i == (length-1)) {
			uint8_t t = length%8;
			uint8_t r = (8-t)*3;
			if (t) {
				for (uint8_t j = 0; j < r; ++j) {
						printf(" ");
				}
			}
			if ((r == 12) || ((t < 4) && t))
				printf("  | ");
			else
				printf(" | ");
			if (!t)
				t = 8;
			for (uint64_t j = (length-t); j < length; ++j) {
				char c = ((const char*)data)[j];
				if (!isprint(c))
					c = '.';
				printf("%c ", c);
			}
		}
	}
	uint8_t t = length%8;
	uint8_t r = (8-t)*2;
	if (t) {
		for (uint8_t j = 0; j < r; ++j)
			printf(" ");
	}
	printf(" |\n\n");
	return;
}

#endif /* _DUMPHX_H */
