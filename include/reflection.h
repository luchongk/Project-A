#ifndef REFLECTION_H
#define REFLECTION_H

#define REFLECT     static void initReflection(reflection::Type *CLASS); \
                    friend class reflection::TypeDB;
                    
#define REFLECTION_REGISTRATION(T) void T::initReflection(reflection::Type *CLASS)

#include "reflection/type.h"
#include "reflection/type_db.h"

#endif