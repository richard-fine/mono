using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Diagnostics;
using System.Linq;

class FastCollectionsDemo {
	
    // Primitive tests
    
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static int[] GetSampleDataNewArray();
	
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static void GetSampleDataExistingList(List<int> result);
    
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static void GetAltSampleDataExistingList(List<float> result);
	
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static int[] GetDynamicSampleDataNewArray();
	
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static void GetDynamicSampleDataExistingList(List<int> result);
    
    // Object tests
    
    public class MyObject
    {
        public int value;
    }
    
    public static MyObject[] SourceObjects;
    
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    extern static void SetObjSampleData(MyObject[] arr);
	
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static MyObject[] GetObjSampleDataNewArray();
	
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static void GetObjSampleDataExistingList(List<MyObject> result);
	
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static MyObject[] GetObjDynamicSampleDataNewArray();
	
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static void GetObjDynamicSampleDataExistingList(List<MyObject> result);
    
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
    
    public static void CheckCorrectness(IEnumerable<MyObject> data)
    {
        if(data.Count() != SourceObjects.Length)
        {
            System.Console.WriteLine("Sample data is the wrong length! Expected {0}, actual {1}.", SourceObjects.Length, data.Count());
            return;
        }
        
        int i = 0;
        foreach(var elem in data)
        {
            if(elem != SourceObjects[i])
                System.Console.WriteLine("Error in slot {0}! Expected {0}, actual {1}.", SourceObjects[i], elem);
            ++i;
        }

    }

	
	static void Main () {
    
        var l = new List<int>();
        var l2 = new List<float>();
    
        l.Capacity = 1000;
        l2.Capacity = 1000;
    
        System.Console.WriteLine("Checking correctness of GetSampleDataNewArray:");
        CheckCorrectness(GetSampleDataNewArray());
        System.Console.WriteLine("Done.");
        
        System.Console.WriteLine("Checking correctness of GetSampleDataExistingList:");
        GetSampleDataExistingList(l);
        CheckCorrectness(l);
        System.Console.WriteLine("Done.");
        
        System.Console.WriteLine("Checking correctness of GetDynamicSampleDataNewArray:");
        CheckCorrectness(GetDynamicSampleDataNewArray());
        System.Console.WriteLine("Done.");

        System.Console.WriteLine("Checking correctness of GetDynamicSampleDataExistingList:");
        GetDynamicSampleDataExistingList(l);
        CheckCorrectness(l);
        System.Console.WriteLine("Done.");
    
        RunTimingTest(() => GetSampleDataNewArray(), iterations, "GetSampleDataNewArray");
        
        RunTimingTest(() => GetSampleDataExistingList(l), iterations, "GetSampleDataExistingList");
        
        RunTimingTest(() => GetAltSampleDataExistingList(l2), iterations, "GetAltSampleDataExistingList");
        
        RunTimingTest(() => GetDynamicSampleDataNewArray(), iterations, "GetDynamicSampleDataNewArray");
        
        RunTimingTest(() => GetDynamicSampleDataExistingList(l), iterations, "GetDynamicSampleDataExistingList");
        
        SourceObjects = new MyObject[1000];
        for(int i = 0; i < 1000; ++i)
            SourceObjects[i] = new MyObject{value = i};
            
        SetObjSampleData(SourceObjects);
        
        var l3 = new List<MyObject>();
        
        System.Console.WriteLine("Checking correctness of GetSampleDataNewArray:");
        CheckCorrectness(GetObjSampleDataNewArray());
        System.Console.WriteLine("Done.");
        
        System.Console.WriteLine("Checking correctness of GetSampleDataExistingList:");
        GetObjSampleDataExistingList(l3);
        CheckCorrectness(l3);
        System.Console.WriteLine("Done.");
        
        System.Console.WriteLine("Checking correctness of GetDynamicSampleDataNewArray:");
        CheckCorrectness(GetObjDynamicSampleDataNewArray());
        System.Console.WriteLine("Done.");

        System.Console.WriteLine("Checking correctness of GetDynamicSampleDataExistingList:");
        GetObjDynamicSampleDataExistingList(l3);
        CheckCorrectness(l3);
        System.Console.WriteLine("Done.");
    
        RunTimingTest(() => GetObjSampleDataNewArray(), iterations, "GetObjSampleDataNewArray");
        
        RunTimingTest(() => GetObjSampleDataExistingList(l3), iterations, "GetObjSampleDataExistingList");
        
        RunTimingTest(() => GetObjDynamicSampleDataNewArray(), iterations, "GetObjDynamicSampleDataNewArray");
        
        RunTimingTest(() => GetObjDynamicSampleDataExistingList(l3), iterations, "GetObjDynamicSampleDataExistingList");

    }
}
