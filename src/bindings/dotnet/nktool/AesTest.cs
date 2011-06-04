/*
 * libk - AesTest.cs
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
	public static class AesTest
	{
		private static readonly BlockcipherTestVector ecbVector128 = new BlockcipherTestVector {
			Cipher		=	BlockcipherKind.AES,
			Mode		=	BlockcipherModeKind.ElectronicCodeBook,
			Key		=	"2b7e151628aed2a6abf7158809cf4f3c",
			Plaintext	=	"6bc1bee22e409f96e93d7e117393172a" +
						"ae2d8a571e03ac9c9eb76fac45af8e51" +
						"30c81c46a35ce411e5fbc1191a0a52ef" +
						"f69f2445df4f9b17ad2b417be66c3710",
			Ciphertext	=	"3ad77bb40d7a3660a89ecaf32466ef97" +
						"f5d3d58503b9699de785895a96fdbaaf" +
						"43b1cd7f598ece23881b00e3ed030688" +
						"7b0c785e27e8ad3f8223207104725dd4"
		};

		private static readonly BlockcipherTestVector ecbVector192 = new BlockcipherTestVector {
			Cipher		=	BlockcipherKind.AES,
			Mode		=	BlockcipherModeKind.ElectronicCodeBook,
			Key		=	"8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b",
			Plaintext	=	"6bc1bee22e409f96e93d7e117393172a" +
						"ae2d8a571e03ac9c9eb76fac45af8e51" +
						"30c81c46a35ce411e5fbc1191a0a52ef" +
						"f69f2445df4f9b17ad2b417be66c3710",
			Ciphertext	=	"bd334f1d6e45f25ff712a214571fa5cc" +
						"974104846d0ad3ad7734ecb3ecee4eef" +
						"ef7afd2270e2e60adce0ba2face6444e" +
						"9a4b41ba738d6c72fb16691603c18e0e"
		};

		private static readonly BlockcipherTestVector cbcVector128 = new BlockcipherTestVector {
			Cipher		=	BlockcipherKind.AES,
			Mode		=	BlockcipherModeKind.CipherBlockChaining,
			Key		=	"2b7e151628aed2a6abf7158809cf4f3c",
			Iv		=	"000102030405060708090a0b0c0d0e0f",
			Plaintext	=	"6bc1bee22e409f96e93d7e117393172a" +
						"ae2d8a571e03ac9c9eb76fac45af8e51" +
						"30c81c46a35ce411e5fbc1191a0a52ef" +
						"f69f2445df4f9b17ad2b417be66c3710",
			Ciphertext	=	"7649abac8119b246cee98e9b12e9197d" +
						"5086cb9b507219ee95db113a917678b2" +
						"73bed6b8e3c1743b7116e69e22229516" +
						"3ff1caa1681fac09120eca307586e1a7"
		};

		private static readonly BlockcipherTestVector cbcVector192 = new BlockcipherTestVector {
			Cipher		=	BlockcipherKind.AES,
			Mode		=	BlockcipherModeKind.CipherBlockChaining,
			Key		=	"8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b",
			Iv		=	"000102030405060708090a0b0c0d0e0f",
			Plaintext	=	"6bc1bee22e409f96e93d7e117393172a" +
						"ae2d8a571e03ac9c9eb76fac45af8e51" +
						"30c81c46a35ce411e5fbc1191a0a52ef" +
						"f69f2445df4f9b17ad2b417be66c3710",
			Ciphertext	=	"4f021db243bc633d7178183a9fa071e8" +
						"b4d9ada9ad7dedf4e5e738763f69145a" +
						"571b242012fb7ae07fa9baac3df102e0" +
						"08b0e27988598881d920a9e64f5615cd"
		};

		private static readonly BlockcipherTestVector cfbVector128 = new BlockcipherTestVector {
			Cipher		=	BlockcipherKind.AES,
			Mode		=	BlockcipherModeKind.CipherFeedback,
			Key		=	"2b7e151628aed2a6abf7158809cf4f3c",
			Iv		=	"000102030405060708090a0b0c0d0e0f",
			Plaintext	=	"6bc1bee22e409f96e93d7e117393172a" +
						"ae2d8a571e03ac9c9eb76fac45af8e51" +
						"30c81c46a35ce411e5fbc1191a0a52ef" +
						"f69f2445df4f9b17ad2b417be66c3710",
			Ciphertext	=	"3b3fd92eb72dad20333449f8e83cfb4a" +
						"c8a64537a0b3a93fcde3cdad9f1ce58b" +
						"26751f67a3cbb140b1808cf187a4f4df" +
						"c04b05357c5d1c0eeac4c66f9ff7f2e6"
		};

		private static readonly BlockcipherTestVector cfbVector192 = new BlockcipherTestVector {
			Cipher		=	BlockcipherKind.AES,
			Mode		=	BlockcipherModeKind.CipherFeedback,
			Key		=	"8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b",
			Iv		=	"000102030405060708090a0b0c0d0e0f",
			Plaintext	=	"6bc1bee22e409f96e93d7e117393172a" +
						"ae2d8a571e03ac9c9eb76fac45af8e51" +
						"30c81c46a35ce411e5fbc1191a0a52ef" +
						"f69f2445df4f9b17ad2b417be66c3710",
			Ciphertext	=	"cdc80d6fddf18cab34c25909c99a4174" +
						"67ce7f7f81173621961a2b70171d3d7a" +
						"2e1e8a1dd59b88b1c8e60fed1efac4c9" +
						"c05f9f9ca9834fa042ae8fba584b09ff"
		};

		private static readonly BlockcipherTestVector ofbVector128 = new BlockcipherTestVector {
			Cipher		=	BlockcipherKind.AES,
			Mode		=	BlockcipherModeKind.OutputFeedback,
			Key		=	"2b7e151628aed2a6abf7158809cf4f3c",
			Iv		=	"000102030405060708090a0b0c0d0e0f",
			Plaintext	=	"6bc1bee22e409f96e93d7e117393172a" +
						"ae2d8a571e03ac9c9eb76fac45af8e51" +
						"30c81c46a35ce411e5fbc1191a0a52ef" +
						"f69f2445df4f9b17ad2b417be66c3710",
			Ciphertext	=	"3b3fd92eb72dad20333449f8e83cfb4a" +
						"7789508d16918f03f53c52dac54ed825" +
						"9740051e9c5fecf64344f7a82260edcc" +
						"304c6528f659c77866a510d9c1d6ae5e"
		};

		private static readonly BlockcipherTestVector ofbVector192 = new BlockcipherTestVector {
			Cipher		=	BlockcipherKind.AES,
			Mode		=	BlockcipherModeKind.OutputFeedback,
			Key		=	"8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b",
			Iv		=	"000102030405060708090a0b0c0d0e0f",
			Plaintext	=	"6bc1bee22e409f96e93d7e117393172a" +
						"ae2d8a571e03ac9c9eb76fac45af8e51" +
						"30c81c46a35ce411e5fbc1191a0a52ef" +
						"f69f2445df4f9b17ad2b417be66c3710",
			Ciphertext	=	"cdc80d6fddf18cab34c25909c99a4174" +
						"fcc28b8d4c63837c09e81700c1100401" +
						"8d9a9aeac0f6596f559c6d4daf59a5f2" +
						"6d9f200857ca6c3e9cac524bd9acc92a"
		};

		private static readonly BlockcipherTestVector ctrVector192 = new BlockcipherTestVector {
			Cipher		=	BlockcipherKind.AES,
			Mode		=	BlockcipherModeKind.Counter,
			Key		=	"8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b",
			Iv		=	"000102030405060708090a0b0c0d0e0f",
			Plaintext	=	"6bc1bee22e409f96e93d7e117393172a" +
						"ae2d8a571e03ac9c9eb76fac45af8e51" +
						"30c81c46a35ce411e5fbc1191a0a52ef" +
						"f69f2445df4f9b17ad2b417be66c3710",
			Ciphertext	=	"cdc80d6fddf18cab34c25909c99a4174" +
						"a90b2b2b80deda532d5427537f8cbd88" +
						"eaff8fd059a7995d0233e064f04f1631" +
						"d4d223fa51805f30df67f2aa5823f0a7"
		};

		private static readonly BlockcipherStreamTestVector ctrStreamVector192 = new BlockcipherStreamTestVector {
			Cipher		=	BlockcipherKind.AES,
			Mode		=	BlockcipherModeKind.Counter,
			Key		=	"8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b",
			Iv		=	"000102030405060708090a0b0c0d0e0f",
			Plaintext	=	"6bc1bee22e409f96e93d7e117393172a" +
						"ae2d8a571e03ac9c9eb76fac45af8e51" +
						"30c81c46a35ce411e5fbc1191a0a52ef" +
						"f69f2445df4f9b17ad",
			Ciphertext	=	"cdc80d6fddf18cab34c25909c99a4174" +
						"a90b2b2b80deda532d5427537f8cbd88" +
						"eaff8fd059a7995d0233e064f04f1631" +
						"d4d223fa51805f30df"
		};

		public static bool TestECB()
		{
			return ecbVector128.Test() && ecbVector192.Test();
		}

		public static bool TestCBC()
		{
			return cbcVector128.Test() && cbcVector192.Test();
		}

		public static bool TestCFB()
		{
			return cfbVector128.Test() && cfbVector192.Test();
		}

		public static bool TestOFB()
		{
			return ofbVector128.Test() && ofbVector192.Test();
		}

		public static bool TestCTR()
		{
			return ctrVector192.Test();
		}

		public static bool TestCTRStream()
		{
			return ctrStreamVector192.Test();
		}

		public static double Bench128Ctr()
		{
			return new TestBlockStreamcipherThroughput {
				cipherkind = BlockcipherKind.AES,
				streammode = BlockcipherModeKind.Counter,
				keybits = 128,
				megabytes = 128
			}.Run();
		}

		public static double Bench192Ctr()
		{
			return new TestBlockStreamcipherThroughput {
				cipherkind = BlockcipherKind.AES,
				streammode = BlockcipherModeKind.Counter,
				keybits = 192,
				megabytes = 128
			}.Run();
		}

		public static double Bench128EcbEnc()
		{
			return new TestBlockcipherThroughput {
				cipherkind = BlockcipherKind.AES,
				ciphermode = BlockcipherModeKind.ElectronicCodeBook,
				keybits = 128,
				keytype = KeyKind.Encrypt,
				megabytes = 128
			}.Run();
		}

		public static double Bench192EcbEnc()
		{
			return new TestBlockcipherThroughput {
				cipherkind = BlockcipherKind.AES,
				ciphermode = BlockcipherModeKind.ElectronicCodeBook,
				keybits = 192,
				keytype = KeyKind.Encrypt,
				megabytes = 128
			}.Run();
		}
	}
}
