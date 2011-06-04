/*
 * libk - BlockcipherTestVector.cs
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
	public class BlockcipherTestVector
	{
		public BlockcipherKind Cipher;
		public BlockcipherModeKind Mode;
		public byte[] _key;
		public byte[] _iv;
		public byte[] _plaintext;
		public byte[] _ciphertext;

		public string Key
		{
			set
			{
				_key = StringConvert.Base16ToByte(value);
			}
		}

		public string Iv
		{
			set
			{
				_iv = StringConvert.Base16ToByte(value);
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

		public bool CipherMatch(byte[] compare)
		{
			return StringConvert.ByteToBase16(compare) == StringConvert.ByteToBase16(_ciphertext);
		}
		public bool PlainMatch(byte[] compare)
		{
			return StringConvert.ByteToBase16(compare) == StringConvert.ByteToBase16(_plaintext);
		}

		public bool Test()
		{
			bool enc = false, dec = false;
			byte[] ciphertext = null;
			byte[] plaintext = null;

			using (Blockcipher cipher = new Blockcipher(Cipher)) {
				cipher.SetMode(Mode);
				if (_iv != null)
					cipher.SetIV(_iv);
				cipher.SetKey(KeyKind.Encrypt, _key);
				ciphertext = new byte[_plaintext.Length];
				cipher.Update(_plaintext, ciphertext);
				enc = CipherMatch(ciphertext);

				cipher.SetKey(KeyKind.Decrypt, _key);
				if (_iv != null)
					cipher.SetIV(_iv);
				plaintext = new byte[_ciphertext.Length];
				cipher.Update(_ciphertext, plaintext);
				dec = PlainMatch(plaintext);
			}
			return enc && dec;
		}
	}

	public sealed class BlockcipherStreamTestVector : BlockcipherTestVector
	{
		public new bool Test()
		{
			bool enc = false, dec = false, dec2 = false;
			byte[] ciphertext = null;
			byte[] plaintext = null;

			using (Streamcipher cipher = new Streamcipher(Cipher, Mode)) {
				cipher.SetKey(_iv ,_key);
				ciphertext = new byte[_plaintext.Length];

				byte[] c = new byte[64];
				byte[] p = new byte[64];
				/* test byte-for-byte updating */
				for (int s = 0; s < _plaintext.Length; ++s) {
					System.Buffer.BlockCopy(_plaintext, s, p, 0, 1);
					cipher.Update(p, c, 1);
					System.Buffer.BlockCopy(c, 0, ciphertext, s, 1);
				}

				enc = CipherMatch(ciphertext);

				cipher.SetKey(_iv ,_key);
				plaintext = new byte[_ciphertext.Length];
				cipher.Update(_ciphertext, plaintext);
				dec = PlainMatch(plaintext);

				/* test mix-sized block updating */
				if (_ciphertext.Length > 48) {
					cipher.SetKey(_iv ,_key);

					System.Buffer.BlockCopy(_ciphertext, 0, c, 0, 24);
					cipher.Update(c, p, 24);
					System.Buffer.BlockCopy(p, 0, plaintext, 0, 24);

					System.Buffer.BlockCopy(_ciphertext, 24, c, 0, 6);
					cipher.Update(c, p, 6);
					System.Buffer.BlockCopy(p, 0, plaintext, 24, 6);

					System.Buffer.BlockCopy(_ciphertext, 30, c, 0, 5);
					cipher.Update(c, p, 5);
					System.Buffer.BlockCopy(p, 0, plaintext, 30, 5);

					System.Buffer.BlockCopy(_ciphertext, 35, c, 0, 12);
					cipher.Update(c, p, 12);
					System.Buffer.BlockCopy(p, 0, plaintext, 35, 12);

					int remaining = _ciphertext.Length - 6 - 24 - 5 - 12;
					for (int s = 0; s < remaining; ++s) {
						System.Buffer.BlockCopy(_ciphertext, s+6+24+5+12, c, 0, 1);
						cipher.Update(c, p, 1);
						System.Buffer.BlockCopy(p, 0, plaintext, s+6+24+5+12, 1);
					}
					dec2 = PlainMatch(plaintext);
				}
				else dec2 = true;
			}
			return enc && dec && dec2;
		}
	}
}
