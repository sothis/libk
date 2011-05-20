namespace nlibk
{
	public enum KeyKind
	{
		Encrypt			= 0,
		Decrypt			= 1,
	};

	public enum BlockcipherKind
	{
		Threefish256		= 1,
		Threefish512		= 2,
		Threefish1024		= 3,
		AES			= 4,
	};

	public enum StreamcipherKind
	{
		ARC4			= 1,
	};

	public enum BlockcipherModeKind
	{
		ElectronicCodeBook	= 1,
		CipherBlockChaining	= 2,
		CipherFeedback		= 3,
		OutputFeedback		= 4,
		Counter			= 5,
	};

	public enum BlockcipherStreamModeKind
	{
		CipherFeedback		= BlockcipherModeKind.CipherFeedback,
		OutputFeedback		= BlockcipherModeKind.OutputFeedback,
		Counter			= BlockcipherModeKind.Counter,
	};

	public enum HashKind
	{
		Skein256		= 1,
		Skein512		= 2,
		Skein1024		= 3,
		SHA1			= 4
	};

	public enum PrngKind
	{
		MersenneTwister19937_32	= 1
	};
}
