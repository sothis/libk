using System;
using System.Runtime.InteropServices;
using System.Security;

namespace nlibk
{
	public sealed class Prng : IDisposable
	{
		#region C API Interface

		[SuppressUnmanagedCodeSecurityAttribute]
		internal static class SafeNativeMethods
		{
			/* k_sc_t* k_prng_init(enum prng_e prng); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern UIntPtr k_prng_init(PrngKind cipher);

			/* void k_prng_finish(k_sc_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern void k_prng_finish(UIntPtr context);

			/* void k_prng_set_seed(k_prng_t* c, const void* seed, size_t seed_bytes); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern int k_prng_set_seed(UIntPtr context, [In] byte[] seed, UIntPtr bytes);

			/* void k_prng_update(k_prng_t* c, void* output, size_t bytes); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern void k_prng_update(UIntPtr context, [Out] byte[] output, UIntPtr bytes);

			/* uint8_t k_prng_get_uint8(k_prng_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern byte k_prng_get_uint8(UIntPtr context);

			/* uint16_t k_prng_get_uint16(k_prng_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern ushort k_prng_get_uint16(UIntPtr context);

			/* uint32_t k_prng_get_uint32(k_prng_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern uint k_prng_get_uint32(UIntPtr context);

			/* uint64_t k_prng_get_uint64(k_prng_t* c); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern ulong k_prng_get_uint64(UIntPtr context);
		}

		#endregion


		#region Initialization

		private UIntPtr context;
		public Prng(PrngKind algorithm)
		{
			if (UnmanagedError.RegisterThread() != ErrorKind.K_ESUCCESS)
				throw new Exception("unable to register libk error handler");
			if ((context = SafeNativeMethods.k_prng_init(algorithm)) == (UIntPtr)0)
				UnmanagedError.ThrowLastError();
		}

		#endregion

		public void SetSeed(byte[] seed, int bytes)
		{
			if (seed == null)
				throw new ArgumentNullException();
			if (seed.Length < bytes)
				throw new ArgumentOutOfRangeException();
			SafeNativeMethods.k_prng_set_seed(context, seed, (UIntPtr)bytes);
		}

		public void SetSeed(byte[] seed)
		{
			SetSeed(seed, seed.Length);
		}

		public void Update(byte[] output, int bytes)
		{
			if (output == null)
				throw new ArgumentNullException();
			if ((bytes <= 0) || (output.LongLength < bytes))
				throw new ArgumentOutOfRangeException();
			SafeNativeMethods.k_prng_update(context, output, (UIntPtr)bytes);
		}

		public void Update(byte[] output)
		{
			Update(output, output.Length);
		}

		public byte GetByte()
		{
			return SafeNativeMethods.k_prng_get_uint8(context);
		}
		public short GetShort()
		{
			return (short)SafeNativeMethods.k_prng_get_uint16(context);
		}
		public int GetInt()
		{
			return (int)SafeNativeMethods.k_prng_get_uint32(context);
		}
		public long GetLong()
		{
			return (long)SafeNativeMethods.k_prng_get_uint64(context);
		}

		#region IDisposable

		public void Dispose()
		{
			SafeNativeMethods.k_prng_finish(context);
			context = (UIntPtr)0;
			GC.SuppressFinalize(this);
		}

		~Prng()
		{
			if (context != (UIntPtr)0)
				Dispose();
		}

		#endregion
	}
}
