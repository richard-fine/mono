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
    
    inline guint32& getSizeField() const { return *(guint32*)((char*)_list + _refl.size->offset); }
    inline guint32& getVersionField() const { return *(guint32*)((char*)_list + _refl.version->offset); }
    
    inline MonoArray* getItemsArray() const
    {
        gpointer* contentPtr = (gpointer*)((char*)_list + _refl.items->offset);
        // No need for a write barrier as we only ever use this pointer temporarily
        return *(MonoArray**)contentPtr;
    }

    MonoListWrapperBase(MonoObject* list, MonoListReflectionCache& refl) : _list(list), _refl(refl)
    {
        if(_refl.klass == NULL)
            _refl.PopulateFrom(mono_object_get_class(list));
    }
    
public:

    mono_array_size_t getCapacity() const
	{
        return mono_array_length(getItemsArray());
	}
    
    void setCapacity(mono_array_size_t value)
	{
		gpointer args[1];
		args[0] = &value;
		mono_property_set_value(_refl.capacityProp, _list, args, NULL);
	}
	
	guint32 getSize() const
	{
        return getSizeField();
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
    
        getSizeField() = 0;
            
        // also increment the version number so that any pending
        // enumerators get nuked
        
        getVersionField()++;
	}
	
	void add(const T item)
	{
        MonoArray* items = getItemsArray();
        mono_array_size_t curCapacity = items->max_length;
        guint32& size = getSizeField();
        
        if(curCapacity == size)
        {
            // grow the list
            setCapacity(MAX(4, curCapacity * 2));
            // refetch the items array
            items = getItemsArray();
        }
        
        ((T*)items->vector)[size++] = item;
	}
	
	void load(const T* data, int count)
	{
		clear();
        if(getCapacity() < count)
            setCapacity(count);
	
        MonoArray* content = getItemsArray();	
		memcpy(content->vector, data, sizeof(T) * count);
	
        getSizeField() = count;
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
		gpointer args[1];
		args[0] = item;
		mono_runtime_invoke(_refl.addMethod, _list, args, NULL);
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
	
		mono_field_set_value(_list, _refl.size, &count);
	}
};
