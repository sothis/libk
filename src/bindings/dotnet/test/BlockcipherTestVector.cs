using nlibk;

namespace nlibktest
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
			bool enc = false, dec = false;
			byte[] ciphertext = null;
			byte[] plaintext = null;

			using (Streamcipher cipher = new Streamcipher(Cipher, (BlockcipherStreamModeKind)Mode)) {
				cipher.SetNonce(_iv);
				cipher.SetKey(KeyKind.Encrypt, _key);
				ciphertext = new byte[_plaintext.Length];
				cipher.Update(_plaintext, ciphertext);
				enc = CipherMatch(ciphertext);

				cipher.SetKey(KeyKind.Decrypt, _key);
				cipher.SetNonce(_iv);
				plaintext = new byte[_ciphertext.Length];
				cipher.Update(_ciphertext, plaintext);
				dec = PlainMatch(plaintext);
			}
			return enc && dec;
		}
	}
}
