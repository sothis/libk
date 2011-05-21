/*
 * Hash.cs
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

using System;
using System.Runtime.InteropServices;
using System.Security;

namespace nlibk
{
	public sealed class Hash : IDisposable
	{
		#region C API Interface

		[SuppressUnmanagedCodeSecurityAttribute]
		internal static class SafeNativeMethods
		{
			/* k_hash_t* k_hash_init(enum hashsum_e hashsum, uint32_t output_bits); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern UIntPtr k_hash_init(HashKind hashsum, uint output_bits);

			/* void k_hash_finish(k_hash_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern void k_hash_finish(UIntPtr context);

			/* void k_hash_reset(k_bc_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern void k_hash_reset(UIntPtr context);

			/* void k_hash_update(k_hash_t* c, const void* input, size_t bytes); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern void k_hash_update(UIntPtr context, [In] byte[] input, UIntPtr bytes);

			/* void k_hash_final(k_hash_t* c, void* output); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern void k_hash_final(UIntPtr context, [Out] byte[] output);
		}

		#endregion


		#region Initialization

		private UIntPtr context;
		public Hash(HashKind hashsum, int output_bits)
		{
			if (UnmanagedError.RegisterThread() != ErrorKind.K_ESUCCESS)
				throw new Exception("unable to register libk error handler");
			if ((context = SafeNativeMethods.k_hash_init(hashsum, (uint)output_bits)) == (UIntPtr)0)
				UnmanagedError.ThrowLastError();
		}

		#endregion


		#region Public API

		public void Reset()
		{
			SafeNativeMethods.k_hash_reset(context);
		}

		public void Update(byte[] message, int bytes)
		{
			SafeNativeMethods.k_hash_update(context, message, (UIntPtr)bytes);
		}

		public void Final(byte[] digest)
		{
			SafeNativeMethods.k_hash_final(context, digest);
		}

		#endregion


		#region IDisposable

		public void Dispose()
		{
			SafeNativeMethods.k_hash_finish(context);
			context = (UIntPtr)0;
			GC.SuppressFinalize(this);
		}

		~Hash()
		{
			if (context != (UIntPtr)0)
				Dispose();
		}

		#endregion
	}
}
