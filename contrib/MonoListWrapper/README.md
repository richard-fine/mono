MonoListWrapper
===============

What is it?
-----------

A C++ library to make it easier to work with System.Collections.Generic.List<T> objects from the native side of an embedded Mono.

It is:

* Fast
* Allocation-free (except where necessary)
* Pretty small
* Particularly focused on fast bulk-loading of data to be returned to Managed land

It is not:

* Hugely typesafe - up to you to ensure your List<T> is a type that matches your MonoListWrapper<T>
* a good example of respect for encapsulation
* Keyser Soze

How do I use it?
----------------

    #include "MonoListWrapper.hpp"

    // Get all the reflection data needed. These objects can be set up once per list type 
    // and reused, see below
    MonoListReflectionCache refl;
    refl.PopulateFrom(mono_object_get_class(myList));
    
    // Make the wrapper - assumes myList is a MonoObject* to 
    // a System.Collections.Generic.List<int>
    MonoListWrapper<int> listOfInt(myList, refl);
    
    // Use it
    listOfInt.clear();
    listOfInt.setCapacity(100);
    listOfInt.add(5);
    
    // Bulk load data all in one go - almost a naked memcpy for primitives
    listOfInt.load(bigIntArray, 500);
    
    // Or pin it and write some unknown-ahead-of-time number of entries in
    guint32 pinHandle;
    int* ptr = listOfInt.beginWriting(/* capacity to reserve: */ 100, &pinHandle);
    ptr[0] = 1;
    ptr[1] = 3;
    ptr[2] = 3;
    ptr[3] = 7;
    listOfInt.endWriting(/* number actually written: */ 4, pinHandle);
    
    // No read access yet :)
    
    // Instead of fetching reflection information every time, use ReflectionCacheGroup:
    ReflectionCacheGroup cacheGroup;
    MonoListWrapper<int> listOfInt(myList, cacheGroup.GetCacheFor(myList));
    
    // Or just save the MonoListReflectionCache in a nearby variable
    // and retrieve it when needed. Just don't keep it across an AppDomain reload.
    
Just how fast is it?
--------------------

On my Macbook Air, 1.6GHz Core i5, sending 1000 integers from native to managed land
by various methods repeated 100,000 times:

Method                                                    | Time
----------------------------------------------------------|-------
Allocating an array, copying the data in and returning it | 657ms
Using a passed-in list with reserved capacity, creating the wrapper without caching the reflection information, loading the data and returning it | 157ms
Using a passed-in list with reserved capacity, creating the wrapper with cached reflection information, loading the data and returning it | 23ms
    
Also, sending 1000 class instances from native to managed land by various methods,
repeated 100,000 times - these are slower because there's more GC involvement:

Method                                                    | Time
----------------------------------------------------------|-------
Allocating an array, copying the data in and returning it | 1378ms
Using a passed-in list with reserved capacity, creating the wrapper with cached reflection information, loading the data and returning it | 83ms
    
These tests are all in the demo program.

What's still to do?
-------------------

* Make sure it works with AOT
* Read support
* More tests
* Error handling/type verification