/*
 * libk - Mt19937_32Test.cs
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
