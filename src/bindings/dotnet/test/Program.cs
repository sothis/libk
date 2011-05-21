/*
 * Program.cs
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

using System;
using nlibk;

namespace nlibktest
{
	class Program
	{
		static double BenchTf256CtrEnc()
		{
			return new TestBlockcipherThroughput {
				cipherkind = BlockcipherKind.Threefish256,
				ciphermode = BlockcipherModeKind.Counter,
				keybits = 256,
				keytype = KeyKind.Encrypt,
				megabytes = 128
			}.Run();
		}

		static double BenchTf512CtrEnc()
		{
			return new TestBlockcipherThroughput {
				cipherkind = BlockcipherKind.Threefish512,
				ciphermode = BlockcipherModeKind.Counter,
				keybits = 512,
				keytype = KeyKind.Encrypt,
				megabytes = 128
			}.Run();
		}

		static double BenchTf1024CtrEnc()
		{
			return new TestBlockcipherThroughput {
				cipherkind = BlockcipherKind.Threefish1024,
				ciphermode = BlockcipherModeKind.Counter,
				keybits = 1024,
				keytype = KeyKind.Encrypt,
				megabytes = 128
			}.Run();
		}

		static void worker()
		{
			try {
#if true
				Console.WriteLine("AES ECB        : {0}", AesTest.TestECB() ? "passed" : "failed");
				Console.WriteLine("AES CBC        : {0}", AesTest.TestCBC() ? "passed" : "failed");
				Console.WriteLine("AES CFB        : {0}", AesTest.TestCFB() ? "passed" : "failed");
				Console.WriteLine("AES OFB        : {0}", AesTest.TestOFB() ? "passed" : "failed");
				Console.WriteLine("AES CTR        : {0}", AesTest.TestCTR() ? "passed" : "failed");
				Console.WriteLine("AES CTR Stream : {0}", AesTest.TestCTRStream() ? "passed" : "failed");
				Console.WriteLine("ARC4           : {0}", Arc4Test.Test() ? "passed" : "failed");
				Console.WriteLine("SKEIN256       : {0}", SkeinTest.Test256() ? "passed" : "failed");
				Console.WriteLine("SKEIN512       : {0}", SkeinTest.Test512() ? "passed" : "failed");
				Console.WriteLine("SKEIN1024      : {0}", SkeinTest.Test1024() ? "passed" : "failed");
				Console.WriteLine("SHA1           : {0}", Sha1Test.Test() ? "passed" : "failed");
				Console.WriteLine("");
				Console.WriteLine("AES128 CTR    : {0:f2} MiB/s", AesTest.Bench128CtrEnc());
				Console.WriteLine("AES192 CTR    : {0:f2} MiB/s", AesTest.Bench192CtrEnc());
				Console.WriteLine("TF256 CTR     : {0:f2} MiB/s", BenchTf256CtrEnc());
				Console.WriteLine("TF512 CTR     : {0:f2} MiB/s", BenchTf512CtrEnc());
				Console.WriteLine("TF1024 CTR    : {0:f2} MiB/s", BenchTf1024CtrEnc());
				Console.WriteLine("");
				Console.WriteLine("SKEIN256      : {0:f2} MiB/s", SkeinTest.Bench256());
				Console.WriteLine("SKEIN512      : {0:f2} MiB/s", SkeinTest.Bench512());
				Console.WriteLine("SKEIN1024     : {0:f2} MiB/s", SkeinTest.Bench1024());
				Console.WriteLine("SHA1          : {0:f2} MiB/s", Sha1Test.Bench());
				Console.WriteLine("ARC4          : {0:f2} MiB/s", Arc4Test.Bench());
#endif
				Console.WriteLine("MT19937-32    : {0:f2} MiB/s", Mt19937_32Test.Bench());
			} catch (Exception e) {
				Console.WriteLine("An exception occured : {0}", e.Message);
			}
		}

		static void Main(string[] args)
		{
			worker();
#if false
			using (var mt = new Prng(PrngKind.MersenneTwister19937_32)) {
				byte[] seed = new byte[] {
					0x01, 0x23, 0x02, 0x34, 0x03, 0x45, 0x04, 0x56
				};
				mt.SetSeed(seed);
				for (int i = 0; i < 64; ++i)
					Console.WriteLine("{0}", (uint)mt.GetInt());
			}
#endif
		}
	}
}
