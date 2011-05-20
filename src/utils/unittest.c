#include "unittest_desc.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

section_prologue(__unittests, struct unittest_desc);

__export_function int run_unittests
(int verbose)
{
	const struct unittest_desc* tests = section_start(__unittests);
	size_t n = section_items(__unittests, struct unittest_desc);
	size_t i = 0;
	int failed = 0;

	if (verbose)
		printf("found %lu unittests\n", (unsigned long)n);
	for (i = 0; i < n; ++i) {
		const char* details = 0;
		if (verbose) {
			fprintf(stderr, "%s ...\t", tests[i].description);
			fflush(stderr);
		}
		if (!tests[i].run(&details)) {
			if (verbose)
				fprintf(stderr, "succeeded\n");
		} else {
			failed++;
			if (verbose) {
				fprintf(stderr, "failed\n");
				fprintf(stderr, "see '%s'\n",
					tests[i].filename);
			}
		}
		if (verbose) {
			fflush(stderr);
			if (details)
				fprintf(stderr, "\t(%s)\n", details);
		}
	}
	return failed;
}
