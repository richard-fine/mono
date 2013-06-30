using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Diagnostics;

	class FastCollectionsDemo {
		
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		extern static int[] GetSampleDataNewArray();
		
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		extern static void GetSampleDataExistingList(List<int> result);
		
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		extern static void GetSampleDataExistingListUnshared(List<int> result);
		
		public const int iterations = 100000;
		
		static void Main () {
			var sw = new Stopwatch();
			
			System.Console.WriteLine("Running {0} iterations of GetSampleDataNewArray()", iterations);
			sw.Start();
			for(int i = 0; i < iterations; ++i)
				GetSampleDataNewArray();
			sw.Stop();
			System.Console.WriteLine("Done in {0}ms.", sw.ElapsedMilliseconds);
			
			sw.Reset();
			
			var l = new List<int>();
			System.Console.WriteLine("Running {0} iterations of GetSampleDataExistingList()", iterations);
			sw.Start();
			for(int i = 0; i < iterations; ++i)
				GetSampleDataExistingList(l);
			sw.Stop();
			System.Console.WriteLine("Done in {0}ms.", sw.ElapsedMilliseconds);
			
			sw.Reset();
			
			System.Console.WriteLine("Running {0} iterations of GetSampleDataExistingListUnshared()", iterations);
			sw.Start();
			for(int i = 0; i < iterations; ++i)
				GetSampleDataExistingListUnshared(l);
			sw.Stop();
			System.Console.WriteLine("Done in {0}ms.", sw.ElapsedMilliseconds);
		}
	}
