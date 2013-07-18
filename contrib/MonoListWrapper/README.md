MonoListWrapper
===============

What is it?
-----------

A C library to make it easier to work with System.Collections.Generic.List<T> objects from the native side of an embedded Mono.

It is:

* Fast
* Allocation-free (except where necessary)
* Pretty small
* Particularly focused on fast bulk-loading of data to be returned to Managed land

It is not:

* Hugely typesafe (it is C after all)
* a good example of respect for encapsulation
* Keyser Soze

How do I use it?
----------------

    #include "MonoListWrapper.h"

    // Make a wrapper, wherever you like
    MonoListWrapper wr;
    
    // Initialise it - let's assume myList is a MonoObject* to
    // a System.Collections.Generic.List<int>
    mono_listwrapper_init(&wr, myList)
    
    // Bulk load data all in one go - almost a naked memcpy for primitives
    mono_listwrapper_load(&wr, bigArrayOfInts, sizeof(int), numberOfThingsInTheArray);
    
    // Or pin it and write some unknown-ahead-of-time number of entries in
    guint32 pinHandle;
    int* ptr = mono_listwrapper_begin_writing(&wr, /* capacity to reserve: */ 100, &pinHandle);
    ptr[0] = 1;
    ptr[1] = 3;
    ptr[2] = 3;
    ptr[3] = 7;
    mono_listwrapper_end_writing(&wr, /* number actually written: */ 4, pinHandle);
    
    // No read access yet :)
    
Just how fast is it?
--------------------

On my Macbook Air, 1.6GHz Core i5, sending 1000 integers from native to managed land
by various methods repeated 100,000 times:

Method                                                    | Time
----------------------------------------------------------|-------
In a newly allocated array, data memcopied in | 957ms
In a passed-in list with reserved capacity, bulk loading the data | 30ms
In a newly allocated array, data computed cell-by-cell | 1843ms
In a passed-in list, data computer cell-by-cell and stored via mono_listwrapper_begin_write | 775ms
    
Also, sending 1000 class instances from native to managed land by various methods,
repeated 100,000 times - these are slower because there's more GC involvement:

Method                                                    | Time
----------------------------------------------------------|-------
mono_array_clone of sample data set | 2014ms
Bulk load into existing list | 67ms
In a newly allocated array, mono_gc_wbarrier_generic_store each cell | 3819ms
Existing list, via mono_listwrapper_begin_writing | 926ms
    
These tests are all in the demo program.

What's still to do?
-------------------

* Make sure it works with AOT
* Read support
* More tests
* Error handling/type verification