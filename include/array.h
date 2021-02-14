#ifndef ARRAY_H
#define ARRAY_H

#include "memory.h"
#include "types.h"

template<typename T>
struct Array {
    T* data = nullptr;
    s64 count = 0;
    s64 allocated = 0;

    Allocator allocator = nullptr;
    void* allocator_data = nullptr;
};

template<typename T>
void array_add(Array<T>* array, T toAdd) {
    if(array->count >= array->allocated) {
        s64 newSize = 2 * array->allocated;
        if(newSize < 8) newSize = 8;

        if(array->allocator) {
            array->data = (T*)array->allocator(array->data, sizeof(T) * array->count, sizeof(T) * newSize, array->allocator_data);
        } else {
            array->data = realloc_<T>(array->data, array->count, newSize);
        }

        array->allocated = newSize;
    }

    array->data[array->count++] = toAdd;
}

#define For(array) for(auto it = &array.data[0]; it < &array.data[array.count]; it++)

#endif