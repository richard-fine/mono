#ifndef _MONO_LISTWRAPPER_H_
#define _MONO_LISTWRAPPER_H_

#include <mono/metadata/object.h>

typedef struct
{
    MonoObject* list;
    MonoType* elementType;
    guint32* sizeField;
    guint32* versionField;
    MonoArray** itemsField;
} MonoListWrapper;

void mono_listwrapper_init(MonoListWrapper* wrapper, MonoObject* list);

mono_array_size_t mono_listwrapper_get_capacity(MonoListWrapper* wrapper);
void mono_listwrapper_set_capacity(MonoListWrapper* wrapper, mono_array_size_t value);

guint32 mono_listwrapper_get_size(MonoListWrapper* wrapper);
void mono_listwrapper_set_size(MonoListWrapper* wrapper, int size);

void mono_listwrapper_clear(MonoListWrapper* wrapper);
void mono_listwrapper_load(MonoListWrapper* wrapper, void* data, size_t elemSize, mono_array_size_t count);

void* mono_listwrapper_begin_writing(MonoListWrapper* wrapper, int maxElems, guint32* handle);
void mono_listwrapper_end_writing(MonoListWrapper* wrapper, int totalWritten, guint32 handle);

void mono_listwrapper_clear_refl_data();

#endif
