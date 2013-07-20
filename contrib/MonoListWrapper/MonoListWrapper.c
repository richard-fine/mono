#include <mono/mini/jit.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/class-internals.h>
#include "MonoListWrapper.h"

static struct
{
    char isInited;
	MonoProperty* capacityProp;
	MonoClassField* size;
	MonoClassField* items;
    MonoClassField* version;
} ListRefl = { 0 };


static void mono_listwrapper_init_reflection(MonoObject* listObj)
{
    MonoClass* klass = mono_object_get_class(listObj);
    
    ListRefl.capacityProp = mono_class_get_property_from_name(klass, "Capacity");
    
    ListRefl.size = mono_class_get_field_from_name(klass, "_size");
	ListRefl.items = mono_class_get_field_from_name(klass, "_items");
    ListRefl.version = mono_class_get_field_from_name(klass, "_version");
    
    ListRefl.isInited = 1;
}

void mono_listwrapper_clear_refl_data()
{
    ListRefl.isInited = 0;
}

void mono_listwrapper_init(MonoListWrapper* wrapper, MonoObject* list)
{
    if(list == NULL)
        mono_raise_exception(mono_get_exception_argument_null("list"));
    wrapper->list = list;
    
    if(ListRefl.isInited == 0)
        mono_listwrapper_init_reflection(list);
    
    wrapper->sizeField = (guint32*)((char*)list + ListRefl.size->offset);
    wrapper->versionField = (guint32*)((char*)list + ListRefl.version->offset);
    wrapper->itemsField = (MonoArray**)((char*)list + ListRefl.items->offset);
    
    MonoClass* listClass = mono_object_get_class(list);
        
    if(!mono_class_is_inflated(listClass))
        mono_raise_exception(mono_get_exception_invalid_cast());
        
    MonoGenericClass* genericClass = mono_class_get_generic_class(listClass);
    MonoGenericContext* genericContext = mono_generic_class_get_context(genericClass);
    
    wrapper->elementType = genericContext->class_inst->type_argv[0];
}

mono_array_size_t mono_listwrapper_get_capacity(MonoListWrapper* wrapper)
{
    return wrapper->itemsField != NULL ? (*(wrapper->itemsField))->max_length : 0;
}

void mono_listwrapper_set_capacity(MonoListWrapper* wrapper, mono_array_size_t value)
{
    if(mono_listwrapper_get_capacity(wrapper) == value) return;
    
    gpointer args[1];
    args[0] = &value;
    mono_property_set_value(ListRefl.capacityProp, wrapper->list, args, NULL);
}

guint32 mono_listwrapper_get_size(MonoListWrapper* wrapper)
{
    return *(wrapper->sizeField);
}

void mono_listwrapper_clear(MonoListWrapper* wrapper)
{
    // If the element type of the list is a value type, we can just do a 'fast resize'
    // If it's not a value type, we need to actually wipe the values back to zero so that
    // the GC doesn't treat them as outstanding references
    if(MONO_TYPE_IS_REFERENCE(wrapper->elementType) && (*(wrapper->sizeField) > 0))
    {
        int sz = mono_array_element_size (mono_object_class (*(wrapper->itemsField)));
        
        // Let's assume the array has been well-maintained and therefore that we only need to clear as far as the current size
        memset ((*wrapper->itemsField)->vector, 0, sz * (*(wrapper->sizeField)));
    }
    
    *(wrapper->sizeField) = 0;
    
    // also increment the version number so that any pending
    // enumerators get nuked
    
    (*(wrapper->versionField))++;
}

void mono_listwrapper_set_size(MonoListWrapper* wrapper, int size)
{
    // Ensure that the capacity is there
    if(mono_listwrapper_get_capacity(wrapper) < size)
        mono_listwrapper_set_capacity(wrapper, size);
    
    // If there are presently extra entries in the array, and this is a reference-typed list, clear the extra entries so that
    // the GC doesn't find them
    if(MONO_TYPE_IS_REFERENCE(wrapper->elementType) && (*(wrapper->sizeField) > size))
    {
        int sz = mono_array_element_size (mono_object_class (*(wrapper->itemsField)));
        
        memset ((char*)((*wrapper->itemsField)->vector) + sz * size, 0, sz * (*(wrapper->sizeField) - size));
    }
    
    // Set the size
    *(wrapper->sizeField) = size;
    
    // also increment the version number so that any pending
    // enumerators get nuked
    
    (*(wrapper->versionField))++;
}

void mono_listwrapper_load(MonoListWrapper* wrapper, void* data, size_t elemSize, mono_array_size_t count)
{
    mono_listwrapper_set_size(wrapper, count);
    
    gpointer dest = (gpointer)(*(wrapper->itemsField))->vector;
    
    if(MONO_TYPE_IS_REFERENCE(wrapper->elementType))
    {
        // Need a write-barrier here as we're changing references within managed space
        // (though despite the name it doesn't actually *do* the copy)
        mono_gc_wbarrier_arrayref_copy (*(wrapper->itemsField), dest, count);
	}
    
    memcpy((void*)dest, data, elemSize * count);
}

void* mono_listwrapper_begin_writing(MonoListWrapper* wrapper, int maxElems, guint32* handle)
{
    mono_listwrapper_clear(wrapper);
    if(mono_listwrapper_get_capacity(wrapper) < maxElems)
        mono_listwrapper_set_capacity(wrapper, maxElems);
    
    *handle = mono_gchandle_new((MonoObject*)(*(wrapper->itemsField)), TRUE);
    
    return (*(wrapper->itemsField))->vector;
}

void mono_listwrapper_end_writing(MonoListWrapper* wrapper, int totalWritten, guint32 handle)
{
    *(wrapper->sizeField) = totalWritten;
    
    if(MONO_TYPE_IS_REFERENCE(wrapper->elementType))
    {
        // Mark all the newly added objects for the GC
        mono_gc_wbarrier_arrayref_copy(*(wrapper->itemsField), (*(wrapper->itemsField))->vector, totalWritten);
    }
    
    mono_gchandle_free(handle);
}