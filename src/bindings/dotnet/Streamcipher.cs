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
			/* k_sc_t* k_sc_init(enum streamcipher_e cipher, uint32_t noncebits); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern UIntPtr k_sc_init(StreamcipherKind cipher, UInt32 noncebits);

			/* k_sc_t* k_sc_init_with_blockcipher(enum blockcipher_e cipher, enum bcmode_e mode, size_t max_workers); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern UIntPtr k_sc_init_with_blockcipher(BlockcipherKind cipher, BlockcipherModeKind mode, UIntPtr max_workers);

			/* void k_sc_finish(k_sc_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_sc_finish(UIntPtr context);

			/* int32_t k_sc_set_key(k_sc_t* c, const void* nonce, const void* key, uint32_t keybits); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern int k_sc_set_key(UIntPtr context, [In] byte[] nonce, [In] byte[] key, UInt32 keybits);

			/* void k_sc_update(k_sc_t* c, const void* input, void* output, size_t bytes); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_sc_update(UIntPtr context, [In] byte[] input, [Out] byte[] output, UIntPtr bytes);

			/* size_t k_sc_get_nonce_bytes(k_sc_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern UIntPtr k_sc_get_nonce_bytes(UIntPtr context);
		}

		#endregion


		#region Initialization

		private UIntPtr context;
		public Streamcipher(StreamcipherKind algorithm, int noncebits)
		{
			if (UnmanagedError.RegisterThread() != ErrorKind.K_ESUCCESS)
				throw new Exception("unable to register libk error handler");
			if ((context = SafeNativeMethods.k_sc_init(algorithm, (uint)noncebits)) == (UIntPtr)0)
				UnmanagedError.ThrowLastError();
		}

		public Streamcipher(BlockcipherKind algorithm, BlockcipherModeKind mode, int max_workers)
		{
			if (UnmanagedError.RegisterThread() != ErrorKind.K_ESUCCESS)
				throw new Exception("unable to register libk error handler");
			if ((context = SafeNativeMethods.k_sc_init_with_blockcipher(algorithm, mode, (UIntPtr)max_workers)) == (UIntPtr)0)
				UnmanagedError.ThrowLastError();
		}

		public Streamcipher(BlockcipherKind algorithm, BlockcipherModeKind mode) :
			this(algorithm, mode, 0)
		{}

		#endregion

		public void SetKey(byte[] nonce, byte[] key, int keybits)
		{
			if (nonce == null)
				throw new ArgumentNullException();
			if ((key != null) && (keybits > int.MaxValue - 7))
				throw new ArgumentException();
			if ((key != null) && (key.Length < ((keybits + 7) / 8)))
				throw new ArgumentOutOfRangeException();
			if (SafeNativeMethods.k_sc_set_key(context, nonce, key, (uint)keybits) != 0)
				UnmanagedError.ThrowLastError();
		}

		public void SetKey(byte[] nonce, byte[] key)
		{
			SetKey(nonce, key, (key == null) ? 0 : key.Length * 8);
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

		#region Properties

		public long Noncesize
		{
			get
			{
				return (long)SafeNativeMethods.k_sc_get_nonce_bytes(context);
			}
		}

		#endregion


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
