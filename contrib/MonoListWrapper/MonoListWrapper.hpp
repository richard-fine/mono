#include <mono/mini/jit.h>
#include <mono/metadata/object.h>

struct MonoListReflectionCache
{
	MonoClass* klass;
	MonoProperty* capacityProp;
	MonoMethod* addMethod;
	MonoMethod* clearMethod;
	MonoClassField* size;
	MonoClassField* items;
	
	MonoListReflectionCache();
	void PopulateFrom(MonoClass* klass);
};

template<typename T>
class MonoListWrapper
{
	private:
	MonoObject* _list;
	
	MonoListReflectionCache& _refl;
	
	public:
	MonoListWrapper(MonoObject* list, MonoListReflectionCache& refl) : _refl(refl)
	{		
		_list = list;
		
		if(_refl.klass == NULL)
		{
			_refl.PopulateFrom(mono_object_get_class(list));
		}
	}
	
	void clear()
	{
		mono_runtime_invoke(_refl.clearMethod, _list, NULL, NULL);
	}
	
	int getCapacity() const
	{
		return *(int*)mono_object_unbox(mono_property_get_value(_refl.capacityProp, _list, NULL, NULL));
	}
	
	int setCapacity(int value)
	{
		gpointer args[1];
		args[0] = &value;
		mono_property_set_value(_refl.capacityProp, _list, args, NULL);
	}
	
	int getSize() const
	{
		int result = 0;
		mono_field_get_value(_list, _refl.size, &result);
		return result;
	}
	
	void add(const T item)
	{
		gpointer args[1];
		args[0] = item;
		mono_runtime_invoke(_refl.addMethod, _list, args, NULL);
	}
	
	void load(const T* data, int count)
	{
		clear();
		setCapacity(count);
	
		MonoArray* content;
		mono_field_get_value(_list, _refl.items, &content);
	
		gpointer dest = mono_array_addr_with_size(content, sizeof(T), 0);
		memcpy(dest, data, sizeof(T) * count);
	
		mono_field_set_value(_list, _refl.size, &count);
	}
};

// Specialize the template for MonoObject* so we can do the memory barrier stuff
template<>
class MonoListWrapper<MonoObject*>
{
	private:
	MonoObject* _list;
	
	MonoListReflectionCache& _refl;
	
	public:
	MonoListWrapper(MonoObject* list, MonoListReflectionCache& refl) : _refl(refl)
	{		
		_list = list;
		
		if(_refl.klass == NULL)
		{
			_refl.PopulateFrom(mono_object_get_class(list));
		}
	}
	
	void clear()
	{
		mono_runtime_invoke(_refl.clearMethod, _list, NULL, NULL);
	}
	
	int getCapacity() const
	{
		return *(int*)mono_object_unbox(mono_property_get_value(_refl.capacityProp, _list, NULL, NULL));
	}
	
	int setCapacity(int value)
	{
		gpointer args[1];
		args[0] = &value;
		mono_property_set_value(_refl.capacityProp, _list, args, NULL);
	}
	
	int getSize() const
	{
		int result = 0;
		mono_field_get_value(_list, _refl.size, &result);
		return result;
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
		mono_gc_wbarrier_arrayref_copy ((content), dest, (count));
		memcpy(dest, data, sizeof(MonoObject*) * count);
	
		mono_field_set_value(_list, _refl.size, &count);
	}
};
