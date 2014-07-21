/*
 * libk - Errors.cs
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

namespace nlibk
{
	public enum ErrorKind
	{
		K_ESUCCESS		= 0,
		K_ENOMEM		= 1 << 12,
		K_ENOCIPHER		= 2 << 12,
		K_ESCHEDZERO		= 3 << 12,
		K_ESCHEDTOOLARGE	= 4 << 12,
		K_ENOKEYSIZE		= 5 << 12,
		K_EINVKEYSIZE		= 6 << 12,
		K_EINSECUREENC		= 7 << 12,
		K_ENOWORKERS		= 8 << 12,
		K_ENOKEYSETTER		= 9 << 12,
		K_ENOMODE		= 10 << 12,
		K_EINVKEYTYPE		= 11 << 12,
		K_ENOMODESET		= 12 << 12,
		K_EINVTWEAKSIZE		= 13 << 12,
		K_ENOTWEAKSET		= 14 << 12,
		K_ENOHASH		= 15 << 12,
		K_ECTXZERO		= 16 << 12,
		K_ECTXTOOLARGE		= 17 << 12,
		K_EINSECUREHASH		= 18 << 12,
		K_ENORNDDEV		= 19 << 12,
		K_EINVMODE		= 20 << 12,

		K_FIRSTERRNO		= K_ENOMEM,
		K_LASTERRNO		= K_EINVMODE
	};

	public enum ErrorLevel
	{
		K_LFATAL		= 0,
		K_LWARN
	};
}
