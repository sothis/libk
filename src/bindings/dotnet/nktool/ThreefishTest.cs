/*
 * libk - ThreefishTest.cs
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

using nlibk;

namespace nktool
{
	public static class ThreefishTest
	{
		public static double BenchTf256Ctr()
		{
			return new TestBlockStreamcipherThroughput {
				cipherkind = BlockcipherKind.Threefish256,
				streammode = BlockcipherStreamModeKind.Counter,
				keybits = 256,
				megabytes = 128
			}.Run();
		}

		public static double BenchTf512Ctr()
		{
			return new TestBlockStreamcipherThroughput {
				cipherkind = BlockcipherKind.Threefish512,
				streammode = BlockcipherStreamModeKind.Counter,
				keybits = 512,
				megabytes = 128
			}.Run();
		}

		public static double BenchTf1024Ctr()
		{
			return new TestBlockStreamcipherThroughput {
				cipherkind = BlockcipherKind.Threefish1024,
				streammode = BlockcipherStreamModeKind.Counter,
				keybits = 1024,
				megabytes = 128
			}.Run();
		}

		public static double Bench256EcbEnc()
		{
			return new TestBlockcipherThroughput {
				cipherkind = BlockcipherKind.Threefish256,
				ciphermode = BlockcipherModeKind.ElectronicCodeBook,
				keybits = 256,
				keytype = KeyKind.Encrypt,
				megabytes = 128
			}.Run();
		}

		public static double Bench512EcbEnc()
		{
			return new TestBlockcipherThroughput {
				cipherkind = BlockcipherKind.Threefish512,
				ciphermode = BlockcipherModeKind.ElectronicCodeBook,
				keybits = 512,
				keytype = KeyKind.Encrypt,
				megabytes = 128
			}.Run();
		}

		public static double Bench1024EcbEnc()
		{
			return new TestBlockcipherThroughput {
				cipherkind = BlockcipherKind.Threefish1024,
				ciphermode = BlockcipherModeKind.ElectronicCodeBook,
				keybits = 1024,
				keytype = KeyKind.Encrypt,
				megabytes = 128
			}.Run();
		}
	}
}
