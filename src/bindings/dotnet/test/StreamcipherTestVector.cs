/*
 * StreamcipherTestVector.cs
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
	public sealed class StreamcipherTestVector
	{
		public StreamcipherKind Cipher;
		private byte[] _key;
		private byte[] _plaintext;
		private byte[] _ciphertext;

		public string Key
		{
			set
			{
				_key = StringConvert.Base16ToByte(value);
			}
		}

		public string Plaintext
		{
			set
			{
				_plaintext = StringConvert.Base16ToByte(value);
			}
		}

		public string Ciphertext
		{
			set
			{
				_ciphertext = StringConvert.Base16ToByte(value);
			}
		}

		private bool CipherMatch(byte[] compare)
		{
			return StringConvert.ByteToBase16(compare) == StringConvert.ByteToBase16(_ciphertext);
		}
		private bool PlainMatch(byte[] compare)
		{
			return StringConvert.ByteToBase16(compare) == StringConvert.ByteToBase16(_plaintext);
		}

		public bool Test()
		{
			bool enc = false, dec = false;
			byte[] ciphertext = null;
			byte[] plaintext = null;

			using (Streamcipher cipher = new Streamcipher(Cipher)) {
				cipher.SetKey(KeyKind.Encrypt, _key);
				ciphertext = new byte[_plaintext.Length];
				cipher.Update(_plaintext, ciphertext);
				enc = CipherMatch(ciphertext);

				cipher.SetKey(KeyKind.Decrypt, _key);
				plaintext = new byte[_ciphertext.Length];
				cipher.Update(_ciphertext, plaintext);
				dec = PlainMatch(plaintext);
			}
			return enc && dec;
		}
	}
}
