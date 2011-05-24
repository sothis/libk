/*
 * libk - Arc4Test.cs
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
	public static class Arc4Test
	{
		private static readonly StreamcipherTestVector arc4Vector = new StreamcipherTestVector {
			Cipher		=	StreamcipherKind.ARC4,
			Key		=	"0123456789abcdef",
			Plaintext	=	"0123456789abcdef",
			Ciphertext	=	"75b7878099e0c596"
		};

		public static bool Test()
		{
			return arc4Vector.Test();
		}

		public static double Bench()
		{
			return new TestStreamcipherThroughput {
				cipherkind = StreamcipherKind.ARC4,
				keybits = 2048,
				megabytes = 128
			}.Run();
		}
	}
}
