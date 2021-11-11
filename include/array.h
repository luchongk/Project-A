#ifndef ARRAY_H
#define ARRAY_H

#include <initializer_list>
#include "memory.h"
#include "types.h"

template<typename T>
struct ArrayView {
    T* data = nullptr;
    uint count = 0;
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
    uint allocated = 0;

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
void array_resize(Array<T>* array, int new_size) {
    if(array->allocator) {
        array->data = (T*)array->allocator(array->data, sizeof(T) * array->allocated, sizeof(T) * new_size, array->allocator_data);
    }
    else {
        array->data = realloc_<T>(array->data, array->allocated, new_size);
    }

    array->allocated = new_size;
}

template<typename T>
inline void array_reserve(Array<T>* array, int size) {
    if(array->allocated < size) {
        array_resize(array, size);
    }
}

template<typename T>
inline void array_grow_if_full(Array<T>* array) {
    if(array->allocated == array->count) {
        uint new_size = 2 * array->allocated;
        if(new_size < 8) new_size = 8;

        array_resize(array, new_size);
    }
}

template<typename T>
void array_reset(Array<T>* array) {
    array->count = 0;
}

template<typename T>
T* array_add(Array<T>* array, T to_add) {
    array_grow_if_full(array);

    array->data[array->count] = to_add;
    auto result = &array->data[array->count];
    array->count++;

    return result;
}

#define For(array) for(auto it = &(array).data[0]; it < &(array).data[(array).count]; it++)

#define ReverseFor(array) for(auto it = &(array).data[(array).count - 1]; it >= &(array).data[0]; it--)

#endif