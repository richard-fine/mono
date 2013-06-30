#include <mono/mini/jit.h>
#include <map>
#include "ReflectionCacheGroup.hpp"
#include "MonoListWrapper.hpp"

std::map<MonoClass*, MonoListReflectionCache> _reflectionCaches;

MonoListReflectionCache& ReflectionCacheGroup::GetCacheFor(MonoObject* listObj)
{
	MonoClass* klass = mono_object_get_class(listObj);
	
	std::map<MonoClass*, MonoListReflectionCache>::iterator it;
	it = _reflectionCaches.find(klass);
	if(it == _reflectionCaches.end())
	{
		it = _reflectionCaches.insert(it, std::pair<MonoClass*, MonoListReflectionCache>(klass, MonoListReflectionCache()));
		it->second.PopulateFrom(klass);
	}
	
	return it->second;
}