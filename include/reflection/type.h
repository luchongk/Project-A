#ifndef TYPE_H
#define TYPE_H

#include <cstring>

#include "utils.h"

namespace reflection
{

template <typename TYPE>
struct isPointer
{
    static const bool val = false;
};
// Specialise for yes
template <typename TYPE>
struct isPointer<TYPE*>
{
    static const bool val = true;
};

struct Type;
struct Field
{
    char name[32];
    Type *type;
    size_t offset;
    bool isPointer;
};

class TypeDB;
struct Type
{
    static const int MAX_FIELD_COUNT = 20;

    char name[32];
    size_t size;
    int fieldCount = 0;
    Field fields[MAX_FIELD_COUNT];
    void (*constructor)(void*);
    void (*destructor)(void*);
    bool isClass;
    Type* baseClass;
    TypeDB *db;

    Type(const Type &) = delete;

    Type(char *name, size_t size, TypeDB *db) : size{size}, db(db)
    {
        strncpy_s(this->name, name, 31);
    }

    template<typename T>
    Type* setBase() {
        this->baseClass = this->db->get<T>(true);
        for(int i = 0; i < this->baseClass->fieldCount; i++) {
            fields[fieldCount] = this->baseClass->fields[i];
            ++fieldCount;
        }

        return this;
    }

    template <typename T, typename U>
    Type *addField(char *name, U T::*field)
    {
        assert(fieldCount < MAX_FIELD_COUNT);

        strncpy_s(fields[fieldCount].name, name, 31);
        
        fields[fieldCount].type = this->db->get<U>(true);
        fields[fieldCount].offset = offsetof(T, *field);
        fields[fieldCount].isPointer = isPointer<U>::val;
        ++fieldCount;

        return this;
    }

    Field *findField(char *name) {
        for (int i = 0; i < fieldCount; i++)
        {
            if (strncmp(fields[i].name, name, 32) == 0)
            {
                return &fields[i];
            }
        }

        return nullptr;
    }
};

}; // namespace reflection

#endif