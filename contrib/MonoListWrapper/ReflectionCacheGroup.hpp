#include <map>
#include <mono/metadata/object.h>

struct MonoListReflectionCache;

class ReflectionCacheGroup
{
    std::map<MonoClass*, MonoListReflectionCache> _reflectionCaches;
    
public:
    MonoListReflectionCache& GetCacheFor(MonoObject* listObj);
};