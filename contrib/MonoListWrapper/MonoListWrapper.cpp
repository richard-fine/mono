#include <mono/mini/jit.h>
#include "MonoListWrapper.hpp"

MonoListReflectionCache::MonoListReflectionCache()
{
	klass = 0;
}

void MonoListReflectionCache::PopulateFrom(MonoClass* inKlass)
{
	klass = inKlass;
	
	capacityProp = mono_class_get_property_from_name(klass, "Capacity");
	
	addMethod = mono_class_get_method_from_name(klass, "Add", 1);
	clearMethod = mono_class_get_method_from_name(klass, "Clear", 0);
	
	size = mono_class_get_field_from_name(klass, "_size");
	items = mono_class_get_field_from_name(klass, "_items");
}