using nlibk;

namespace nlibktest
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
				keytype = KeyKind.Encrypt,
				megabytes = 128
			}.Run();
		}
	}
}
