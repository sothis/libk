/*
 * SkeinTest.cs
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
	public static class SkeinTest
	{
		private static readonly HashsumTestVector[] skein256vectors = new HashsumTestVector[] {
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	256,
				Digest		=	"c8877087da56e072870daa843f176e94" +
							"53115929094c3a40c463a196c29bf7ba",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	256,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0",
				Digest		=	"53403b16a293104a517bcccdd136ff71" +
							"f584f7ffb057a849133af3d25002a01d",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	256,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0",
				Digest		=	"8d0fa4ef777fd759dfd4044e6f6a5ac3" +
							"c774aec943dcfc07927b723b5dbf408b",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	256,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5",
				Digest		=	"291d9233c6d54d823e042db198e58916" +
							"9e6c24382a3025b060f5dcc5ac5b53b6",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	256,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5d4d3d2d1d0" +
							"cfcecdcccbcac9c8c7c6c5c4c3c2c1c0",
				Digest		=	"df28e916630d0b44c4a849dc9a02f07a" +
							"07cb30f732318256b15d865ac4ae162f",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	128,
				Digest		=	"07e8ff2191c5052e1a25914c7c213078",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	128,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0",
				Digest		=	"c1692aae24735a7dadb6f7d9474eb92c",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	128,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0",
				Digest		=	"3322ab027d0fb1d6651d57078e660c2b",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	128,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5",
				Digest		=	"cb5ed1cd28da74df35a3caf08b24cce8",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	128,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5d4d3d2d1d0" +
							"cfcecdcccbcac9c8c7c6c5c4c3c2c1c0",
				Digest		=	"ea7f07a4a793706bc8a57b8d22373b8f",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	173,
				Digest		=	"cc516b5ddfc3709c88007ee52e77fa64" +
							"395aae38b233",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	173,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0",
				Digest		=	"f30d89dd81a7ceadcfedf5d887e79bd6" +
							"1911677af37a",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	173,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0",
				Digest		=	"f7a462a143106980014c44f4f1d62b89" +
							"7d932a1c59db",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	173,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5",
				Digest		=	"58b167da89cab3b2c1b240d198065b4e" +
							"ae1587cf9e2f",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	173,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5d4d3d2d1d0" +
							"cfcecdcccbcac9c8c7c6c5c4c3c2c1c0",
				Digest		=	"6dd98921aceaf81151665d73be77a0bc" +
							"1759c4eb8152",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	384,
				Digest		=	"44d8b126c5dcaf022028fd9c4ea41afd" +
							"545d7b32adf06e42cccbab8b83f99bb1" +
							"1391e4354ed5b5d51d4976075557025f",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	384,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0",
				Digest		=	"e4a3f336e1391d86a999b7cee405d1c1" +
							"0afd5fbadaf9370c362e540e2c2057a1" +
							"42ec6983ac73439dec88e5853f590381",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	384,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0",
				Digest		=	"5b669ce9840fc06f43d2e046d13ef126" +
							"c5a3e26e070bd670ba1aef062547253f" +
							"e0a55322da920a20ce2d86875d7c9095",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	384,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5",
				Digest		=	"65c4b9d13ba86ee700c7cc99ee9b0e3d" +
							"51bd3a6adb35607e2423d64d4ee6bc18" +
							"5c750ac2f06cad3a1b92a1329a123e5f",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein256,
				OutputBits	=	384,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5d4d3d2d1d0" +
							"cfcecdcccbcac9c8c7c6c5c4c3c2c1c0",
				Digest		=	"04e1e347e02e7f70df8cb5dfd3d5130c" +
							"8794c481ea571c4fa3cacb0ce7dc3818" +
							"af0ba29cf8a95687e01afbdc46f59ed2",
			}
		};

		private static readonly HashsumTestVector[] skein512vectors = new HashsumTestVector[] {
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein512,
				OutputBits	=	512,
				Digest		=	"bc5b4c50925519c290cc634277ae3d62" +
							"57212395cba733bbad37a4af0fa06af4" +
							"1fca7903d06564fea7a2d3730dbdb80c" +
							"1f85562dfcc070334ea4d1d9e72cba7a",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein512,
				OutputBits	=	512,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0",
				Digest		=	"8eaf7afc9e85a23f4e46ba4c55130664" +
							"09a41779b471ae84fac5f5c0d6648040" +
							"e19337e367adc7ab1fac2c78d379b636" +
							"9d905cd6cdfac2b0d98e6260c47193f7",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein512,
				OutputBits	=	512,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5d4d3d2d1d0" +
							"cfcecdcccbcac9c8c7c6c5c4c3c2c1c0",
				Digest		=	"45863ba3be0c4dfc27e75d358496f4ac" +
							"9a736a505d9313b42b2f5eada79fc17f" +
							"63861e947afb1d056aa199575ad3f8c9" +
							"a3cc1780b5e5fa4cae050e989876625b",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein512,
				OutputBits	=	512,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5d4d3d2d1d0" +
							"cfcecdcccbcac9c8c7c6c5c4c3c2c1c0" +
							"bfbebdbcbbbab9b8b7",
				Digest		=	"41322b2bb336a4481919817efe56bf5b" +
							"ae0be8eea44926c9aa7eadd7313ca808" +
							"21a8c670632236a34c59837f9cde8856" +
							"137d6c36c5a23c1c18e3a62374af335f",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein512,
				OutputBits	=	512,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5d4d3d2d1d0" +
							"cfcecdcccbcac9c8c7c6c5c4c3c2c1c0" +
							"bfbebdbcbbbab9b8b7b6b5b4b3b2b1b0" +
							"afaeadacabaaa9a8a7a6a5a4a3a2a1a0" +
							"9f9e9d9c9b9a99989796959493929190" +
							"8f8e8d8c8b8a89888786858483828180",
				Digest		=	"91cca510c263c4ddd010530a33073309" +
							"628631f308747e1bcbaa90e451cab92e" +
							"5188087af4188773a332303e6667a7a2" +
							"10856f742139000071f48e8ba2a5adb7",
			},
		};

		private static readonly HashsumTestVector[] skein1024vectors = new HashsumTestVector[] {
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein1024,
				OutputBits	=	1024,
				Digest		=	"0fff9563bb3279289227ac77d319b6ff" +
							"f8d7e9f09da1247b72a0a265cd6d2a62" +
							"645ad547ed8193db48cff847c06494a0" +
							"3f55666d3b47eb4c20456c9373c86297" +
							"d630d5578ebd34cb40991578f9f52b18" +
							"003efa35d3da6553ff35db91b81ab890" +
							"bec1b189b7f52cb2a783ebb7d823d725" +
							"b0b4a71f6824e88f68f982eefc6d19c6",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein1024,
				OutputBits	=	1024,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0",
				Digest		=	"d736bd0e3a4edaeff13d263cf2784a25" +
							"aab0cd6a1efe453206d728fffd93ae8d" +
							"1e0e4634ffdebe567caede2b25346f62" +
							"1a3869a40f7c68a79f2f82b637851854" +
							"4140dc2f1e5de3074de74da43538a81d" +
							"711715b2d21662332b33c94cc5f4e7a0" +
							"e9cf94d0f51d1fc33317340d2e4d2d1a" +
							"b2e75a815e6f0bee1994b7608f432e2e",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein1024,
				OutputBits	=	1024,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5d4d3d2d1d0" +
							"cfcecdcccbcac9c8c7c6c5c4c3c2c1c0" +
							"bfbebdbcbbbab9b8b7b6b5b4b3b2b1b0" +
							"afaeadacabaaa9a8a7a6a5a4a3a2a1a0" +
							"9f9e9d9c9b9a99989796959493929190" +
							"8f8e8d8c8b8a89888786858483828180",
				Digest		=	"1f3e02c46fb80a3fcd2dfbbc7c173800" +
							"b40c60c2354af551189ebf433c3d85f9" +
							"ff1803e6d920493179ed7ae7fce69c35" +
							"81a5a2f82d3e0c7a295574d0cd7d217c" +
							"484d2f6313d59a7718ead07d0729c248" +
							"51d7e7d2491b902d489194e6b7d369db" +
							"0ab7aa106f0ee0a39a42efc54f18d937" +
							"76080985f907574f995ec6a37153a578",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein1024,
				OutputBits	=	1024,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5d4d3d2d1d0" +
							"cfcecdcccbcac9c8c7c6c5c4c3c2c1c0" +
							"bfbebdbcbbbab9b8b7b6b5b4b3b2b1b0" +
							"afaeadacabaaa9a8a7a6a5a4a3a2a1a0" +
							"9f9e9d9c9b9a99989796959493929190" +
							"8f8e8d8c8b8a89888786858483828180" +
							"7f7e7d7c7b7a79787776757473727170" +
							"6f6e6d6c6b6a69686766656463626160" +
							"5f5e5d5c5b5a59585756555453",
				Digest		=	"5935d014f41b7dbe616c3e498f66b213" +
							"274c9cfe4846da8de7556d004922e6e3" +
							"ce8444eccfbe22ee0fcabc1184f3d44e" +
							"5c193867862570b6290b8fd1279d60a1" +
							"1c5fed902fc8b65d6efdd8ca1e9429a7" +
							"c3b0d41d0bf7d197d233f3659c83688d" +
							"60efa141a694e0ae308d44c71671e111" +
							"178688ad80130a8067be8544c3888b21",
			},
			new HashsumTestVector {
				Hashsum		=	HashKind.Skein1024,
				OutputBits	=	1024,
				Message		=	"fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0" +
							"efeeedecebeae9e8e7e6e5e4e3e2e1e0" +
							"dfdedddcdbdad9d8d7d6d5d4d3d2d1d0" +
							"cfcecdcccbcac9c8c7c6c5c4c3c2c1c0" +
							"bfbebdbcbbbab9b8b7b6b5b4b3b2b1b0" +
							"afaeadacabaaa9a8a7a6a5a4a3a2a1a0" +
							"9f9e9d9c9b9a99989796959493929190" +
							"8f8e8d8c8b8a89888786858483828180" +
							"7f7e7d7c7b7a79787776757473727170" +
							"6f6e6d6c6b6a69686766656463626160" +
							"5f5e5d5c5b5a59585756555453525150" +
							"4f4e4d4c4b4a49484746454443424140" +
							"3f3e3d3c3b3a39383736353433323130" +
							"2f2e2d2c2b2a29282726252423222120" +
							"1f1e1d1c1b1a19181716151413121110" +
							"0f0e0d0c0b0a09080706050403020100",
				Digest		=	"842a53c99c12b0cf80cf69491be5e2f7" +
							"515de8733b6ea9422dfd676665b5fa42" +
							"ffb3a9c48c217777950848cecdb48f64" +
							"0f81fb92bef6f88f7a85c1f7cd1446c9" +
							"161c0afe8f25ae444f40d3680081c35a" +
							"a43f640fd5fa3c3c030bcc06abac01d0" +
							"98bcc984ebd8322712921e00b1ba07d6" +
							"d01f26907050255ef2c8e24f716c52a5",
			},
		};

		public static bool Test256()
		{
			bool res = true;
			for (uint i = 0; i < skein256vectors.Length; ++i) {
				res = res && skein256vectors[i].Test();
			}
			return res;
		}

		public static bool Test512()
		{
			bool res = true;
			for (uint i = 0; i < skein512vectors.Length; ++i) {
				res = res && skein512vectors[i].Test();
			}
			return res;
		}

		public static bool Test1024()
		{
			bool res = true;
			for (uint i = 0; i < skein1024vectors.Length; ++i) {
				res = res && skein1024vectors[i].Test();
			}
			return res;
		}

		public static double Bench256()
		{
			return new TestHashThroughput {
				Hashsum = HashKind.Skein256,
				DigestBits = 256,
				megabytes = 128
			}.Run();
		}

		public static double Bench512()
		{
			return new TestHashThroughput {
				Hashsum = HashKind.Skein512,
				DigestBits = 512,
				megabytes = 128
			}.Run();
		}

		public static double Bench1024()
		{
			return new TestHashThroughput {
				Hashsum = HashKind.Skein1024,
				DigestBits = 1024,
				megabytes = 128
			}.Run();
		}
	}
}
