/*
 * libk - errors.h
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

#if !__LIBRARY_BUILD
namespace nlibk {
public
enum ErrorKind {
#else
enum k_error_e {
#endif
	K_ESUCCESS		= 0,
	K_ENOMEM,
	K_ENOCIPHER,
	K_ESCHEDZERO,
	K_ESCHEDTOOLARGE,
	K_ENOKEYSIZE,
	K_EINVKEYSIZE,
	K_EINSECUREENC,
	K_ENOWORKERS,
	K_ENOKEYSETTER,
	K_ENOMODE,
	K_EINVKEYTYPE,
	K_ENOMODESET,
	K_EINVTWEAKSIZE,
	K_ENOTWEAKSET,
	K_ENOHASH,
	K_ECTXZERO,
	K_ECTXTOOLARGE,
	K_EINSECUREHASH,
	K_ENORNDDEV
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
