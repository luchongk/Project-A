#ifndef ARRAY_H
#define ARRAY_H

#include <initializer_list>
#include "memory.h"
#include "types.h"

template<typename T>
struct ArrayView {
    T* data = nullptr;
    s64 count = 0;
};

template<typename T>
ArrayView<T> make_view(T array[], int count) {
    ArrayView<T> view;
    view.data = array;
    view.count = count;

    return view;
}

template<typename T>
struct Array : public ArrayView<T> {
    s64 allocated = 0;
    s64 start_size = 8;

    Allocator allocator = nullptr;
    void* allocator_data = nullptr;

    Array() = default;
    
    Array(std::initializer_list<T> init) {
        for(auto it = init.begin(); it != init.end(); it++) {
            array_add(this, *it);
        }
    }

    T& operator[](int i) {
        return this->data[i];
    }
};

template<typename T>
void array_reset(Array<T>* array) {
    array->count = 0;
}

template<typename T>
void array_add(Array<T>* array, T to_add) {
    if(array->count >= array->allocated) {
        s64 new_size = 2 * array->allocated;
        if(new_size < array->start_size) new_size = array->start_size;

        if(array->allocator) {
            array->data = (T*)array->allocator(array->data, sizeof(T) * array->allocated, sizeof(T) * new_size, array->allocator_data);
        }
        else {
            array->data = realloc_<T>(array->data, array->allocated, new_size);
        }

        array->allocated = new_size;
    }

    array->data[array->count++] = to_add;
}

#define For(array) for(auto it = &(array).data[0]; it < &(array).data[(array).count]; it++)

#define ReverseFor(array) for(auto it = &(array).data[(array).count - 1]; it >= &(array).data[0]; it--)

#endif