/*
 * libk - errors.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#include <errno.h>

/* this produces error codes larger than 4095, except for zero */
#define ERR_CODE(x)	(x << 12)
/* reverse, in order to be able to use kerrno as array index */
#define ERR_IDX(x)	(x >> 12)

#if !__LIBRARY_BUILD
namespace nlibk {
public
enum ErrorKind {
#else
enum k_error_e {
#endif
	K_ESUCCESS		= ERR_CODE(0),
	K_ENOMEM		= ERR_CODE(1),
	K_ENOCIPHER		= ERR_CODE(2),
	K_ESCHEDZERO		= ERR_CODE(3),
	K_ESCHEDTOOLARGE	= ERR_CODE(4),
	K_ENOKEYSIZE		= ERR_CODE(5),
	K_EINVKEYSIZE		= ERR_CODE(6),
	K_EINSECUREENC		= ERR_CODE(7),
	K_ENOWORKERS		= ERR_CODE(8),
	K_ENOKEYSETTER		= ERR_CODE(9),
	K_ENOMODE		= ERR_CODE(10),
	K_EINVKEYTYPE		= ERR_CODE(11),
	K_ENOMODESET		= ERR_CODE(12),
	K_EINVTWEAKSIZE		= ERR_CODE(13),
	K_ENOTWEAKSET		= ERR_CODE(14),
	K_ENOHASH		= ERR_CODE(15),
	K_ECTXZERO		= ERR_CODE(16),
	K_ECTXTOOLARGE		= ERR_CODE(17),
	K_EINSECUREHASH		= ERR_CODE(18),
	K_ENORNDDEV		= ERR_CODE(19),
	K_EINVMODE		= ERR_CODE(20),

	K_FIRSTERRNO		= K_ENOMEM,
	K_LASTERRNO		= K_EINVMODE
};

#if !__LIBRARY_BUILD
public
enum ErrorLevel {
#else
enum k_errorlevel_e {
#endif
	K_LFATAL		= 0,
	K_LWARN
};

#if !__LIBRARY_BUILD
} /* namespace nlibk */
#endif
