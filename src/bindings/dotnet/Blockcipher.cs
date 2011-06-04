/*
 * libk - Blockcipher.cs
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
	public sealed class Blockcipher : IDisposable
	{
		#region C API Interface

		[SuppressUnmanagedCodeSecurityAttribute]
		internal static class SafeNativeMethods
		{
			/* low level api without error checking */

			/* k_bc_t* k_bc_init(enum blockcipher_e cipher); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern UIntPtr k_bc_init(BlockcipherKind cipher);

			/* void k_bc_finish(k_bc_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_bc_finish(UIntPtr context);

			/* void k_bc_set_encrypt_key(k_bc_t* c, const void* k, uint32_t bits); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_bc_set_encrypt_key(UIntPtr context, [In] byte[] key, UInt32 bits);

			/* void k_bc_set_decrypt_key(k_bc_t* c, const void* k, uint32_t bits); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_bc_set_decrypt_key(UIntPtr context, [In] byte[] key, UInt32 bits);

			/* void k_bc_set_tweak(k_bc_t* c, const void* t, uint32_t bits); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_bc_set_tweak(UIntPtr context, [In] byte[] tweak, UInt32 bits);

			/* void k_bc_encrypt(k_bc_t* c, const void* i, void* o); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_bc_encrypt(UIntPtr context, [In] byte[] input, [Out] byte[] output);

			/* void k_bc_decrypt(k_bc_t* c, const void* i, void* o); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_bc_decrypt(UIntPtr context, [In] byte[] input, [Out] byte[] output);

			/* size_t k_bc_get_blocksize(k_bc_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern UIntPtr k_bc_get_blocksize(UIntPtr context);


			/* high level api using a specific blockcipher mode */

			/* int32_t k_bcmode_set_mode(k_bc_t* c, enum bcmode_e mode, int32_t max_workers); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern int k_bcmode_set_mode(UIntPtr context, BlockcipherModeKind mode, Int32 max_workers);

			/* int32_t k_bcmode_set_key(k_bc_t* c, const void* k, uint32_t bits, enum keytype_e t); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern int k_bcmode_set_key(UIntPtr context, [In] byte[] key, UInt32 bits, KeyKind type);

			/* int32_t k_bcmode_set_tweak(k_bc_t* c, const void* t, uint32_t bits); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern int k_bcmode_set_tweak(UIntPtr context, [In] byte[] tweak, UInt32 bits);

			/* void k_bcmode_set_iv(k_bc_t* c, const void* iv); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_bcmode_set_iv(UIntPtr context, [In] byte[] iv);

			/* void k_bcmode_update(k_bc_t* c, const void* i, void* o, size_t blocks); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl)]
			internal static extern void k_bcmode_update(UIntPtr context, [In] byte[] input, [Out] byte[] output, UIntPtr blocks);
		}

		#endregion


		#region Initialization

		private UIntPtr context;
		public Blockcipher(BlockcipherKind algorithm)
		{
			if (UnmanagedError.RegisterThread() != ErrorKind.K_ESUCCESS)
				throw new Exception("unable to register libk error handler");
			if ((context = SafeNativeMethods.k_bc_init(algorithm)) == (UIntPtr)0)
				UnmanagedError.ThrowLastError();
		}

		#endregion


		#region Low Level API

		public void ModelessSetEncryptKey(byte[] key, int bits)
		{
			SafeNativeMethods.k_bc_set_encrypt_key(context, key, (uint)bits);
		}

		public void ModelessSetDecryptKey(byte[] key, int bits)
		{
			SafeNativeMethods.k_bc_set_decrypt_key(context, key, (uint)bits);
		}

		public void ModelessSetTweak(byte[] tweak, int bits)
		{
			SafeNativeMethods.k_bc_set_tweak(context, tweak, (uint)bits);
		}

		public void ModelessEncrypt(byte[] input, byte[] output)
		{
			SafeNativeMethods.k_bc_encrypt(context, input, output);
		}

		public void ModelessDecrypt(byte[] input, byte[] output)
		{
			SafeNativeMethods.k_bc_decrypt(context, input, output);
		}

		#endregion


		#region High Level API

		public void SetMode(BlockcipherModeKind mode, int max_workers)
		{
			if (SafeNativeMethods.k_bcmode_set_mode(context, mode, max_workers) != 0)
				UnmanagedError.ThrowLastError();
		}

		public void SetMode(BlockcipherModeKind mode)
		{
			SetMode(mode, 0);
		}

		public void SetKey(KeyKind type, byte[] key, int bits)
		{
			if (key == null)
				throw new ArgumentNullException();
			if (bits > int.MaxValue - 7)
				throw new ArgumentException();
			if (key.Length < ((bits + 7) / 8))
				throw new ArgumentOutOfRangeException();
			if (SafeNativeMethods.k_bcmode_set_key(context, key, (uint)bits, type) != 0)
				UnmanagedError.ThrowLastError();
		}

		public void SetKey(KeyKind type, byte[] key)
		{
			SetKey(type, key, key.Length * 8);
		}

		public void SetTweak(byte[] tweak, int bits)
		{
			if (tweak == null)
				throw new ArgumentNullException();
			if (bits > int.MaxValue - 7)
				throw new ArgumentException();
			if (tweak.Length < ((bits + 7) / 8))
				throw new ArgumentOutOfRangeException();
			if (SafeNativeMethods.k_bcmode_set_tweak(context, tweak, (uint)bits) != 0)
				UnmanagedError.ThrowLastError();
		}

		public void SetTweak(byte[] tweak)
		{
			SetTweak(tweak, tweak.Length * 8);
		}

		public void SetIV(byte[] iv)
		{
			if (iv == null)
				throw new ArgumentNullException();
			if (iv.LongLength < Blocksize)
				throw new ArgumentOutOfRangeException();
			SafeNativeMethods.k_bcmode_set_iv(context, iv);
		}

		public void Update(byte[] input, byte[] output, int blocks)
		{
			if (input == null || output == null)
				throw new ArgumentNullException();
			if ((blocks <= 0) || (input.LongLength < blocks * Blocksize) || (output.LongLength < blocks * Blocksize))
				throw new ArgumentOutOfRangeException();
			SafeNativeMethods.k_bcmode_update(context, input, output, (UIntPtr)blocks);
		}

		public void Update(byte[] input, byte[] output)
		{
			if (input == null || output == null)
				throw new ArgumentNullException();
			if (input.Length % Blocksize != 0)
				throw new ArgumentException();
			if (output.LongLength < input.LongLength)
				throw new ArgumentException();
			Update(input, output, input.Length / Blocksize);
		}

		#endregion


		#region Properties

		public int Blocksize
		{
			get
			{
				return (int)SafeNativeMethods.k_bc_get_blocksize(context);
			}
		}

		#endregion


		#region IDisposable

		public void Dispose()
		{
			SafeNativeMethods.k_bc_finish(context);
			context = (UIntPtr)0;
			GC.SuppressFinalize(this);
		}

		~Blockcipher()
		{
			if (context != (UIntPtr)0)
				Dispose();
		}

		#endregion
	}
}
