/*
 * libk - unittest_desc.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _UNITTEST_DESC_H
#define _UNITTEST_DESC_H

#include "sections.h"
#include "dumphx.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

typedef int (*unittest_fn)(const char**);
#define _unittest_code_						\
	__attribute__((section(_CODE_SEGMENT "__unittestcode"))) static
#define _unittest_data_						\
	__attribute__((section(_DATA_SEGMENT "__unittestdata"), \
	_section_alignment)) static const
#define _unittest_entry_					\
	__attribute__((section(_DATA_SEGMENT "__unittests"),	\
	used, _section_alignment, externally_visible)) const

struct unittest_desc {
	const char*		description;
	const char*		filename;
	const unittest_fn	run;
} __attribute__((_section_alignment));

#define unittest(_name, _description)					\
	static int unittest_##_name(const char** details);		\
	_unittest_entry_ struct unittest_desc				\
	__unittest_##_name = {						\
		.description	= _description,				\
		.filename	= __FILE__,				\
		.run		= &unittest_##_name			\
	};								\
	_unittest_code_ int unittest_##_name(const char** details)


#endif /* _UNITTEST_DESC_H */
