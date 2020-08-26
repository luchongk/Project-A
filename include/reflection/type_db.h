#ifndef TYPE_DB_H
#define TYPE_DB_H

#include <type_traits>

#include "utils.h"
#include "string_map.h"

class LinearAllocator;

namespace reflection
{

struct Type;
class TypeDB
{
    LinearAllocator *allocator;
    StringMap<Type> types;

    //SFINAE weirdness
    template <typename T>
    static char func(decltype(&T::initReflection));
    template <typename T>
    static int func(...);
    template <typename T>
    struct IsReflected
    {
        enum
        {
            value = (sizeof(func<T>(nullptr)) == sizeof(char))
        };
    };

    // This version is called if T has a static method named "initReflection":
    template <typename T, typename std::enable_if<IsReflected<T>::value, int>::type = 0>
    static void registerFields(Type *t)
    {
        T::initReflection(t);
    }

    // This version is called otherwise:
    template <typename T, typename std::enable_if<!IsReflected<T>::value, int>::type = 0>
    static void registerFields(Type *t)
    {
        //TODO: Find a way to check if what comes in here is a class
        //assert(strncmp("class ", typeid(T).name(), 6));
    }

public:
    TypeDB(const TypeDB&) = delete;
    
    TypeDB(LinearAllocator *allocator);

    inline void clear() {
        this->types.clear();
    }

    template <typename T>
    Type *get(bool shouldCreate = false)
    {
        char *name = (char *)typeid(T).name();

        Type *t = types.get(name);
        if (shouldCreate && !t)
        {
            t = create<T>();
        }

        return t;
    }

    template <typename T>
    Type *create();

    template <typename TYPE>
    static void constructObject(void* object)
    {
        new (object) TYPE;
    }

    template <typename TYPE>
    static void destructObject(void* object)
    {
        ((TYPE*)object)->TYPE::~TYPE();
    }
};

TypeDB::TypeDB(LinearAllocator *allocator)
    : allocator(allocator),
      types(allocator)
{
    //Initialize all builtin types 
}

template <typename T>
inline Type *TypeDB::create()
{
    char *name = (char *)typeid(T).name();

    Type *t = new (*allocator) Type(name, sizeof(T), this);
    t->constructor = constructObject<T>;
    t->destructor = destructObject<T>;
    registerFields<T>(t);

    types.set(name, t);
    return t;
}

};

#endif