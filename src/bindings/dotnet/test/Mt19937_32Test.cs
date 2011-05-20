using nlibk;

namespace nlibktest
{
	public static class Mt19937_32Test
	{
		public static double Bench()
		{
			return new TestPrngThroughput {
				Prng = PrngKind.MersenneTwister19937_32,
				megabytes = 128
			}.Run();
		}
	}
}
