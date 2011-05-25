#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

int getln(char** lineptr, size_t* n, FILE* stream)
{
	int nchars_avail;
	char* read_pos;
	int ret;

	if (!lineptr || !n || !stream) {
		errno = EINVAL;
		return -1;
	}

	if (!*lineptr) {
		*n = 64;
		*lineptr = malloc(*n);
		if (!*lineptr) {
			errno = ENOMEM;
			return -1;
		}
		*lineptr[0] = '\0';
	}

	nchars_avail = *n;
	read_pos = *lineptr;

	while(1) {
		int save_errno;
		int c;
		c = getc(stream);
		save_errno = errno;
		if (nchars_avail < 2) {
			if (*n > 64)
				*n *= 2;
			else
				*n += 64;

			nchars_avail = *n + *lineptr - read_pos;
			*lineptr = realloc(*lineptr, *n);
			if (!*lineptr) {
				errno = ENOMEM;
				return -1;
			}
			read_pos = *n - nchars_avail + *lineptr;
		}

		if (ferror(stream)) {
			errno = save_errno;
			return -1;
		}
		if (c == EOF) {
			if (read_pos == *lineptr)
				return -1;
			else
				break;
		}
		*read_pos++ = c;
		nchars_avail--;
		if (c == '\n')
			break;
	}
	*read_pos = '\0';
	ret = read_pos - (*lineptr);
	return ret;
}
