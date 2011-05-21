/*
 * sections.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#ifndef _SECTIONS_H
#define _SECTIONS_H

#include <stdint.h>

/* structure and section alignment must be the same, otherwise
 * indexing within the section won't work
 */
#define _section_alignment aligned(32)

#if defined (__GNUC__) && defined (__GNUC_MINOR__)
	#define PREREQ(maj, min)	\
		((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
	#define PREREQ(maj, min) 0
#endif

#ifdef __GNUC__
	#if PREREQ(4,5)
		#define externally_visible externally_visible
	#else
		#define externally_visible used
	#endif
#endif

#if __WINNT__
#define export_prefix __declspec(dllexport)
#else
#define export_prefix
#endif
#define __export_function export_prefix \
	__attribute__ ((visibility("default"), externally_visible))

#define section_start(_name) __start_##_name
#define section_end(_name) __stop_##_name

#if defined(__MACH__)

#include <mach-o/dyld.h>
#include <mach-o/getsect.h>

#define _CODE_SEGMENT	"__TEXT,"
#define _DATA_SEGMENT	"__DATA,"

#ifdef MH_MAGIC_64
#define getsectdatafromheader	getsectdatafromheader_64
#define sect_len_t		uint64_t
#define header_cast		const struct mach_header_64*
#else
#define sect_len_t		uint32_t
#define header_cast
#endif

#define section_prologue(_name, _item_type)					\
	static const _item_type* section_start(_name);				\
	static const _item_type* section_end(_name);				\
	__attribute__((constructor)) static void init_##_name(void)		\
	{									\
		unsigned long l;						\
		sect_len_t len;							\
		section_start(_name) = (_item_type*)				\
		getsectdata("__DATA", #_name, &l);				\
		section_end(_name) = section_start(_name) + l;			\
		if (section_start(_name))					\
			return;							\
		for (long i = 0; i < _dyld_image_count(); ++i) {		\
			const struct mach_header* hdr =				\
				_dyld_get_image_header(i);			\
			section_start(_name) = (_item_type*)			\
				getsectdatafromheader((header_cast)hdr,		\
					"__DATA", #_name, &len);		\
       			if (hdr->filetype == MH_DYLIB &&			\
       			section_start(_name)) {					\
				section_start(_name) =				\
					((void*)section_start(_name)) +		\
					_dyld_get_image_vmaddr_slide(i);	\
				section_end(_name) =				\
					section_start(_name) + len;		\
				break;						\
			}							\
		}								\
	}

#define section_items(_name, _item_type)				\
	((section_end(_name)-section_start(_name)) / sizeof(_item_type))

#else

#define _CODE_SEGMENT
#define _DATA_SEGMENT

#define section_prologue(_name, _item_type)				\
	extern const _item_type section_start(_name)[];			\
	extern const _item_type section_end(_name)[];

#define section_items(_name, _item_type)				\
	(((void*)section_end(_name)-(void*)section_start(_name))	\
	/ sizeof(_item_type))

#endif /* __MACH__ */

#define foreach_section_item(_item_type, _var, _section)		\
	const _item_type* _var = section_start(_section);		\
	for (size_t i = 0; i < section_items(_section, _item_type); ++i)

#endif /* _SECTIONS_H */
