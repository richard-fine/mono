#include <mono/mini/jit.h>
#include <mono/metadata/object.h>
#include <mono/metadata/class-internals.h>

struct MonoListReflectionCache
{
	MonoClass* klass;
	MonoProperty* capacityProp;
	MonoMethod* addMethod;
	MonoMethod* clearMethod;
	MonoClassField* size;
	MonoClassField* items;
    MonoClassField* version;
	
	MonoListReflectionCache();
	void PopulateFrom(MonoClass* klass);
};

class MonoListWrapperBase
{
protected:
    MonoObject* _list;
    MonoListReflectionCache& _refl;
    
    guint32& _sizeField;
    guint32& _versionField;
    MonoArray*& _itemsField;

    MonoListWrapperBase(MonoObject* list, MonoListReflectionCache& refl) :
        _list(list), _refl(refl),
        _sizeField(*(guint32*)((char*)_list + _refl.size->offset)),
        _versionField(*(guint32*)((char*)_list + _refl.version->offset)),
        _itemsField(*(MonoArray**)((char*)_list + _refl.items->offset))
    {
        
    }
    
public:

    inline mono_array_size_t getCapacity() const
	{
        return _itemsField != NULL ? _itemsField->max_length : 0;
	}
    
    void setCapacity(mono_array_size_t value)
	{
        if(getCapacity() == value) return;
        
		gpointer args[1];
		args[0] = &value;
		mono_property_set_value(_refl.capacityProp, _list, args, NULL);
	}
	
	inline guint32 getSize() const
	{
        return _sizeField;
	}
};

template<typename T>
class MonoListWrapper : public MonoListWrapperBase
{
		
	public:
	MonoListWrapper(MonoObject* list, MonoListReflectionCache& refl) : MonoListWrapperBase(list, refl)
	{
		
    }
	
	void clear()
	{
        // Ideally we want to avoid actually invoking Clear()
        // as it's another native->managed transition and it also
        // spends time nulling out all the entries in the array.
        // While that's valuable for reference types (so they can be
        // GCed) it's not worth it for primitive types.
        //
        // Reference types should all be handled by the MonoObject*
        // template specialization further down in the file, leaving
        // us free to do a 'fast clear' here.
    
        _sizeField = 0;
            
        // also increment the version number so that any pending
        // enumerators get nuked
        
        _versionField++;
	}
	
	void add(const T item)
	{
        if(getCapacity() == _sizeField)
        {
            // grow the list
            setCapacity(MAX(4, _itemsField->max_length * 2));
        }
        
        ((T*)(_itemsField->vector))[_sizeField++] = item;
	}
	
	void load(const T* data, int count)
	{
		clear();
        if(getCapacity() < count)
            setCapacity(count);
	
		memcpy((void*)_itemsField->vector, data, sizeof(T) * count);
	
        _sizeField = count;
	}
    
    T* beginWriting(int capacity, guint32* handle)
    {
        clear();
        if(getCapacity() < capacity)
            setCapacity(capacity);
        
        *handle = mono_gchandle_new((MonoObject*)_itemsField, TRUE);
        
        return (T*)_itemsField->vector;
    }
    
    void endWriting(int totalWritten, guint32 handle)
    {
        _sizeField = totalWritten;
        mono_gchandle_free(handle);
    }

};

// Specialize the template for MonoObject* so we can do the memory barrier stuff
template<>
class MonoListWrapper<MonoObject*> : public MonoListWrapperBase
{	
	public:
	MonoListWrapper(MonoObject* list, MonoListReflectionCache& refl) : MonoListWrapperBase(list, refl)
	{		
		
	}
	
	void clear()
	{
        // MonoObject being the base of all reference types, we
        // need to do a 'proper' clear so that all references in
        // the array stop keeping things alive against the GC.
        
		mono_runtime_invoke(_refl.clearMethod, _list, NULL, NULL);
	}
		
	void add(MonoObject* item)
	{		
		if(getCapacity() == _sizeField)
        {
            // grow the list
            setCapacity(MAX(4, _itemsField->max_length * 2));
        }
        
        mono_gc_wbarrier_generic_store(((T*)(_itemsField->vector))[_sizeField++], item);
	}
	
	void load(const MonoObject* data, int count)
	{
		clear();
		setCapacity(count);
	
		MonoArray* content;
		mono_field_get_value(_list, _refl.items, &content);
	
		gpointer dest = mono_array_addr_with_size(content, sizeof(MonoObject*), 0);
        // Need a write-barrier here as we're changing references within managed space
		mono_gc_wbarrier_arrayref_copy ((content), dest, (count));
		memcpy(dest, data, sizeof(MonoObject*) * count);
	
		_sizeField = count;
	}
};
