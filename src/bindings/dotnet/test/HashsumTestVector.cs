using nlibk;

namespace nlibktest
{
	public sealed class HashsumTestVector
	{
		public HashKind Hashsum;
		public int OutputBits;
		private byte[] _message;
		private byte[] _digest;

		public string Message
		{
			set
			{
				_message = StringConvert.Base16ToByte(value);
			}
		}
		public string Digest
		{
			set
			{
				_digest = StringConvert.Base16ToByte(value);
			}
		}

		private bool DigestMatch(byte[] compare)
		{
			return StringConvert.ByteToBase16(compare) == StringConvert.ByteToBase16(_digest);
		}

		public bool Test()
		{
			bool res = false;
			byte[] digest = null;

			using (Hash hash = new Hash(Hashsum, OutputBits)) {
				digest = new byte[(OutputBits + 7) / 8];
				hash.Update(_message, _message == null ? 0 : _message.Length);
				hash.Final(digest);
				res = DigestMatch(digest);
			}
			return res;
		}
	}
}
