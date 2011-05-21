/*
 * Sha1Test.cs
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

using nlibk;

namespace nlibktest
{
	public static class Sha1Test
	{
		private static readonly HashsumTestVector[] sha1vectors = new HashsumTestVector[] {
			new HashsumTestVector {
				Hashsum		=	HashKind.SHA1,
				OutputBits	=	160,
				Digest		=	"da39a3ee5e6b4b0d3255bfef95601890" +
							"afd80709",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.SHA1,
				OutputBits	=	160,
				Message		=	"ec29561244ede706b6eb30a1c371d744" +
							"50a105c3f9735f7fa9fe38cf67f304a5" +
							"736a106e92e17139a6813b1c81a4f3d3" +
							"fb9546ab4296fa9f722826c066869eda" +
							"cd73b2548035185813e22634a9da4400" +
							"0d95a281ff9f264ecce0a931222162d0" +
							"21cca28db5f3c2aa24945ab1e31cb413" +
							"ae29810fd794cad5dfaf29ec43cb38d1" +
							"98fe4ae1da2359780221405bd6712a53" +
							"05da4b1b737fce7cd21c0eb7728d0823" +
							"5a9011",
				Digest		=	"970111c4e77bcc88cc20459c02b69b4a" +
							"a8f58217",
			},
		};

		public static bool Test()
		{
			bool res = true;
			for (uint i = 0; i < sha1vectors.Length; ++i) {
				res = res && sha1vectors[i].Test();
			}
			return res;
		}

		public static double Bench()
		{
			return new TestHashThroughput {
				Hashsum = HashKind.SHA1,
				DigestBits = 160,
				megabytes = 128
			}.Run();
		}
	}
}
