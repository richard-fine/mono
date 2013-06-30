#include <mono/mini/jit.h>
#include <mono/metadata/object.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <string.h>
#include <stdlib.h>
#include "MonoListWrapper.hpp"
#include "ReflectionCacheGroup.hpp"

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

MonoArray* GetSampleDataNewArray()
{
	MonoArray* result = mono_array_new(mono_domain_get(), mono_get_int32_class(), DATASIZE);
	
	gpointer dest = mono_array_addr_with_size(result, sizeof(int), 0);
	memcpy(dest, SAMPLE_DATA, sizeof(int) * sizeof(DATASIZE));
	
	return result;
}

ReflectionCacheGroup* cacheGroup;

void GetSampleDataExistingList(MonoObject* list)
{	
	static MonoListReflectionCache& intList(cacheGroup->GetCacheFor(list));
	MonoListWrapper<int> wr(list, intList);
	wr.load(SAMPLE_DATA, DATASIZE);
}

void GetSampleDataExistingListUnshared(MonoObject* list)
{	
	MonoListReflectionCache intList;
	MonoListWrapper<int> wr(list, intList);
	wr.load(SAMPLE_DATA, DATASIZE);
}

int 
main (int argc, char* argv[]) {
	MonoDomain *domain;
	const char *file;
	int retval;
    
    cacheGroup = new ReflectionCacheGroup();
	
	int i;
	for(i = 0; i < DATASIZE; ++i)
		SAMPLE_DATA[i] = i;
	
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

	main_function (domain, file, argc - 1, argv + 1);

	retval = mono_environment_exitcode_get ();
	
	mono_jit_cleanup (domain);
    
    delete cacheGroup;
    
	return retval;
}

