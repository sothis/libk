/*
 * libk - StringConvert.cs
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

using System;
using System.Collections;

namespace nktool
{
	public static class StringConvert
	{
		public static byte[] Base16ToByte(string hex)
		{
			if (hex == null)
				return null;
			var chars = hex.Length;
			var al = new ArrayList(chars / 2);
			for (var i = 0; i < chars; i += 2) {
				var b = Convert.ToByte(hex.Substring(i, 2), 16);
				al.Add(b);
			}
			return (byte[])al.ToArray(typeof(byte));
		}

		public static string ByteToBase16(byte[] data)
		{
			if (data == null)
				return null;
			return BitConverter.ToString(data).Replace("-", "").ToLower();
		}

		public static byte[] Base64ToByte(string base64)
		{
			if (base64 == null)
				return null;
			return Convert.FromBase64String(base64);
		}

		public static string ByteToBase64(byte[] data, bool linebreaks)
		{
			if (data == null)
				return null;
			var o = linebreaks ? Base64FormattingOptions.InsertLineBreaks : Base64FormattingOptions.None;
			var res = Convert.ToBase64String(data, o);
			return res.Replace("\r", "");
		}

		public static string ByteToBase64(byte[] data)
		{
			return ByteToBase64(data, false);
		}
	}
}
