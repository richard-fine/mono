using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Diagnostics;
using System.Linq;

class FastCollectionsDemo {
	
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static int[] GetSampleDataNewArray();
	
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static void GetSampleDataExistingList(List<int> result);
	
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static void GetSampleDataExistingListUnshared(List<int> result);
	
	public const int iterations = 100000;
    
    public static void RunTimingTest(Action test, int iterations, string name)
    {
        var sw = new Stopwatch();
        
        System.Console.WriteLine("Timing {0} iterations of {1}", iterations, name);
        sw.Start();
        for(int i = 0; i < iterations; ++i)
            test();
        sw.Stop();
        System.Console.WriteLine("Done in {0}ms.", sw.ElapsedMilliseconds);
    }
    
    public static void CheckCorrectness(IEnumerable<int> data)
    {
        if(data.Count() != 1000)
        {
            System.Console.WriteLine("Sample data is the wrong length! Expected {0}, actual {1}.", 1000, data.Count());
            return;
        }
        
        int i = 0;
        foreach(var elem in data)
        {
            if(elem != i)
                System.Console.WriteLine("Error in slot {0}! Expected {0}, actual {1}.", i, elem);
            ++i;
        }
    }
	
	static void Main () {
    
        var l = new List<int>();
    
        System.Console.WriteLine("Checking correctness of GetSampleDataNewArray:");
        CheckCorrectness(GetSampleDataNewArray());
        System.Console.WriteLine("Done.");
        
        System.Console.WriteLine("Checking correctness of GetSampleDataExistingList:");
        GetSampleDataExistingList(l);
        CheckCorrectness(l);
        System.Console.WriteLine("Done.");
        
        System.Console.WriteLine("Checking correctness of GetSampleDataExistingListUnshared:");
        GetSampleDataExistingListUnshared(l);
        CheckCorrectness(l);
        System.Console.WriteLine("Done.");
    
        RunTimingTest(() => GetSampleDataNewArray(), iterations, "GetSampleDataNewArray");
        
        RunTimingTest(() => GetSampleDataExistingList(l), iterations, "GetSampleDataExistingList");
        
        RunTimingTest(() => GetSampleDataExistingListUnshared(l), iterations, "GetSampleDataExistingListUnshared");
        
    }
}
