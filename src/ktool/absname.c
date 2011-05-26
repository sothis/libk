#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* only remove '.', '..' and multiple '/' compontents, keep symlinks */
char* absfilename(const char* name)
{
	size_t wds, ns;
	char* p = 0;
	char* cwd = 0;
	char* last;

	ns = strlen(name);
	if (!ns)
		goto err_out;

	cwd = getcwd(0, 0);
	if (!cwd)
		goto err_out;
	wds = strlen(cwd);
	if (!wds)
		goto err_out;

	p = calloc(wds+ns+3, sizeof(char*));
	if (!p)
		goto err_out;

	if (name[0] == '/') {
		memcpy(p, name, ns);
		last = p+ns-1;
	} else {
		memcpy(p, cwd, wds);
		p[wds] = '/';
		memcpy(p+wds+1, name, ns);
		last = p+ns+wds;
	}

	char* f = p;
	char* t = p;
	while(*f) {
		char* ls;
		char n = *f++; *t++ = n;

		if ((n == '/') && (t > p+1)) {
			char* e = &t[-2];
			switch (*e) {
			case '/':
				t--;
				break;
			case '.':
				ls = &e[-1];
				if (t > p+2) {
					if (*ls == '.') {
						if ((t > p+4) &&
						(*--ls == '/')) {
							while ((ls > p) &&
							!(*--ls == '/'));
							t = ls+1;
						}
					} else if (*ls == '/')
						t = e;
				}
				break;
			default:
				break;
			}
		}
	}
	if ((t[-1] == '/') && (t > p+1))
		t--;
	t[0] = 0;

	free(cwd);
	return p;
err_out:
	if (p)
		free(p);
	if (cwd)
		free(cwd);
	return 0;

}
