/*
 * libk - Streamcipher.cs
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
	public sealed class Streamcipher : IDisposable
	{
		#region C API Interface

		[SuppressUnmanagedCodeSecurityAttribute]
		internal static class SafeNativeMethods
		{
			/* k_sc_t* k_sc_init(enum streamcipher_e cipher); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern UIntPtr k_sc_init(StreamcipherKind cipher);

			/* k_sc_t* k_sc_init_with_blockcipher(enum blockcipher_e cipher, enum bcstreammode_e mode, size_t max_workers); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern UIntPtr k_sc_init_with_blockcipher(BlockcipherKind cipher, BlockcipherStreamModeKind mode, UIntPtr max_workers);

			/* void k_sc_finish(k_sc_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_sc_finish(UIntPtr context);

			/* void k_sc_set_nonce(k_sc_t* c, const void* nonce); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_sc_set_nonce(UIntPtr context, [In] byte[] nonce);

			/* int32_t k_sc_set_key(k_sc_t* c, const void* key, uint32_t keybits); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern int k_sc_set_key(UIntPtr context, [In] byte[] key, UInt32 bits);

			/* void k_sc_update(k_sc_t* c, const void* input, void* output, size_t bytes); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_sc_update(UIntPtr context, [In] byte[] input, [Out] byte[] output, UIntPtr bytes);
		}

		#endregion


		#region Initialization

		private UIntPtr context;
		public Streamcipher(StreamcipherKind algorithm)
		{
			if (UnmanagedError.RegisterThread() != ErrorKind.K_ESUCCESS)
				throw new Exception("unable to register libk error handler");
			if ((context = SafeNativeMethods.k_sc_init(algorithm)) == (UIntPtr)0)
				UnmanagedError.ThrowLastError();
		}

		public Streamcipher(BlockcipherKind algorithm, BlockcipherStreamModeKind mode, int max_workers)
		{
			if (UnmanagedError.RegisterThread() != ErrorKind.K_ESUCCESS)
				throw new Exception("unable to register libk error handler");
			if ((context = SafeNativeMethods.k_sc_init_with_blockcipher(algorithm, mode, (UIntPtr)max_workers)) == (UIntPtr)0)
				UnmanagedError.ThrowLastError();
		}

		public Streamcipher(BlockcipherKind algorithm, BlockcipherStreamModeKind mode) :
			this(algorithm, mode, 0)
		{}

		#endregion

		public void SetKey(byte[] key, int bits)
		{
			if (key == null)
				throw new ArgumentNullException();
			if (bits > int.MaxValue - 7)
				throw new ArgumentException();
			if (key.Length < ((bits + 7) / 8))
				throw new ArgumentOutOfRangeException();
			if (SafeNativeMethods.k_sc_set_key(context, key, (uint)bits) != 0)
				UnmanagedError.ThrowLastError();
		}

		public void SetKey(byte[] key)
		{
			SetKey(key, key.Length * 8);
		}

		public void SetNonce(byte[] nonce)
		{
			if (nonce == null)
				throw new ArgumentNullException();
			SafeNativeMethods.k_sc_set_nonce(context, nonce);
		}

		public void Update(byte[] input, byte[] output, int bytes)
		{
			if (output == null)
				throw new ArgumentNullException();
			if ((bytes <= 0) || (output.LongLength < bytes))
				throw new ArgumentOutOfRangeException();
			if ((input != null) && (output.LongLength < input.LongLength))
				throw new ArgumentOutOfRangeException();
			SafeNativeMethods.k_sc_update(context, input, output, (UIntPtr)bytes);
		}

		public void Update(byte[] input, byte[] output)
		{
			if (output == null)
				throw new ArgumentNullException();
			Update(input, output, output.Length);
		}


		#region IDisposable

		public void Dispose()
		{
			SafeNativeMethods.k_sc_finish(context);
			context = (UIntPtr)0;
			GC.SuppressFinalize(this);
		}

		~Streamcipher()
		{
			if (context != (UIntPtr)0)
				Dispose();
		}

		#endregion
	}
}
