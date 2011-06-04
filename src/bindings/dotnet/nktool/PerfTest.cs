/*
 * libk - PerfTest.cs
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

using System;
using System.Diagnostics;
using nlibk;

namespace nktool
{
	public class PerfWatch
	{
		private Stopwatch watch;

		public PerfWatch()
		{
			watch = new Stopwatch();
		}

		public void Start()
		{
			watch.Start();
		}

		public void Stop()
		{
			watch.Stop();
		}

		public double milliseconds()
		{
			return (double)watch.ElapsedMilliseconds;
		}

		public double seconds()
		{
			return (double)watch.ElapsedMilliseconds / 1000.0;
		}
	}

	public class TestBlockcipherThroughput
	{
		public BlockcipherKind cipherkind;
		public BlockcipherModeKind ciphermode;
		public KeyKind keytype;
		public int keybits;
		public int megabytes;

		public double Run()
		{
			PerfWatch t = new PerfWatch();
			byte[] key = new byte[(keybits + 7) / 8];
			byte[] input = new byte[megabytes * 1048576];
			byte[] output = new byte[megabytes * 1048576];
			t.Start();
			t.Stop();
			using (Blockcipher bc = new Blockcipher(cipherkind)) {
				byte[] iv = new byte[bc.Blocksize];

				bc.SetMode(ciphermode);
				bc.SetKey(keytype, key, keybits);
				bc.SetIV(iv);
				bc.Update(input, output);
				GC.Collect();
				t.Start();
				bc.Update(input, output);
				t.Stop();
			}
			GC.Collect();
			return megabytes / t.seconds();
		}
	}

	public class TestStreamcipherThroughput
	{
		public StreamcipherKind cipherkind;
		public int keybits;
		public int megabytes;

		public double Run()
		{
			PerfWatch t = new PerfWatch();
			byte[] key = new byte[(keybits + 7) / 8];
			byte[] input = new byte[megabytes * 1048576];
			byte[] output = new byte[megabytes * 1048576];
			t.Start();
			t.Stop();
			using (Streamcipher sc = new Streamcipher(cipherkind)) {
				sc.SetKey(key, keybits);
				sc.Update(input, output);
				GC.Collect();
				t.Start();
				sc.Update(input, output);
				t.Stop();
			}
			GC.Collect();
			return megabytes / t.seconds();
		}
	}

	public class TestBlockStreamcipherThroughput
	{
		public BlockcipherKind cipherkind;
		public BlockcipherModeKind streammode;
		public int keybits;
		public int megabytes;

		public double Run()
		{
			PerfWatch t = new PerfWatch();
			byte[] key = new byte[(keybits + 7) / 8];
			byte[] input = new byte[megabytes * 1048576];
			byte[] output = new byte[megabytes * 1048576];
			t.Start();
			t.Stop();
			using (Streamcipher sc = new Streamcipher(cipherkind, streammode)) {
				sc.SetKey(key, keybits);
				sc.Update(input, output);
				GC.Collect();
				t.Start();
				sc.Update(input, output);
				t.Stop();
			}
			GC.Collect();
			return megabytes / t.seconds();
		}
	}

	public class TestHashThroughput
	{
		public HashKind Hashsum;
		public int DigestBits;
		public int megabytes;

		public double Run()
		{
			PerfWatch t = new PerfWatch();
			byte[] digest = new byte[(DigestBits + 7) / 8];
			byte[] input = new byte[megabytes * 1048576];
			t.Start();
			t.Stop();
			using (Hash hash = new Hash(Hashsum, 0)) {
				hash.Update(input, input.Length);
				hash.Final(digest);
				hash.Reset();
				GC.Collect();
				t.Start();
				hash.Update(input, input.Length);
				hash.Final(digest);
				t.Stop();
			}
			GC.Collect();
			return megabytes / t.seconds();
		}
	}

	public class TestPrngThroughput
	{
		public PrngKind Prng;
		public int megabytes;

		public double Run()
		{
			byte[] seed = new byte[] {
				0x01, 0x23, 0x02, 0x34, 0x03, 0x45, 0x04, 0x56
			};
			PerfWatch t = new PerfWatch();
			byte[] output = new byte[megabytes * 1048576];
			t.Start();
			t.Stop();
			using (Prng prng = new Prng(Prng)) {
				prng.SetSeed(seed);
				prng.Update(output);
				prng.SetSeed(seed);
				GC.Collect();
				t.Start();
				prng.Update(output);
				t.Stop();
			}
			GC.Collect();
			return megabytes / t.seconds();
		}
	}
}
