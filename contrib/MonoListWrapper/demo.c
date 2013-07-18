#include <mono/mini/jit.h>
#include <mono/metadata/object.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "MonoListWrapper.h"

static void main_function (MonoDomain *domain, const char *file, int argc, char **argv)
{
	MonoAssembly *assembly;

	/* Loading an assembly makes the runtime setup everything
	 * needed to execute it. If we're just interested in the metadata
	 * we'd use mono_image_load (), instead and we'd get a MonoImage*.
	 */
	assembly = mono_domain_assembly_open (domain, file);
	if (!assembly)
		exit (2);
	/*
	 * mono_jit_exec() will run the Main() method in the assembly.
	 * The return value needs to be looked up from
	 * System.Environment.ExitCode.
	 */
	mono_jit_exec (domain, assembly, argc, argv);
}

#define DATASIZE 1000
int SAMPLE_DATA[DATASIZE];

float ALT_SAMPLE_DATA[DATASIZE];

MonoArray* GetSampleDataNewArray()
{
	MonoArray* result = mono_array_new(mono_domain_get(), mono_get_int32_class(), DATASIZE);
	
	gpointer dest = mono_array_addr_with_size(result, sizeof(int), 0);
	memcpy(dest, SAMPLE_DATA, sizeof(int) * DATASIZE);
	
	return result;
}

void GetSampleDataExistingList(MonoObject* list)
{	
    MonoListWrapper wr;
    mono_listwrapper_init(&wr, list);
    mono_listwrapper_load(&wr, SAMPLE_DATA, sizeof(int), DATASIZE);
}

void GetSampleDataExistingListUnshared(MonoObject* list)
{	
    MonoListWrapper wr;
    mono_listwrapper_init(&wr, list);
    mono_listwrapper_load(&wr, SAMPLE_DATA, sizeof(int), DATASIZE);
}

void GetAltSampleDataExistingList(MonoObject* list)
{
    MonoListWrapper wr;
    mono_listwrapper_init(&wr, list);
    mono_listwrapper_load(&wr, ALT_SAMPLE_DATA, sizeof(float), DATASIZE);
}

int __attribute__ ((noinline)) GetDynamicData(int index)
{
    return index;
}

MonoArray* GetDynamicSampleDataNewArray()
{
	MonoArray* result = mono_array_new(mono_domain_get(), mono_get_int32_class(), DATASIZE);
	
	gpointer dest = mono_array_addr_with_size(result, sizeof(int), 0);
    
    int i;
    for(i = 0; i < DATASIZE; ++i)
        ((int*)dest)[i] = GetDynamicData(i);
	
	return result;
}

void GetDynamicSampleDataExistingList(MonoObject* list)
{
	MonoListWrapper wr;
    mono_listwrapper_init(&wr, list);
    
    guint32 handle;
    int* buf = (int*)mono_listwrapper_begin_writing(&wr, DATASIZE, &handle);
	
    int i;
    for(i = 0; i < DATASIZE; ++i)
        buf[i] = GetDynamicData(i);
    
    mono_listwrapper_end_writing(&wr, i, handle);
}

MonoArray* monoObjectSampleData;
guint32 sampleDataGCHandle;

void SetObjSampleData(MonoArray* arr)
{
    monoObjectSampleData = arr;
    sampleDataGCHandle = mono_gchandle_new((MonoObject*)arr, FALSE);
}

MonoArray* GetObjSampleDataNewArray()
{
	MonoArray* result = mono_array_clone(monoObjectSampleData);
    
    return result;
}

void GetObjSampleDataExistingList(MonoObject* list)
{
    MonoListWrapper wr;
    mono_listwrapper_init(&wr, list);
	mono_listwrapper_load(&wr, &((MonoObject**)monoObjectSampleData->vector)[0], sizeof(MonoObject*), monoObjectSampleData->max_length);
}


MonoObject* __attribute__ ((noinline)) GetObjDynamicData(int index)
{
    return ((MonoObject**)monoObjectSampleData->vector)[index];
}

MonoArray* GetObjDynamicSampleDataNewArray()
{
    MonoClass* elemClass = mono_class_get_element_class(mono_object_get_class((MonoObject*)monoObjectSampleData));
    mono_array_size_t arrayLength = mono_array_length(monoObjectSampleData);
	MonoArray* result = mono_array_new(mono_domain_get(), elemClass, arrayLength);
	
	gpointer dest = mono_array_addr_with_size(result, sizeof(int), 0);
    
    int i;
    for(i = 0; i < arrayLength; ++i)
        mono_gc_wbarrier_generic_store(&((MonoObject**)dest)[i], GetObjDynamicData(i));
    
	return result;
}

void GetObjDynamicSampleDataExistingList(MonoObject* list)
{
	MonoListWrapper wr;
    mono_listwrapper_init(&wr, list);
    
    mono_array_size_t arrayLength = mono_array_length(monoObjectSampleData);
    
    guint32 handle;
    MonoObject** buf = (MonoObject**)mono_listwrapper_begin_writing(&wr, arrayLength, &handle);
	
    int i;
    for(i = 0; i < arrayLength; ++i)
        buf[i] = GetObjDynamicData(i);
    
    mono_listwrapper_end_writing(&wr, arrayLength, handle);
}


int
main (int argc, char* argv[]) {
	MonoDomain *domain;
	const char *file;
	int retval;
	
    // Create sample data
	int i;
	for(i = 0; i < DATASIZE; ++i)
    {
		SAMPLE_DATA[i] = i;
        ALT_SAMPLE_DATA[i] = (float)sin(i);
    }
    
	
	if (argc < 2){
		fprintf (stderr, "Please provide an assembly to load\n");
		return 1;
	}
	file = argv [1];
	/*
	 * mono_jit_init() creates a domain: each assembly is
	 * loaded and run in a MonoDomain.
	 */
	domain = mono_jit_init (file);
	
	mono_add_internal_call("FastCollectionsDemo::GetSampleDataNewArray", (void*)GetSampleDataNewArray);
	mono_add_internal_call("FastCollectionsDemo::GetSampleDataExistingList", (void*)GetSampleDataExistingList);
	mono_add_internal_call("FastCollectionsDemo::GetSampleDataExistingListUnshared", (void*)GetSampleDataExistingListUnshared);
    mono_add_internal_call("FastCollectionsDemo::GetAltSampleDataExistingList", (void*)GetAltSampleDataExistingList);
    mono_add_internal_call("FastCollectionsDemo::GetDynamicSampleDataNewArray", (void*)GetDynamicSampleDataNewArray);
	mono_add_internal_call("FastCollectionsDemo::GetDynamicSampleDataExistingList", (void*)GetDynamicSampleDataExistingList);
    
    mono_add_internal_call("FastCollectionsDemo::SetObjSampleData", (void*)SetObjSampleData);
    mono_add_internal_call("FastCollectionsDemo::GetObjSampleDataNewArray", (void*)GetObjSampleDataNewArray);
	mono_add_internal_call("FastCollectionsDemo::GetObjSampleDataExistingList", (void*)GetObjSampleDataExistingList);
    mono_add_internal_call("FastCollectionsDemo::GetObjDynamicSampleDataNewArray", (void*)GetObjDynamicSampleDataNewArray);
	mono_add_internal_call("FastCollectionsDemo::GetObjDynamicSampleDataExistingList", (void*)GetObjDynamicSampleDataExistingList);
	

	main_function (domain, file, argc - 1, argv + 1);

	retval = mono_environment_exitcode_get ();
	
	mono_jit_cleanup (domain);
    
	return retval;
}

