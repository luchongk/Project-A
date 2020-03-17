#ifndef REFLECTION_H
#define REFLECTION_H

#include <iostream>
#include <string.h>

namespace reflection {
    struct Field;
    
    class Type {
    public:    
        char name[32];
        size_t size;

        Type() {}
        
        Type(char* name, size_t size) : size{size} {
            strncpy_s(this->name, name, 32);
        }

        virtual Field* getField(char* name) { return nullptr; }
        virtual int getFieldCount() { return 0; }
        virtual Field* getFields() { return nullptr; }
    };

    struct Field {
        char name[32];
        Type* type;
        //TODO: int offset;
    };

    class TypeClass : public Type {
        Field fields[10];
        int fieldCount = 0;
    public:
        template<typename T>
        friend class class_;
        
        virtual Field* getField(char* name) {
            for(int i = 0; i < fieldCount; i++) {
                if(strncmp(fields[i].name, name, 32) == 0) {
                    return &fields[i];
                }
            }

            return nullptr;
        }

        virtual int getFieldCount() {
            return fieldCount;
        }

        virtual Field* getFields() {
            return fields;
        }
    };

    namespace {
        template<typename T>
        class IsClass {
        private:
            typedef char One;
            typedef struct { char a[2]; } Two;
            template<typename C> static One test(int C::*);
            template<typename C> static Two test(...);
        public:
            static constexpr bool Yes() { return sizeof(IsClass<T>::test<T>(nullptr)) == 1; }
            static constexpr bool No() { return !Yes; }
        };
    }
    
    template<typename T>
    static Type* get() {
        if(IsClass<T>::Yes()) {
            static TypeClass t;
            return &t;
        }
        else {
            static Type t{(char*)typeid(T).name(), sizeof(T)};
            return &t;
        }
    }

    template<typename T>
    class class_ {
        TypeClass* type;
    public:
        class_(char* name) {
            this->type = (TypeClass*)get<T>();
            strncpy_s(this->type->name, name, 32);
            this->type->size = sizeof(T);
        }

        template<typename U>
        class_ setField(char* name, U T::* field) {
            strncpy_s(this->type->fields[this->type->fieldCount].name, name, 32);
            this->type->fields[this->type->fieldCount].type = get<U>();
            ++this->type->fieldCount;
            return *this;
        }
    };
}

#define REFLECTION_REGISTRATION                     \
static void auto_register_init();                   \
namespace {                                         \
    struct __auto_register__ {                      \
        __auto_register__() {                       \
            auto_register_init();                   \
        }                                           \
    };                                              \
}                                                   \
static __auto_register__ auto_register##__LINE__;   \
static void auto_register_init()

/***

EXAMPLE USAGE

class MyType {
public:
    int a;
    float b;
};

REFLECTION_REGISTRATION {
    reflection::class_<MyType>("MyType")
        .setField("a", &MyType::a)
        .setField("b", &MyType::b);
}

int main() {
    reflection::Type* t = reflection::get<MyType>();
    std::cout << t->name << " " << t->size << std::endl;

    int count = t->getFieldCount();
    reflection::Field* fields = t->getFields();

    std::cout << "Fields:" << std::endl;
    for(int i = 0; i < count; i++) {
        std::cout << fields[i].type->name << " " << fields[i].name << " " << fields[i].type->size << std::endl;
    }
}

***/

#endif