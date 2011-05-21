/*
 * libk - UnmanagedError.cs
 *
 * 2011, Janos Laube <janos.dev@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
*/

using System;
using System.Runtime.InteropServices;
using System.Security;

namespace nlibk
{
	internal sealed class UnmanagedError
	{
		#region C API Interface

		[SuppressUnmanagedCodeSecurityAttribute]
		internal static class SafeNativeMethods
		{
			/* void k_set_error_handler(k_err_fn error_handler); */
			[DllImport("libk", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
			internal static extern void k_set_error_handler(ErrorCallback errorcallback);
		}

		#endregion

		internal delegate void ErrorCallback(ErrorKind error, ErrorLevel level, string errormessage);
		private ErrorKind LastError;
		private string LastErrorMessage;
		private ErrorCallback errorDelegate;
		private void defaultErrorDelegate(ErrorKind error, ErrorLevel level, string errormessage)
		{
			if (level == ErrorLevel.K_LWARN) {
				// TODO: use a user supplied callback here or provide some other mechanism to override this
				// NOTE: if using a callback here, the user _must not_ throw exceptions in that callback
				Console.WriteLine("nlibk warning: {0}", errormessage);
				return;
			}
			LastError = error;
			LastErrorMessage = errormessage;
		}

		[ThreadStatic]
		private static volatile UnmanagedError instance;
		private static object syncRoot = new Object();

		private UnmanagedError()
		{
			errorDelegate = new ErrorCallback(defaultErrorDelegate);
			SafeNativeMethods.k_set_error_handler(errorDelegate);
		}

		private static UnmanagedError Instance
		{
			get
			{
				if (instance == null) {
					lock (syncRoot) {
						if (instance == null)
							instance = new UnmanagedError();
					}
				}
				return instance;
			}
		}

		public static ErrorKind RegisterThread()
		{
			return Instance.LastError;
		}

		public static void ThrowLastError()
		{
			switch (Instance.LastError) {
				default:
					throw new Exception(Instance.LastErrorMessage);
			}
		}
	}
}
