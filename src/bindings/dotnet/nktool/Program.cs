/*
 * libk - Program.cs
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

using System;
using nlibk;

namespace nktool
{
	class Program
	{
		static void worker()
		{
			try {
				Console.WriteLine("Algorithm Unittests");
				Console.WriteLine("\tAES ECB        : {0}", AesTest.TestECB() ? "passed" : "failed");
				Console.WriteLine("\tAES CBC        : {0}", AesTest.TestCBC() ? "passed" : "failed");
				Console.WriteLine("\tAES CFB        : {0}", AesTest.TestCFB() ? "passed" : "failed");
				Console.WriteLine("\tAES OFB        : {0}", AesTest.TestOFB() ? "passed" : "failed");
				Console.WriteLine("\tAES CTR Block  : {0}", AesTest.TestCTR() ? "passed" : "failed");
				Console.WriteLine("\tAES CTR Stream : {0}", AesTest.TestCTRStream() ? "passed" : "failed");
				Console.WriteLine("\tARC4           : {0}", Arc4Test.Test() ? "passed" : "failed");
				Console.WriteLine("\tSKEIN256       : {0}", SkeinTest.Test256() ? "passed" : "failed");
				Console.WriteLine("\tSKEIN512       : {0}", SkeinTest.Test512() ? "passed" : "failed");
				Console.WriteLine("\tSKEIN1024      : {0}", SkeinTest.Test1024() ? "passed" : "failed");
				Console.WriteLine("\tSHA1           : {0}", Sha1Test.Test() ? "passed" : "failed");
#if true
				Console.WriteLine("Blockcipher Performance Tests");
				Console.WriteLine("\tAES128 ECB    : {0:f2} MiB/s", AesTest.Bench128EcbEnc());
				Console.WriteLine("\tAES192 ECB    : {0:f2} MiB/s", AesTest.Bench192EcbEnc());
				Console.WriteLine("\tTF256  ECB    : {0:f2} MiB/s", ThreefishTest.Bench256EcbEnc());
				Console.WriteLine("\tTF512  ECB    : {0:f2} MiB/s", ThreefishTest.Bench512EcbEnc());
				Console.WriteLine("\tTF1024 ECB    : {0:f2} MiB/s", ThreefishTest.Bench1024EcbEnc());

				Console.WriteLine("Streamcipher Performance Tests");
				Console.WriteLine("\tARC4          : {0:f2} MiB/s", Arc4Test.Bench());
				Console.WriteLine("\tAES128 CTR    : {0:f2} MiB/s", AesTest.Bench128Ctr());
				Console.WriteLine("\tAES192 CTR    : {0:f2} MiB/s", AesTest.Bench192Ctr());
				Console.WriteLine("\tTF256  CTR    : {0:f2} MiB/s", ThreefishTest.BenchTf256Ctr());
				Console.WriteLine("\tTF512  CTR    : {0:f2} MiB/s", ThreefishTest.BenchTf512Ctr());
				Console.WriteLine("\tTF1024 CTR    : {0:f2} MiB/s", ThreefishTest.BenchTf1024Ctr());

				Console.WriteLine("Hashsum Performance Tests");
				Console.WriteLine("\tSKEIN256      : {0:f2} MiB/s", SkeinTest.Bench256());
				Console.WriteLine("\tSKEIN512      : {0:f2} MiB/s", SkeinTest.Bench512());
				Console.WriteLine("\tSKEIN1024     : {0:f2} MiB/s", SkeinTest.Bench1024());
				Console.WriteLine("\tSHA1          : {0:f2} MiB/s", Sha1Test.Bench());

				Console.WriteLine("Pseudorandomnumber Generator Performance Tests");
				Console.WriteLine("\tMT19937-32    : {0:f2} MiB/s", Mt19937_32Test.Bench());
#endif
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
